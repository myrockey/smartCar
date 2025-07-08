#include "stm32f10x.h"                  // Device header
#include "Motor.h"
#include "PWM.h"

void Motor_Init(void)
{
	//开启RCC时钟 GPIOA
	TB6612_Motor_GPIO_APBX(TB6612_Motor_GPIO_CLK, ENABLE);
	
	//GPIOA初始化
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStructure.GPIO_Pin = TB6612_GPIO_PIN_AIN1 | TB6612_GPIO_PIN_AIN2 | TB6612_GPIO_PIN_BIN1 | TB6612_GPIO_PIN_BIN2;//PA4 PA5 PA6 PA7 引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(TB6612_Motor_GPIO_PORT,&GPIO_InitStructure);
	
	PWM_Init_Motor();//初始化直流电机的底层PWM
}

//左边电机
void MotorLeft_SetSpeed(int8_t Speed)
{
	if(Speed > 0)
	{
		//假设为正转
		GPIO_SetBits(TB6612_Motor_GPIO_PORT,TB6612_GPIO_PIN_AIN1);//置高电平
		GPIO_ResetBits(TB6612_Motor_GPIO_PORT,TB6612_GPIO_PIN_AIN2);//置低电平
		PWM_SetCompare2_Motor(Speed);
	}
	else if(Speed == 0)
	{
		//假设为正转
		GPIO_SetBits(TB6612_Motor_GPIO_PORT,TB6612_GPIO_PIN_AIN1);//置高电平
		GPIO_ResetBits(TB6612_Motor_GPIO_PORT,TB6612_GPIO_PIN_AIN2);//置低电平
		PWM_SetCompare2_Motor(Speed);
	}
	else
	{
		//反之则为反转
		GPIO_ResetBits(TB6612_Motor_GPIO_PORT,TB6612_GPIO_PIN_AIN1);//置低电平
		GPIO_SetBits(TB6612_Motor_GPIO_PORT,TB6612_GPIO_PIN_AIN2);//置高电平
		PWM_SetCompare2_Motor(-Speed);//PWM只能给正数
	}
}

//右边电机
void MotorRight_SetSpeed(int8_t Speed)
{
	if(Speed > 0)
	{
		//假设为正转
		GPIO_SetBits(TB6612_Motor_GPIO_PORT,TB6612_GPIO_PIN_BIN1);//置高电平
		GPIO_ResetBits(TB6612_Motor_GPIO_PORT,TB6612_GPIO_PIN_BIN2);//置低电平
		PWM_SetCompare1_Motor(Speed);
	}
	else if(Speed == 0)
	{
		//假设为正转
		GPIO_SetBits(TB6612_Motor_GPIO_PORT,TB6612_GPIO_PIN_BIN1);//置高电平
		GPIO_ResetBits(TB6612_Motor_GPIO_PORT,TB6612_GPIO_PIN_BIN2);//置低电平
		PWM_SetCompare1_Motor(Speed);
	}
	else
	{
		//反之则为反转
		GPIO_ResetBits(TB6612_Motor_GPIO_PORT,TB6612_GPIO_PIN_BIN1);//置低电平
		GPIO_SetBits(TB6612_Motor_GPIO_PORT,TB6612_GPIO_PIN_BIN2);//置高电平
		PWM_SetCompare1_Motor(-Speed);//PWM只能给正数
	}
}
