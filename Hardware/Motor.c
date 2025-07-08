#include "Motor.h"
#include "PWM.h"

void Motor_Init(void)
{
	//开启RCC时钟 GPIOA
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	//GPIOA初始化
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;//PA4 PA5 PA6 PA7 引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	PWM_Init_Motor();//初始化直流电机的底层PWM
}

//左边电机
void MotorLeft_SetSpeed(int8_t Speed)
{
	if(Speed > 0)
	{
		//假设为正转
		GPIO_SetBits(GPIOA,GPIO_Pin_4);//置高电平
		GPIO_ResetBits(GPIOA,GPIO_Pin_5);//置低电平
		PWM_SetCompare2(Speed);
	}
	else if(Speed == 0)
	{
		//假设为正转
		GPIO_SetBits(GPIOA,GPIO_Pin_4);//置高电平
		GPIO_ResetBits(GPIOA,GPIO_Pin_5);//置低电平
		PWM_SetCompare2(Speed);
	}
	else
	{
		//反之则为反转
		GPIO_ResetBits(GPIOA,GPIO_Pin_4);//置低电平
		GPIO_SetBits(GPIOA,GPIO_Pin_5);//置高电平
		PWM_SetCompare2(-Speed);//PWM只能给正数
	}
}

//右边电机
void MotorRight_SetSpeed(int8_t Speed)
{
	if(Speed > 0)
	{
		//假设为正转
		GPIO_SetBits(GPIOA,GPIO_Pin_6);//置高电平
		GPIO_ResetBits(GPIOA,GPIO_Pin_7);//置低电平
		PWM_SetCompare3(Speed);
	}
	else if(Speed == 0)
	{
		//假设为正转
		GPIO_SetBits(GPIOA,GPIO_Pin_6);//置高电平
		GPIO_ResetBits(GPIOA,GPIO_Pin_7);//置低电平
		PWM_SetCompare3(Speed);
	}
	else
	{
		//反之则为反转
		GPIO_ResetBits(GPIOA,GPIO_Pin_6);//置低电平
		GPIO_SetBits(GPIOA,GPIO_Pin_7);//置高电平
		PWM_SetCompare3(-Speed);//PWM只能给正数
	}
}
