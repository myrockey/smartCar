#ifndef __ULTRA_SONIC_H
#define __ULTRA_SONIC_H

//超声波模块
#include "stm32f10x.h"                  // Device header
#include "BitAction.h"
#include "Delay.h"
#include "motor.h"

#define Hcsr04_GPIO_CLK  RCC_APB2Periph_GPIOB
#define Hcsr04_GPIO      GPIOB
#define TRIG_Pin         GPIO_Pin_0
#define TRIG_Send        PBout(0)
#define Echo_Pin         GPIO_Pin_1        
#define ECHO_Reci        PBin(1)

void Ultrasonic_Init(void);
float Ultrasonic_Distance(void);
void Ultrasonic_Run(void);

#endif


