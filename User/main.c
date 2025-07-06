#include "stm32f10x.h"                  // Device header
#include "Motor.h"
#include "OLED.h"
#include "Key.h"

uint8_t KeyNum;
int8_t Speed;
uint16_t Temp;

int main(void)
{
	Motor_Init();
	OLED_Init();
	Key_Init();
	
	OLED_ShowString(1,1,"Speed:");
	while(1)
	{
		KeyNum = Key_GetNum();
		if(KeyNum == 1)
		{
			Temp += 20;
			if(Temp > 200)
			{
			
				//Speed = -100;//此操作会让电机旋转方向突然改变，可能会因供电不足而导致单片机复位
				Temp = 0;			 //若出现了此现象，则应避免使用这样的操作
			}
		}
		Speed = Temp <= 100 ? Temp : 100 - Temp;
		Motor_SetSpeed(Speed);
		OLED_ShowSignedNum(1,7,Speed,3);
	}
	
}


