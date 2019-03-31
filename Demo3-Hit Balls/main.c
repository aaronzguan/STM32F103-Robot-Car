/*******
Author: GUAN Zhong 14110265d
Date: 14 Mar, 2019
Results: Hit and push three balls into the region in 20s.
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

float Kp_forward = 0.35; // 0.5
float Ki_forward = 0;
float Kd_forward = 0;

float Kp_back = 0.25; // 0.35
float Ki_back = 0;
float Kd_back = 0.4; // 0.4

int PWM_forward = 35;
int PWM_back = 50;

int BBE_X = 500, BBE_Y, BOE_X, BOE_Y, BYL_X,BYL_Y;
int init_posX = 840, init_posY = 285;
int car_HeadX = 873, car_HeadY = 286, car_EndX = 988, car_EndY = 286;
float carAngle = 0;
float targetAngle = 0;
float error = 0;
float last_error = 0;
float integral = 0;
float derivative = 0;
int PWM_R = 0;
int PWM_L = 0;

unsigned int button_pressed = 0;
bool isOn = false;
bool shutDown = false;
bool startTurn_1 = false;
bool startTurn_2 = false;
bool startTurn_3 = false;
bool startTurn_4 = false;

int turningCnt = 0;
int turningPeriod = 38;

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
			//blinkLight();
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
			blinkLight();
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
		wifi_data[charPos - 7] == 'L' && 
		wifi_data[charPos - 8] == 'Y' && 
		wifi_data[charPos - 9] == 'B'){
			BYL_X = Hex2Dec(wifi_data[charPos - 6]) * 256 + Hex2Dec(wifi_data[charPos - 5]) * 16 + Hex2Dec(wifi_data[charPos - 4]) * 1;
			BYL_Y = Hex2Dec(wifi_data[charPos - 3]) * 256 + Hex2Dec(wifi_data[charPos - 2]) * 16 + Hex2Dec(wifi_data[charPos - 1]) * 1;
			//blinkLight();
			charPos = -1;
		}
}

void move(void){
	if(state == 1){
		if((BBE_X > 465) && (car_EndX > BBE_X)){	
			carAngle = asin((car_HeadY - car_EndY) / sqrt(pow(car_HeadX-car_EndX,2) + pow(car_HeadY-car_EndY,2))) * (180.0 / PI); 
			targetAngle = asin((BBE_Y - car_HeadY) / sqrt(pow(BBE_X-car_HeadX,2) + pow(BBE_Y-car_HeadY,2))) * (180.0 / PI);
			
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
		powerRight(-50);
		powerLeft(50);
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
		powerRight(50);
		powerLeft(-50);
		startTurn_2 = true;
	}
	
	if(state == 5){
		if((BOE_X > 480) && (car_EndX > BOE_X)){	
			carAngle = asin((car_HeadY - car_EndY) / sqrt(pow(car_HeadX-car_EndX,2) + pow(car_HeadY-car_EndY,2))) * (180.0 / PI); 
			targetAngle = asin((BOE_Y - car_HeadY) / sqrt(pow(BOE_X-car_HeadX,2) + pow(BOE_Y-car_HeadY,2))) * (180.0 / PI);
			
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
	
	if(state == 6){
		powerRight(-50);
		powerLeft(50);
		startTurn_3 = true;
	}
	
	if(state == 7){
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
	
	if(state == 8){
		powerRight(50);
		powerLeft(-50);
		startTurn_4 = true;
	}
	
	if(state == 9){
		if((BYL_X > 465) && (car_EndX > BYL_X)){	
			carAngle = asin((car_HeadY - car_EndY) / sqrt(pow(car_HeadX-car_EndX,2) + pow(car_HeadY-car_EndY,2))) * (180.0 / PI); 
			targetAngle = asin((BYL_Y - car_HeadY) / sqrt(pow(BYL_X-car_HeadX,2) + pow(BYL_Y-car_HeadY,2))) * (180.0 / PI);
			
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
		
		if(startTurn_3){
			turningCnt ++;
			if(turningCnt == turningPeriod){
				turningCnt = 0;
				startTurn_3 = false;
				state = 7;
			}
		}
		
		if(startTurn_4){
			turningCnt ++;
			if(turningCnt == turningPeriod){
				turningCnt = 0;
				startTurn_4 = false;
				state = 9;
			}
		}
		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}