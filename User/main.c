#include "stm32f10x.h"                  // Device header
#include "SmartCar.h"

int main(void)
{
	SmartCar_Init();
	Clockwise_Rotation();
	while(1)
	{
		
	}
	
}
