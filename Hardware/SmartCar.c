#include "stm32f10x.h"                  // Device header
#include "Motor.h"

int8_t Speed = 50;
/**
  * ��    ������ʼ��
  * ��    ������
  * �� �� ֵ����
  */
void SmartCar_Init(void)
{
	Motor_Init();
}

//��ǰ
void Move_Forward(void)
{
	MotorLeft_SetSpeed(Speed);
	MotorRight_SetSpeed(Speed);
}

//���
void Move_Backward(void)
{
	MotorLeft_SetSpeed(-Speed);
	MotorRight_SetSpeed(-Speed);
}

//ֹͣ
void Car_Stop(void)
{
	MotorLeft_SetSpeed(0);
	MotorRight_SetSpeed(0);
}

//����ת
void Turn_Left(void)
{
	MotorLeft_SetSpeed(Speed-40);
	MotorRight_SetSpeed(Speed);
}

//����ת
void Turn_Right(void)
{
	MotorLeft_SetSpeed(Speed);
	MotorRight_SetSpeed(Speed-40);
}

//˳ʱ����ת
void Clockwise_Rotation(void)
{
	MotorLeft_SetSpeed(Speed);
	MotorRight_SetSpeed(-Speed);
}


//��ʱ����ת
void CounterClockwise_Rotation(void)
{
	MotorLeft_SetSpeed(-Speed);
	MotorRight_SetSpeed(Speed);
}
