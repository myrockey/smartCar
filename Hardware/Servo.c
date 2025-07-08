#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Servo_Init(void)
{
	PWM_Init_Servo();
}

/*
0度   500
180度 2500
*/
void Servo_SetAngle(float Angle)
{
	PWM_SetCompare4(Angle/180*2000+500);
}