#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"                  // Device header

void Motor_Init(void);
//左电机
void MotorLeft_SetSpeed(int8_t Speed);
//右电机
void MotorRight_SetSpeed(int8_t Speed);
#endif
