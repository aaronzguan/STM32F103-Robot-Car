#include "stm32f10x.h"                  // Device header

//Wheel PWM Init
//500 HZ PWM and Duty cycle is 50%
void TIM3_CH12_PWM_init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	// PA6 Init for PWM CH1 Right Wheel
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	// PA7 Init for PWM CH2 Left Wheel
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//Tim3 Set up
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_InitStruct;
	TIM_InitStruct.TIM_Prescaler = 1440-1;
	TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStruct.TIM_Period = 100-1;
	TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1; 
	TIM_InitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3,&TIM_InitStruct);
	TIM_Cmd(TIM3,ENABLE);
	
	//Enable Tim3 Ch1 PWM Right-Wheel
	TIM_OCInitTypeDef OC_InitStruct;
	OC_InitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	OC_InitStruct.TIM_Pulse = 1-1;//100-1; // 1000 / 2000 = 0.5 duty cycle
	OC_InitStruct.TIM_OutputState = TIM_OutputState_Enable;
	OC_InitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM3,&OC_InitStruct);
	TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Enable);
	
	//Enable Tim3 Ch2 PWM Left-Wheel
	OC_InitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	OC_InitStruct.TIM_Pulse = 1-1;//100-1; // 1000 / 2000 = 0.5 duty cycle
	OC_InitStruct.TIM_OutputState = TIM_OutputState_Enable;
	OC_InitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC2Init(TIM3,&OC_InitStruct);
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
}

// SPI MASTER init
// PB12 IR LED  SS
// PB13 SPI2_SCK  clock signal from master to floor // Send the clock to floor
// PB14 SPI2_MISO master in floor out  // where the data of photo transistor is from
// PB15 SPI2_MOSI master out floor in // No use

void SPI2_init(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	
	//SPI Configuration
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(SPI2, &SPI_InitStructure);
	// Enable SPI2
	SPI_Cmd(SPI2, ENABLE);
	
	
	// Enable Receive Interrupt
	/*
	NVIC_InitTypeDef NVIC_InitStructure;
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE|SPI_I2S_IT_TXE, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	*/
	
	
}

// Right Wheel Counter Init PA1 Timer2
void Right_Wheel_Cnt_init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	// PA1 Init for Right Wheel Counter
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	// EXTI Configuration
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line1;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	
	// Enable Interrupt
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_Init(&NVIC_InitStruct);
	
	//NVIC_EnableIRQ(EXTI1_IRQn);
}

// Left Wheel Counter Init PB6 Timer4
void Left_Wheel_Cnt_init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB| RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	// PB6 Init for Left Wheel Counter
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	// EXTI Configuration
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line6;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	
	// Enable Interrupt
	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_Init(&NVIC_InitStruct);
	
	//NVIC_EnableIRQ(EXTI9_5_IRQn);
}


void USART2_init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//USART2 ST-LINK USB
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	USART_Cmd(USART2, ENABLE);
}

// PC15 Right Dir
// PA0 Left Dir
void Wheel_Dir_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	//PC15
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC,&GPIO_InitStruct);
	
	//PA0
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
}

void TIM2_Init(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_InitStruct;
	TIM_InitStruct.TIM_Prescaler = 720-1;  
	TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStruct.TIM_Period = 7500-1; // 75ms // 
	TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1; 
	TIM_InitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2,&TIM_InitStruct);
	TIM_Cmd(TIM2,ENABLE);
	
	//Enable update event for Timer2
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	NVIC_EnableIRQ(TIM2_IRQn);
}

// On-board button init PB8
void Button_init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
		// EXTI Configuration
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line8;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	
	NVIC_EnableIRQ(EXTI9_5_IRQn);
}

// On-board LED init PB7
void OnBoard_lED_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}