#ifndef __Buzzer_H
#define __Buzzer_H

// 配置舵机 SG90 , 红外遥控 hx1838
#define SG90_TIM_APBX       RCC_APB2PeriphClockCmd
#define SG90_TIM_CLK        RCC_APB2Periph_TIM1
#define SG90_TIM    	    TIM1		                /* 定时器1 */

//超声波
#define Ultrasonic_TIM_APBX 	  RCC_APB1PeriphClockCmd
#define Ultrasonic_TIM_CLK 	      RCC_APB1Periph_TIM3
#define Ultrasonic_TIM 	          TIM3
#define Ultrasonic_TIM_IRQn       TIM3_IRQn
#define Ultrasonic_TIM_IRQHandler TIM3_IRQHandler

//WIFI
#define WIFI_TIM_APBX 	    RCC_APB1PeriphClockCmd
#define WIFI_TIM_CLK 	    RCC_APB1Periph_TIM4
#define WIFI_TIM 	        TIM4
#define WIFI_TIM_IRQn       TIM4_IRQn
#define WIFI_TIM_IRQHandler TIM4_IRQHandler

void TIM_Servo(void);
void TIM_IR_NEC(void);
void TIM_Ultrasonic(void);
void TIM_WIFI_ENABLE_30S(void);
void TIM_WIFI_ENABLE_2S(void);

#endif

