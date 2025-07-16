#ifndef __Buzzer_H
#define __Buzzer_H

#define Buzzer_GPIO_APBX 	    RCC_APB2PeriphClockCmd
#define Buzzer_GPIO_CLK 	    RCC_APB2Periph_GPIOC		/* GPIO端口时钟 */
#define Buzzer_GPIO_PORT    	GPIOC		                /* GPIO端口 */
#define Buzzer_GPIO_PIN	        GPIO_Pin_13
                    
void Buzzer_Init(void);

#endif

