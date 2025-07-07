#ifndef __Buzzer_H
#define __Buzzer_H

//蜂鸣器
#include "stm32f10x.h"                  // Device header


#define Buzzer_GPIO_PORT    	GPIOA		                /* GPIO端口 */
#define Buzzer_GPIO_CLK 	    RCC_APB2Periph_GPIOA		/* GPIO端口时钟 */
#define Buzzer_GPIO_PIN	        GPIO_Pin_7	


#define Buzzer_OFF             GPIO_ResetBits(Buzzer_GPIO_PORT,Buzzer_GPIO_PIN)
#define Buzzer_ON			   GPIO_SetBits(Buzzer_GPIO_PORT,Buzzer_GPIO_PIN)
                    
void Buzzer_Init(void);

#endif

