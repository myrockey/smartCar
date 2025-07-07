#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"                  // Device header
// 配置蓝牙模块
#include <stdio.h>

//使用宏定义容易修改串口
#define HC_05_GPIO_PORT    	GPIOA		                /* GPIO端口 */
#define HC_05_GPIO_CLK 	    RCC_APB2Periph_GPIOA		/* GPIO端口时钟 */

#define RX_GPIO_PIN	        GPIO_Pin_10	
#define TX_GPIO_PIN	        GPIO_Pin_9

#define HC_05_USARTX        USART1
#define HC_05_CLK           RCC_APB2Periph_USART1

void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);//uint8_t = char 
void Serial_SendNumber(uint32_t Number,uint8_t Length);
void Serial_SendString(char *String);
void Serial_Printf(char *format,...);

uint8_t Serial_GetRxFlag(void);
uint8_t Serial_GetRxData(void);

#endif
