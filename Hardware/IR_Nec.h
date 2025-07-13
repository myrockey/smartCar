#ifndef __IR_NEC_H
#define __IR_NEC_H

#define IR_POWER		0x45
#define IR_MODE			0x46
#define IR_MUTE			0x47
#define IR_START_STOP	0x44
#define IR_PREVIOUS		0x40
#define IR_NEXT			0x43
#define IR_EQ			0x07
#define IR_VOL_MINUS	0x15
#define IR_VOL_ADD		0x09
#define IR_0			0x16
#define IR_RPT			0x19
#define IR_USD			0x0D
#define IR_1			0x0C
#define IR_2			0x18
#define IR_3			0x5E
#define IR_4			0x08
#define IR_5			0x1C
#define IR_6			0x5A
#define IR_7			0x42
#define IR_8			0x52
#define IR_9			0x4A

/* ------------------ 用户可修改的宏 ------------------ */

//#define IR_TIM_APBX       RCC_APB2PeriphClockCmd
//#define IR_TIM_CLK        RCC_APB2Periph_TIM1
//#define IR_GPIO_APBX      RCC_APB2PeriphClockCmd
//#define IR_GPIO_CLK       RCC_APB2Periph_GPIOA       /* GPIO时钟 */
//#define IR_GPIO_PORT      GPIOA		                /* GPIO端口 */
//#define IR_GPIO_PIN       GPIO_Pin_11
//#define IR_TIM    	      TIM1		                /* 定时器1 */
//#define IR_IN_EXTI_Port   GPIO_PortSourceGPIOA        /* 外部中断 */
//#define IR_IN_EXTI_Pin    GPIO_PinSource11
//#define IR_IN_EXTI_Line   EXTI_Line11
//#define IR_IN_EXTI_IRQN   EXTI15_10_IRQn
//#define IR_EXTI_IRQHandler EXTI15_10_IRQHandler
/* --------------------------------------------------- */

void IR_Nec_Init(void);                 // 初始化
uint8_t IR_GetDataFlag(void);
uint8_t IR_GetRepeatFlag(void);
uint8_t IR_GetAddress(void);
uint8_t IR_GetCommand(void);

#endif
