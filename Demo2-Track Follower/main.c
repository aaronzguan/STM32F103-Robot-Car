#include "stm32f10x.h"                  // Device header
#include "stdbool.h"
#include "string.h"
#include "PinMap.h"
	
unsigned int isOn = 0;

// Line Follower Variables
int state = 1;

int wheel_speed_outer = 42; //40
int flag_check_time = 40; // 40

int number_of_flag = 4;

int count = 0;
int turn = 0;
bool outLine = true;
int delay = 0;
int error = 0;
int last_error_inner = 0;
int last_error_outer = 0;

int delay_period = 0;
int time_stamp = 0;
bool isDelay = false;

bool outer2inner = false;
bool inner2outer = false;

// Wait turning
bool delay_turn2in = false;
int delay_turn2in_cnt = 0;
int delay_turn2in_period = 1;

// Trunning from outter to inner delay
bool delay_O2I = false;
int delay_O2I_cnt = 0;
int delay_O2I_period = 12; //15 // use the single one for two turning

// Time Delay to stop from inner to outer
bool delay_I2O = false;
int delay_I2O_cnt = 0;
int delay_I2O_period = 12;//12

// Time Delay to turn around
bool delay_turn2out = false;
int delay_turn2out_cnt = 0;
int delay_turn2out_period = 20; //15


// Time Delay to finally turn to the inner track
bool delay_endPoint = false;
int delay_endPoint_cnt = 0;
int delay_endPoint_period = 15; //13

bool startTurn = false;

void clear_line_follower(){
	state = 1;
	count = 0;
	turn = 0;
	delay = 0;
	outLine = true;
	error = 0;
	last_error_inner = 0;
	last_error_outer = 0;
	
	delay_period = 0;
	time_stamp = 0;
	isDelay = false;
	
	outer2inner = false;
	inner2outer = false;
}

// PID Variables
int target_count_R  = 8;
int target_count_L  = 8;
int wheel_count_R = 0;
int wheel_count_L = 0;

// Kp = 0.95, Ki = 0.01, Kd = 1
float Kp_L        = 0.95;
float Ki_L        = 0.01;
float Kd_L        = 1;
int 	integral_L 	= 0;
int 	lastError_L  = 0;
int 	lastPWM_L = 0;
float PWM_L = 0;
float error_L = 0;
float derivative_L = 0;

// Kp = 1.2, Ki = 0.01, Kd = 1.1
float Kp_R        = 1.2;
float Ki_R        = 0.01;
float Kd_R        = 0.8;
int 	integral_R 	= 0;
int 	lastError_R  = 0;
int 	lastPWM_R = 0;
float PWM_R = 0;
float error_R = 0;
float derivative_R = 0;

void clear_PID(){
		wheel_count_L = 0;
		wheel_count_R = 0;
		integral_L 	= 0;
		integral_R 	= 0;
		error_L = 0;
		error_R = 0;
		lastError_L  = 0;
		lastError_R  = 0;
		PWM_L = 0;
		PWM_R = 0;
		lastPWM_L = 0;
		lastPWM_R = 0;
		derivative_L = 0;
		derivative_R = 0;
}


void PID_Calcul_R(){
	lastError_R = error_R; //Store the last error
	lastPWM_R = PWM_R; //Store the last PWM
	
	error_R = target_count_R - wheel_count_R; // Caculate the present error
	integral_R += error_R; // Sum the error
	derivative_R = error_R - lastError_R;// Caculate the different between the present error and last error
	
	PWM_R = (lastPWM_R + (Kp_R * error_R) + (Ki_R*integral_R) + (Kd_R * derivative_R));
	
	if(PWM_R < 0){
		PWM_R = 0;
	}
	
	TIM3 -> CCR1 = PWM_R;
	wheel_count_R = 0;
}

void PID_Calcul_L(){
	lastError_L = error_L; //Store the last error
	lastPWM_L = PWM_L; //Store the last PWM
	
	error_L = target_count_L - wheel_count_L; // Caculate the present error
	integral_L += error_L; // Sum the error
	derivative_L = error_L - lastError_L; // Caculate the different between the present error and last error
	
	PWM_L = (lastPWM_L + (Kp_L * error_L) + (Ki_L*integral_L) + (Kd_L * derivative_L));
	
	if(PWM_L < 0){
		PWM_L = 0;
	}

	TIM3 -> CCR2 = PWM_L;
	wheel_count_L = 0;
}



// Right Wheel Counter EXTI
void EXTI1_IRQHandler(){
	if (EXTI_GetITStatus(EXTI_Line1) != RESET){
		wheel_count_R++;
		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}

unsigned int pressed = 0;
// Left Wheel Counter EXTI
void EXTI9_5_IRQHandler(){
	
	if(EXTI_GetITStatus(EXTI_Line6) != RESET){
		wheel_count_L++;
		EXTI_ClearITPendingBit(EXTI_Line6);
	}
	
	if(EXTI_GetITStatus(EXTI_Line8)!= RESET){
		clear_PID(); // Clear all PID variables
		clear_line_follower(); // clear all line follower variable
		if(pressed==0){
			if(isOn == 0){
				//TIM3 -> CCR1 = 70; // Give the right wheel a initial speed ???
				isOn = 1;
			}
			else{
				isOn = 0;
				// Set the speed to 0
				TIM3 -> CCR1 = 0;
				TIM3 -> CCR2 = 0;
			}
		}
		pressed ++;
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	else{
			pressed = 0;
	}
}


int flag = 0;
volatile unsigned int data1;
volatile unsigned int data2;
unsigned  int floor_data;

unsigned int b1=1,b2=1,b3=1,b4=1,b5=1,b6=1,b7=1,b8=1;
//  Dark Track, No reflection ->  0, ON Board LED on
//  White Track, has reflection ->  1, ON Board LED off
//	Photo Tran from left to right b1 b2 b3 b4 b5 b6 b7 b8
void track_detect(){
	
		b1=1,b2=1,b3=1,b4=1,b5=1,b6=1,b7=1,b8=1;
	
		GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPI2, '0');
		while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE) == RESET);
		data1 = SPI_I2S_ReceiveData(SPI2);
		SPI_I2S_ClearITPendingBit(SPI2,SPI_I2S_FLAG_RXNE);
	
		if ((data1 & (0xFF))== 0x00){
			b8 = 0;
			//GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
		}
		
		GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPI2, '0');
		while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE) == RESET);
		data2 = SPI_I2S_ReceiveData(SPI2);
		SPI_I2S_ClearITPendingBit(SPI2,SPI_I2S_FLAG_RXNE);

		if ((data2 & (1<<1)) == 0){
			b1 = 0;
			//GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
		}
		if ((data2 & (1<<2)) == 0){
			b2 = 0;
			//GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
		}
		if ((data2 & (1<<3)) == 0){
			b3 = 0;
			//GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
		}
		if ((data2 & (1<<4)) == 0){
			b4 = 0;
			//GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
		}
		if ((data2 & (1<<5)) == 0){
			b5 = 0;
			//GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
		}
		if ((data2 & (1<<6)) == 0){
			b6 = 0;
			//GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
		}
		if ((data2 & (1<<7)) == 0){
			b7 = 0;
			//GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
		}
		
		floor_data = data1 + (data2 >>1);
}

int get_number(){
	int number_of_bits = 0;
	
	if(b8 == 0){
		number_of_bits++;
	}
	
	for(int i=1; i<8; i++){
		if((data2 & (1<<i)) == 0){
			number_of_bits++;
		}
	}
	
	return number_of_bits;
}

void delay_check_time(int period){
			time_stamp = 0;
			delay_period = period;
			isDelay = true;
}

void check_flag_point(){
	if(!isDelay){
		if (get_number() > number_of_flag){
			count ++;
			delay_check_time(flag_check_time);
			GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
			if(count == 5){
				outLine = false;
				delay_turn2in = true;
			}
			if(count == 6){
				inner2outer = true;
			}
			
			if(count == 10){
				delay_turn2in = true;
				state = 3; // Change to the inner track again 
			}
			
			if(count == 11){
				delay_endPoint = true;
			}
		}
	}
}

// Outer line tracker follower 
void outer_tracker(){
	
		last_error_outer = error;
		if(b4 == 0){
			error = 1;
		}
		else if(b5 == 0){
			error = -1;
		}
		else if(b3 == 0){
			error = 3;
		}
		else if(b6 == 0){
			error = -3;
		}
		else if(b7 == 0){
			error = -5;
		}
		else if(b2 == 0){
			error = 5;
		}
		else if(b8 == 0){
			error = -7;
		}
		else if(b1 == 0){
			error = 7;
		}
		
		
	PWM_L = wheel_speed_outer - error * 4 - (error - last_error_outer)*1;
	PWM_R = wheel_speed_outer + error * 4 + (error - last_error_outer)*1;
	
	TIM3 -> CCR2 = PWM_L;
	TIM3 -> CCR1 = PWM_R;
}

// Inner line tracker follower 
void inner_tracker(){
	
	last_error_inner = error;
	
	if(b4 == 0){
		error = 5;
	}
	else if(b5 == 0){
		error = -5; 
	}
	else if(b3 == 0){
		error = 9; 
	}
	else if(b6 == 0){
		error = -9; // 5 * 4 = 20
	}
	else if(b2 == 0){
		error = 11; 
	}
	else if(b7 == 0){
		error = -11; // 6 * 4 = 24
	}
	else if(b1 == 0){
		error = 12; 
	}
	else if(b8 == 0){
		error = -12; // 7 * 4 = 28
	}
	
	PWM_L = 40 - error * 2 - (error - last_error_inner)*1;
	PWM_R = 40 + error * 2 + (error - last_error_inner)*1;

	TIM3 -> CCR2 = PWM_L;
	TIM3 -> CCR1 = PWM_R;
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
	
	while(1){
		if(flag == 1){
			
			track_detect();
			//inner_tracker();
			
			check_flag_point();
			
			
			if(state == 1){
				if (outLine == true){
						outer_tracker();
				}
				else{
						inner_tracker();
				}
				
				if(outer2inner){
					error = -11; //-11
					PWM_L = 40 - error * 2; //1
					PWM_R = 40 + error * 2; //2
					TIM3 -> CCR2 = PWM_L;
					TIM3 -> CCR1 = PWM_R;
					outer2inner = false;
					delay_O2I = true;
				}
				
				if(inner2outer){
					error = -6;
					PWM_L = 40 - error * 4;
					PWM_R = 40 + error * 4;
					TIM3 -> CCR2 = PWM_L;
					TIM3 -> CCR1 = PWM_R;
					inner2outer = false;
					delay_I2O = true;
				}
			}
			
			
			if(state == 2){
				if(startTurn){
					// Turn the direction
					GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_RESET);
					GPIO_WriteBit(GPIOC,GPIO_Pin_15,Bit_SET);
					TIM3-> CCR2 = 45;
					TIM3-> CCR1 = 45;
					
					startTurn = false;
					delay_turn2out = true;
				}
				else{
					GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_SET);
					GPIO_WriteBit(GPIOC,GPIO_Pin_15,Bit_SET);
					//number_of_flag = 3;
					outer_tracker();
				}
			}
			
			if(state == 3){
				if(outer2inner){
					error = 11;
					PWM_L = 40 - error * 2;
					PWM_R = 40 + error * 2;
					TIM3 -> CCR2 = PWM_L;
					TIM3 -> CCR1 = PWM_R;
					outer2inner = false;
					delay_O2I = true;
				}
				else{
					//number_of_flag = 4;
					inner_tracker();
				}
			}
			
			if(state == 4){
				TIM3 -> CCR2 = 0;
				TIM3 -> CCR1 = 0;
			}
			
			flag=0;
			}
		}
}

void TIM2_IRQHandler(){
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		
		GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_RESET);

		/*
		if(isOn == 1 && delay_O2I == false && delay_turn2out == false){
			flag = 1;
		}
		*/
		
		if(isOn == 1 && delay_O2I == false && delay_I2O == false && delay_turn2out == false){
			flag = 1;
		}
		
		
		// Time Delay to start turning to inner
		if(delay_turn2in == true){
			delay_turn2in_cnt ++;
			if(delay_turn2in_cnt == delay_turn2in_period){
				outer2inner = true;
				delay_turn2in = false;
				delay_turn2in_cnt = 0;
			}
		}
		
		// Time for turning 1st Outer to Inner
		if(delay_O2I == true){
			delay_O2I_cnt ++;
			if(delay_O2I_cnt == delay_O2I_period){
				delay_O2I = false;
				delay_O2I_cnt = 0;
			}
		}
		
		// Time Delay to stop for 1st Inner to Outer
		if(delay_I2O == true){
			delay_I2O_cnt ++;
			if(delay_I2O_cnt == delay_I2O_period){
				
				TIM3 -> CCR2 = 0;
				TIM3 -> CCR1 = 0;
				
				delay_I2O = false;
				delay_I2O_cnt = 0;
				
				startTurn = true;
				state = 2; // Change to the second state
			}
		}
		
		// Time for turning to outer
		if(delay_turn2out == true){
			delay_turn2out_cnt ++;
			if(delay_turn2out_cnt == delay_turn2out_period){
				delay_turn2out = false;
				delay_turn2out_cnt = 0;
			}
		}
		
		// Time Delay for turning to inner second time
		if(delay_endPoint == true){
			delay_endPoint_cnt ++;
			if(delay_endPoint_cnt == delay_endPoint_period){
				state = 4; // stop
				delay_endPoint = false;
				delay_endPoint_cnt = 0;
			}
		}
		
		// Time Delay for Checking the Flag Point
		time_stamp ++;
		if((isDelay==true) && (time_stamp == delay_period)){
			isDelay = false;
		}
		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
