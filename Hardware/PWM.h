#ifndef __PWM_H
#define __PWM_H

// 配置电机 tb6612
#define TB6612_TIM_APBX       RCC_APB1PeriphClockCmd
#define TB6612_TIM_CLK        RCC_APB1Periph_TIM2
#define TB6612_GPIO_APBX      RCC_APB2PeriphClockCmd
#define TB6612_GPIO_CLK       RCC_APB2Periph_GPIOA       /* GPIO时钟 */
#define TB6612_GPIO_PORT    	 GPIOA		                /* GPIO端口 */
#define TB6612_GPIO_PIN_PWMA  GPIO_Pin_1
#define TB6612_GPIO_PIN_PWMB  GPIO_Pin_2
#define TB6612_TIM    	     TIM2		                /* 定时器2 */

// 配置舵机 SG90
#define SG90_TIM_APBX       RCC_APB2PeriphClockCmd
#define SG90_TIM_CLK        RCC_APB2Periph_TIM1
#define SG90_GPIO_APBX      RCC_APB2PeriphClockCmd
#define SG90_GPIO_CLK       RCC_APB2Periph_GPIOA       /* GPIO时钟 */
#define SG90_GPIO_PORT    	 GPIOA		                /* GPIO端口 */
#define SG90_GPIO_PIN       GPIO_Pin_11
#define SG90_TIM    	     TIM1		                /* 定时器2 */


void PWM_Init_Motor(void);
void PWM_Init_Servo(void);
void PWM_SetCompare2_Motor(uint16_t Compare);
void PWM_SetCompare3_Motor(uint16_t Compare);
void PWM_SetCompare4_Servo(uint16_t Compare);

#endif
