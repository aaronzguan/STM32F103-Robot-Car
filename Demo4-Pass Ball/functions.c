#include "stm32f10x.h"

//*********************************************************
//	Light Functions
//*********************************************************
void onLight(void){
	GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
}
void offLight(void){
	GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_RESET);
}
void blinkLight(void){
	if(GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_7)){
		GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_RESET);
	}else{
		GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_SET);
	}
}

//*********************************************************
//	Motor Wheel Functions
//*********************************************************
void setForwardRight(void){
	GPIO_WriteBit(GPIOC, GPIO_Pin_15, Bit_SET);
}
void setBackwardRight(void){
	GPIO_WriteBit(GPIOC, GPIO_Pin_15, Bit_RESET);
}
void setForwardLeft(void){	
		GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_SET);
}
void setBackwardLeft(void){	
		GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
}
void powerRight(int cycle){
	if(cycle < 0){
		setBackwardRight();
		TIM3 -> CCR1 = -1 * cycle;
	}else{
		setForwardRight();
		TIM3 -> CCR1 = cycle;
	}
}
void powerLeft(int cycle){
	if(cycle < 0){
		setBackwardLeft();
		TIM3 -> CCR2 = -1 * cycle;
	}else{
		setForwardLeft();
		TIM3 -> CCR2 = cycle;
	}
}
void stopMotor(void){
	TIM3 -> CCR1 = 0;
	TIM3 -> CCR2 = 0;
}


//*********************************************************
//	Convert Hex to Decimal value
//*********************************************************
int Hex2Dec(char temp){
	int val;
	
	if(temp >= '0' && temp <= '9'){
		val = temp - '0';
	}
	else if(temp >= 'a' && temp <= 'f'){
		val = temp - 'a' + 10;
	}
	return val;
}

//*********************************************************
//	USART2 Send
//*********************************************************
void USARTsend(char *pucBuffer, unsigned long ulCount){
	while(ulCount--){
		USART_SendData(USART2,*pucBuffer++);
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC) == RESET);
	}
}
