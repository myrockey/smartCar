#ifndef __Buzzer_H
#define __Buzzer_H

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

void TIM_Ultrasonic(void);

#endif

