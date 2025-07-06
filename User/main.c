#include "stm32f10x.h"                  // Device header
#include "SmartCar.h"
#include "Serial.h"

uint8_t RxData;

int main(void)
{
	SmartCar_Init();
	Serial_Init();
	
	while(1)
	{
		if(Serial_GetRxFlag() == 1)
		{
			RxData = Serial_GetRxData();
		}
		 
		switch(RxData)
		{
			case 1:
				Move_Forward();
				break;
			case 2:
				Move_Backward();
				break;
			case 3:
				Car_Stop();
				break;
			case 4:
				Turn_Left();
				break;
			case 5:
				Turn_Right();
				break;
			case 6:
				Clockwise_Rotation();
				break;
			case 7:
				CounterClockwise_Rotation();
				break;
		}
	}
}
