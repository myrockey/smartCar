#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Motor_Init(void)
{
	//开启RCC时钟 GPIOA
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	//GPIOA初始化
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;//PA4 PA5 引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	PWM_Init();//初始化直流电机的底层PWM
}

void Motor_SetSpeed(int8_t Speed)
{
	if(Speed > 0)
	{
		//假设为正转
		GPIO_SetBits(GPIOA,GPIO_Pin_4);//置高电平
		GPIO_ResetBits(GPIOA,GPIO_Pin_5);//置低电平
		PWM_SetCompare3(Speed);
	}
	else
	{
		//反之则为反转
		GPIO_ResetBits(GPIOA,GPIO_Pin_4);//置低电平
		GPIO_SetBits(GPIOA,GPIO_Pin_5);//置高电平
		PWM_SetCompare3(-Speed);//PWM只能给正数
	}
}
