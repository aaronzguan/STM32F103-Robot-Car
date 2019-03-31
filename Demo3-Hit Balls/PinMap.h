void TIM3_CH12_PWM_init(void);
void SPI2_init(void);
void Right_Wheel_Cnt_init(void);
void Left_Wheel_Cnt_init(void);
void USART2_init(void);
void Wheel_Dir_Init(void);
void TIM2_Init(void);
void Button_init(void);
void OnBoard_lED_Init(void);
void USARTsend(char *pucBuffer, unsigned long ulCount);
void SysTick_Init(void);

// ----User Funcions---- //
void check_wifiData(void);
void USARTsend(char *pucBuffer, unsigned long ulCount);
int Hex2Dec(char temp);
void move(void);
void onLight(void);
void offLight(void);
void blinkLight(void);
void setForwardRight(void);
void setBackwardRight(void);
void setForwardLeft(void);
void setBackwardLeft(void);
void powerLeft(int cycle);
void powerRight(int cycle);
void stopMotor(void);
