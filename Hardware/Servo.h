#ifndef __SERVO_H_
#define __SERVO_H_

#include "stm32f10x.h"                  // Device header

//舵机
void Servo_Init(void);
void Servo_SetAngle(float Angle);

#endif
