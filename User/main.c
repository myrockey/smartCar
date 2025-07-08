#include "stm32f10x.h"                  // Device header
#include <string.h>
#include "OLED.h"
#include "Delay.h"
#include "SmartCar.h"
#include "Bluetooth.h"
#include "tracking.h"
#include "Buzzer.h"
#include "LED.h"
#include "VoiceIdentify.h"

uint8_t RxData;//串口接收数据的变量
int tmp;//记录循迹位置值的变量
int distance;//离障碍物距离
char str[20]; // 定义一个长度为20的字符数组作为字符串

//初始化
void BSP_Init(void);

//根据参数，执行对应功能
void Exec_Function(uint8_t type, char str[]);

//根据参数，播报语音
void Voice_broadcast(uint8_t type);

int main(void)
{
	BSP_Init();

	while(1)
	{
		tmp = (L * 100)+ (M * 10) + (R * 1);
		distance = Ultrasonic_Distance();

        OLED_ShowNum(2,4,distance,3);
        OLED_ShowNum(3,4,tmp,3);
		
		// 接收到数据
		if(Serial_GetRxFlag() == 1)
		{
			RxData = Serial_GetRxData();
		}

		Exec_Function(RxData, str);
		OLED_ShowString(1,4,str);

		Voice_broadcast(RxData);
		
		//距离太近时
		if(distance < 10)
        {
            Buzzer_OFF;
            LED1_OFF;
        }
        if(distance > 10)
        {
            Buzzer_ON;
            LED1_ON;
        }

	}
}

void BSP_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//配置NVIC中断为分组4
	OLED_Init();//显示屏初始化
	SmartCar_Init();//电机驱动初始化
	Bluetooth_Init();//蓝牙初始化
	Ultrasonic_Init();//超声波初始化
	Tracking_Init();//循迹初始化
	Buzzer_Init();//蜂鸣器初始化
	LED_Init();//LED初始化
	VoiceIdentify_Init();//语音识别初始化
}

/*
*根据参数，执行对应功能。
具体原理：蓝牙模块或语音识别模块（发送方） 通过串口传输数据 stm32（接收方），根据接收的数据，执行对应功能。
*/
void Exec_Function(uint8_t type, char str[])
{
	switch(RxData)
	{
		case 1:
			Move_Forward();
			strcpy(str, "forword ");
			break;
		case 2:
			Move_Backward();
			strcpy(str, "backword");
			break;
		case 3:
			Car_Stop();
			strcpy(str, "  stop  ");
			break;
		case 4:
			Turn_Left();
			strcpy(str, "  left  ");
			break;
		case 5:
			Turn_Right();
			strcpy(str, " right  ");
			break;
		case 6://顺时针旋转
			Clockwise_Rotation();
			strcpy(str, " cycle  ");
			break;
		case 7://逆时针旋转
			CounterClockwise_Rotation();
			strcpy(str, " Ncycle ");
			break;
		case 8://超声波避障
			Ultrasonic_Run();
			strcpy(str, " sonic  ");
			break;
		case 9://循迹
			Tracking_Run();
			strcpy(str, " sonic  ");
			break;
		case 10://LED ON
			LED1_ON;LED2_ON;LED3_ON;
			strcpy(str, " led on ");
			break;
		case 11://LED OFF
			LED1_OFF;LED2_OFF;LED3_OFF;
			strcpy(str, " led off ");
			break;
		case 12://Buzzer ON
			Buzzer_ON;
			strcpy(str, " buzzer ");
			break;
		case 13://Buzzer OFF
			Buzzer_OFF;
			strcpy(str, " buzzer ");
			break;
	}
}

/*
*根据参数，播报语音。
具体原理：stm32（发送方）通过串口传输数据 语音识别模块ASRPRO（接收方），根据接收的数据，执行语音播报。
*/
void Voice_broadcast(uint8_t type)
{
	switch(type)
	{
		case 10://LED ON
			VoiceIdentify_SendString("LED ON");
			break;
		case 11://LED OFF
			VoiceIdentify_SendString("LED OFF");
			break;
	}
}