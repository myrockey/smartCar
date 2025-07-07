#include "tracking.h"
#include "motor.h"
#include "Delay.h"

extern int distance;

//3路寻迹模块
void Tracking_GPIO_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
	 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;		
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 	 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 
  GPIO_Init(GPIOB, &GPIO_InitStructure);			     
}

//寻迹运动
//根据线的宽度来决定；
/*
传感器布局
循迹模块通常包含多个红外传感器（如3路或5路），这些传感器分布在沿路径方向的不同位置，用于检测路径的左右偏移情况。例如：
三路循迹模块：左传感器、中传感器、右传感器。
五路循迹模块：左2传感器、左1传感器、中传感器、右1传感器、右2传感器。

1.传感器输出
循迹模块的传感器会根据接收到的红外光强度输出不同的电平信号：
白色路径：反射光强，输出高电平（通常为逻辑1）。
黑色路径：反射光弱，输出低电平（通常为逻辑0）。

2.路径检测逻辑
根据多个传感器的输出，可以判断路径的偏移方向：
直线路径：左右传感器都检测到白色路径，中传感器检测到黑色。
右偏移：左传感器检测到白色，中传感器和右传感器检测到黑色。
左偏移：右传感器检测到白色，中传感器和左传感器检测到黑色。
完全偏离：所有传感器都检测到黑色或白色。

3. 控制器响应
控制器（如微控制器）根据传感器的输出信号，调整机器人的运动方向：
直线行驶：左右传感器都检测到白色路径，中传感器检测到黑色路径时，机器人直线行驶。
右转：检测到右偏移时，机器人右转。
左转：检测到左偏移时，机器人左转。
原地转向：完全偏离路径时，机器人原地转向，重新寻找路径。
*/
void Tracking_Run(void )
{
  if(distance > 20){
    if(L==1 && M==0 && R==1)
    {
      Move_forward();
    }
    else if(L==1 && M==0 && R==0)
    {
      Turn_Right();
    }
    else if(L==1 && M==1 && R==0)
    {
      Turn_Right();
    }
    else if(L==0 && M==0 && R==1)
    {
      Turn_Left();
    }
    else if(L==0 && M==1 && R==1)
    {
      Turn_Left();
    }
    else if(L==0 && M==0 && R==0)
    {
      Clockwise_Rotation();
      Delay_ms(100);
      
      stop();
      Delay_ms(500);	
    }
    else if(L==1 && M==1 && R==1)
    {
      Clockwise_Rotation();
      Delay_ms(100);
      
      stop();
      Delay_ms(500);	
    }
  }
}