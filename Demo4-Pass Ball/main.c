/*******
Author: GUAN Zhong 14110265d
File: The program of Demo 4
Demo Date: 27 Mar, 2019
Results: Hit one ball from region A to B for 3 times in 36.9 s.
********/

#include "stm32f10x.h"                  // Device header
#include "header.h"
#include "stdbool.h"
#include "string.h"
#include "stdio.h"
#include "misc.h"
#include "math.h"

#define PI 3.14159265

char wifi_data[10000];
char start[]="AT+CWMODE=1\r\n";
char connectWIFI[]="AT+CWJAP=\"IntegratedProject\",\"31053106\"\r\n";
char getIP[]="AT+CIFSR\r\n";
char retrieveData[] = "AT+CIPSTART=\"UDP\",\"0\",0,3105,2\r\n";
char receiveBuffer[100];
int charPos = -1;
bool WIFIConnected = false;
bool dataReceived = false;

int state = 1;
unsigned int button_pressed = 0;
bool isOn = false;
bool shutDown = false;
bool startTurn_1 = false;
bool startTurn_2 = false;

bool check_1_pos = false;
bool check_2_pos = false;
bool ball_stop = false;

int checkCnt = 0;
int checkPeriod = 15;
int pos_beforeDelay = 0;
int pos_afterDelay = 0;

int turningCnt = 0;
int turningPeriod = 38;

float Kp_forward = 0.35; // 0.5
float Ki_forward = 0;
float Kd_forward = 0;

float Kp_back = 0.25; // 0.35
float Ki_back = 0;
float Kd_back = 0.4; // 0.4

int PWM_forward = 50;
int PWM_back = 40;
int PWM_turn = 50;

int posX, posY = 0;
bool checked = false;
bool inGreen = false;

int BBE_X = 500, BBE_Y, BOE_X, BOE_Y, BYL_X,BYL_Y;
int init_posX = 860, init_posY = 285;
int car_HeadX = 873, car_HeadY = 286, car_EndX = 988, car_EndY = 286;
float carAngle = 0;
float targetAngle = 0;
float error = 0;
float last_error = 0;
float integral = 0;
float derivative = 0;
int PWM_R = 0;
int PWM_L = 0;

int wheel_count_R = 0;
int wheel_count_L = 0;

void clear_variables(){
	carAngle = 0;
	targetAngle = 0;
	error = 0;
	last_error = 0;
	integral = 0;
	derivative = 0;
	PWM_R = 0;
	PWM_L = 0;
}

// Right Wheel Counter EXTI
void EXTI1_IRQHandler(){
	if (EXTI_GetITStatus(EXTI_Line1) != RESET){
		wheel_count_R++;
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}
// Left Wheel Counter & On-Board Button EXTI
void EXTI9_5_IRQHandler(){
	if(EXTI_GetITStatus(EXTI_Line6) != RESET){
		wheel_count_L++;
		EXTI_ClearITPendingBit(EXTI_Line6);
	}
	
	if(EXTI_GetITStatus(EXTI_Line8) != RESET){
		clear_variables(); // Clear all variables
		button_pressed = 0;
		if(button_pressed==0){
			if(!isOn){
				isOn = true;
			}
			else{
				isOn = false;
				shutDown = true;
				offLight();
				stopMotor();
			}
		}
		button_pressed ++;
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	else{
			button_pressed = 0;
	}
}

// Check USART Data from WIFI
void USART2_IRQHandler(){
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
		charPos++;
		wifi_data[charPos] = (char) USART_ReceiveData(USART2);
		check_wifiData();
	}
}

void check_wifiData(void){
	// Check if Got IP
	if(wifi_data[charPos] == '\r' && 
		wifi_data[charPos - 1] == 'P' && 
		wifi_data[charPos - 2] == 'I' && 
		wifi_data[charPos - 3] == ' ' && 
		wifi_data[charPos - 4] == 'T' && 
		wifi_data[charPos - 5] == 'O' && 
		wifi_data[charPos - 6] == 'G' && 
		wifi_data[charPos - 7] == ' ' && 
		wifi_data[charPos - 8] == 'I' && 
		wifi_data[charPos - 9] == 'F' && 
		wifi_data[charPos - 10] == 'I' && 
		wifi_data[charPos - 11] == 'W'){
			offLight();
			charPos = -1;
			WIFIConnected = true;
		}
		
	// Car Head
	if((!shutDown) && 
		wifi_data[charPos] == '\r' && 
		wifi_data[charPos - 7] == 'D' && 
		wifi_data[charPos - 8] == 'E' && 
		wifi_data[charPos - 9] == 'H'){
			car_HeadX = Hex2Dec(wifi_data[charPos - 6]) * 256 + Hex2Dec(wifi_data[charPos - 5]) * 16 + Hex2Dec(wifi_data[charPos - 4]) * 1;
			car_HeadY = Hex2Dec(wifi_data[charPos - 3]) * 256 + Hex2Dec(wifi_data[charPos - 2]) * 16 + Hex2Dec(wifi_data[charPos - 1]) * 1;
			charPos = -1;
			blinkLight();
		}
	
	// Car End
	if((!shutDown) && 
		wifi_data[charPos] == '\r' && 
		wifi_data[charPos - 7] == 'L' && 
		wifi_data[charPos - 8] == 'A' && 
		wifi_data[charPos - 9] == 'T'){
			car_EndX = Hex2Dec(wifi_data[charPos - 6]) * 256 + Hex2Dec(wifi_data[charPos - 5]) * 16 + Hex2Dec(wifi_data[charPos - 4]) * 1;
			car_EndY = Hex2Dec(wifi_data[charPos - 3]) * 256 + Hex2Dec(wifi_data[charPos - 2]) * 16 + Hex2Dec(wifi_data[charPos - 1]) * 1;
			charPos = -1;
		}
		
	// 'B' 'B' 'E' -> Ball of Blue 
	if((!shutDown) && 
		wifi_data[charPos] == '\r' && 
		wifi_data[charPos - 7] == 'E' && 
		wifi_data[charPos - 8] == 'B' && 
		wifi_data[charPos - 9] == 'B'){
			BBE_X = Hex2Dec(wifi_data[charPos - 6]) * 256 + Hex2Dec(wifi_data[charPos - 5]) * 16 + Hex2Dec(wifi_data[charPos - 4]) * 1;
			BBE_Y = Hex2Dec(wifi_data[charPos - 3]) * 256 + Hex2Dec(wifi_data[charPos - 2]) * 16 + Hex2Dec(wifi_data[charPos - 1]) * 1;
			//blinkLight();
			charPos = -1;
		}
	
	// 'B' 'O' 'E' -> Ball of Orange 
	if((!shutDown) && 
		wifi_data[charPos] == '\r' && 
		wifi_data[charPos - 7] == 'E' && 
		wifi_data[charPos - 8] == 'O' && 
		wifi_data[charPos - 9] == 'B'){
			BOE_X = Hex2Dec(wifi_data[charPos - 6]) * 256 + Hex2Dec(wifi_data[charPos - 5]) * 16 + Hex2Dec(wifi_data[charPos - 4]) * 1;
			BOE_Y = Hex2Dec(wifi_data[charPos - 3]) * 256 + Hex2Dec(wifi_data[charPos - 2]) * 16 + Hex2Dec(wifi_data[charPos - 1]) * 1;
			//blinkLight();
			charPos = -1;
		}
		
		// 'B' 'Y' 'L' -> Ball of Yellow 
	if((!shutDown) && 
		wifi_data[charPos] == '\r' && 
		wifi_data[charPos - 7] == 'W' && 
		wifi_data[charPos - 8] == 'Y' && 
		wifi_data[charPos - 9] == 'B'){
			BYL_X = Hex2Dec(wifi_data[charPos - 6]) * 256 + Hex2Dec(wifi_data[charPos - 5]) * 16 + Hex2Dec(wifi_data[charPos - 4]) * 1;
			BYL_Y = Hex2Dec(wifi_data[charPos - 3]) * 256 + Hex2Dec(wifi_data[charPos - 2]) * 16 + Hex2Dec(wifi_data[charPos - 1]) * 1;
			//blinkLight();
			charPos = -1;
		}
}

// Check the position of blue ball
void checkBallPos(void){
	posX = BYL_X;
	posY = BYL_Y + 20;
	checked = true;
}

// Check whether the ball is hit to the green or not
void checkInGreen(void){
	if(BYL_X < 455) {
		inGreen = true;
	}
	else{
		inGreen = false;
	}
}

void move(void){
	
	if(!checked){
		checkBallPos();
	}

	if(state == 1){
		if(car_HeadX > posX){
			carAngle = asin((car_HeadY - car_EndY) / sqrt(pow(car_HeadX-car_EndX,2) + pow(car_HeadY-car_EndY,2))) * (180.0 / PI); 
			targetAngle = asin((posY - car_HeadY) / sqrt(pow(posX-car_HeadX,2) + pow(posY-car_HeadY,2))) * (180.0 / PI);
			
			last_error = error;
			error = targetAngle - carAngle;
			integral += error;
			derivative = error - last_error;
			
			PWM_R = (int) PWM_forward + error * Kp_forward + integral * Ki_forward + derivative * Kd_forward;
			PWM_L = (int) PWM_forward - error * Kp_forward - integral * Ki_forward - derivative * Kd_forward;
			
			powerRight(PWM_R);
			powerLeft(PWM_L);
		}
		else{
			stopMotor();
			clear_variables();
			state++;
		}
	}
	
	if(state == 2){
		powerRight((-1)*PWM_turn);
		powerLeft(PWM_turn);
		startTurn_1 = true;
	}
	
	if(state == 3){
		if(car_HeadX < init_posX){
			carAngle = asin((car_HeadY - car_EndY) / sqrt(pow(car_HeadX-car_EndX,2) + pow(car_HeadY-car_EndY,2))) * (180.0 / PI); 
			targetAngle = (-1) * asin((init_posY - car_HeadY) / sqrt(pow(init_posX-car_HeadX,2) + pow(init_posY-car_HeadY,2))) * (180.0 / PI);
			
			last_error = error;
			error = targetAngle + carAngle;
			integral += error;
			derivative = error - last_error;
			
			PWM_R = (int) PWM_back + error * Kp_back + integral * Ki_back + derivative * Kd_back;
			PWM_L = (int) PWM_back - error * Kp_back - integral * Ki_back - derivative * Kd_back;
			
			powerRight(PWM_R);
			powerLeft(PWM_L);
		}else{
			stopMotor();
			clear_variables();
			state ++;
		}	
	}
	
	if(state == 4){
		powerRight(PWM_turn);
		powerLeft((-1)*PWM_turn);
		startTurn_2 = true;
	}
	
	// Check if the ball is stoped
	if(state == 5){
		stopMotor();
		check_1_pos = true;
		check_2_pos = true;
		ball_stop = false;
		state ++;
	}
	
	// After the ball is stoped, check if it is in the green zone
	if(state == 6){
		if(ball_stop){
			checkInGreen();
			if(!inGreen){
				state = 1;
				checked = false;
				ball_stop = false;
			}
			else{
				state ++;
			}
		}
	}
	
	// Check if the ball is hit back
	if(state == 7){
		if(BYL_X < 615){
			stopMotor();
		}
		else{
			state ++;
		}
	}
	
	// Check if the ball is stop
	if(state == 8){
		stopMotor();
		check_1_pos = true;
		check_2_pos = true;
		ball_stop = false;
		state ++;
	}
	
	// Hit the ball after the ball is stop
	if(state == 9){
		if(ball_stop){
			state = 1;
			checked = false;
		}
	}
}

int main(){
	TIM3_CH12_PWM_init();
	TIM2_Init();
	Left_Wheel_Cnt_init();
	Right_Wheel_Cnt_init();
	Wheel_Dir_Init();
	Button_init();
	OnBoard_lED_Init();
	SPI2_init();
	USART2_init();
	
	onLight();
	USARTsend(start,sizeof(start));
	USARTsend(connectWIFI,sizeof(connectWIFI));
	
	while(1){
	}
}

void TIM2_IRQHandler(){
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		if(WIFIConnected & !dataReceived){
			USARTsend(retrieveData,sizeof(retrieveData));
		}
		if(isOn){
			move();
		}
		
		if(startTurn_1){
			turningCnt ++;
			if(turningCnt == turningPeriod){
				turningCnt = 0;
				startTurn_1 = false;
				state = 3;
			}
		}
		
		if(startTurn_2){
			turningCnt ++;
			if(turningCnt == turningPeriod){
				turningCnt = 0;
				startTurn_2 = false;
				state = 5;
			}
		}
		
		if(check_1_pos){
			pos_beforeDelay = BYL_X;
			check_1_pos = false;
		}
		
		if(check_2_pos){
			checkCnt ++;
			if(checkCnt == checkPeriod){
				checkCnt = 0;
				pos_afterDelay = BYL_X;
				if(pos_beforeDelay == pos_afterDelay){
					check_1_pos = false;
					check_2_pos = false;
					ball_stop = true;
				}
				else{
					check_1_pos = true;
					check_2_pos = true;
					ball_stop = false;
				}
			}
		}
		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
