#ifndef __TRACKING_H
#define __TRACKING_H

#include "stm32f10x.h"                  // Device header

//三路循迹
#define   R   GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3)
#define   M   GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) 
#define   L   GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) 

void Tracking_Init(void);
void Tracking_Run(void);

#endif

