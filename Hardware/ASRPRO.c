#include "stm32f10x.h"                  // Device header
#include "ASRPRO.h"
#include "Serial.h"

void ASRPRO_Init(void)
{
	Serial_Init_ASRPRO();
}

//发送数据
void ASRPRO_SendString(char *String)
{
	Serial_SendString(ASRPRO_USARTX, String);
}