#include "stm32f10x.h"                  // Device header
#include "Bluetooth.h"
#include "Serial.h"

void WIFI_Init(void)
{
	Serial_Init_ESP8266();
}

//发送数据
void WIFI_SendString(char *String)
{
	Serial_SendString(ESP8266_USARTX, String);
}