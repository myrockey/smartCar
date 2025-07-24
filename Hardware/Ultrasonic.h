#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

#include "BitAction.h"

//超声波模块
#define Hcsr04_GPIO_APBX RCC_APB2PeriphClockCmd
#define Hcsr04_GPIO_CLK  RCC_APB2Periph_GPIOB
#define Hcsr04_GPIO      GPIOB
#define TRIG_Pin         GPIO_Pin_0
#define TRIG_Send        PBout(0)
#define Echo_Pin         GPIO_Pin_1        
#define ECHO_Recieve     PBin(1)

typedef enum
{
    ULTRASONIC_IDLE = 0,        // 空闲状态
    ULTRASONIC_TRIGGERING,    // 触发中（发送脉冲）
    ULTRASONIC_WAITING_ECHO,  // 等待回波
    // ULTRASONIC_DONE,          // 测量完成 (此行可以移除)
    ULTRASONIC_TIMEOUT        // 测量超时
} Ultrasonic_State_TypeDef;

typedef enum
{
    AVOIDANCE_IDLE = 0,         // 空闲状态
    AVOIDANCE_ROTATING,       // 旋转中
    AVOIDANCE_STOPPING        // 停止中
} Ultrasonic_Avoidance_State_TypeDef;

extern volatile Ultrasonic_Avoidance_State_TypeDef ultrasonic_avoidance_state;
extern volatile uint32_t avoidance_start_time;

void Ultrasonic_Init(void);
void Ultrasonic_StartMeasure(void);
uint32_t Ultrasonic_Distance(void);
void Ultrasonic_Run(void);

#endif


