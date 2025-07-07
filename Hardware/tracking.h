#ifndef __TRACKING_H
#define __TRACKING_H

#include "stm32f10x.h"                  // Device header

//三路循迹
#define   R   GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4)
#define   M    GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) 
#define   L   GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) 

void Tracking_GPIO_Config(void);
void Tracking_Run(void);

#endif

