#include "HC_05.h"
#include "Serial.h"

void HC_05_Init(void)
{
	Serial_Init_HC_05();
}

//发送数据
void HC_05_SendString(char *String)
{
	Serial_SendString(HC_05_USARTX, String);
}