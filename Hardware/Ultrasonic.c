#include "stm32f10x.h"                  // Device header
#include "Timer.h"
#include "Ultrasonic.h"
#include "SmartCar.h"
#include "Delay.h"

extern int distance;//距障碍物距离

/*---------------- 仅需新增的全局变量 ----------------*/
volatile Ultrasonic_State_TypeDef ultrasonic_state = ULTRASONIC_IDLE;
static volatile int last_valid_distance = 0;   // 保存上一次有效距离(cm)
volatile uint32_t ic_rising = 0;
volatile uint32_t ic_falling = 0;
volatile uint8_t  ic_done   = 0;
volatile uint16_t ic_ovf    = 0;
static volatile uint32_t measure_start_time = 0; // 用于超时检测

volatile Ultrasonic_Avoidance_State_TypeDef ultrasonic_avoidance_state = AVOIDANCE_IDLE;
volatile uint32_t avoidance_start_time = 0;

//IO口初始化 及其他初始化
void Ultrasonic_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(Hcsr04_GPIO_CLK, ENABLE);
   
    GPIO_InitStructure.GPIO_Pin = TRIG_Pin;      
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(Hcsr04_GPIO, &GPIO_InitStructure);
    GPIO_ResetBits(Hcsr04_GPIO ,TRIG_Pin);
     
    GPIO_InitStructure.GPIO_Pin = Echo_Pin;     
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(Hcsr04_GPIO, &GPIO_InitStructure);  
    GPIO_ResetBits(Hcsr04_GPIO,Echo_Pin);     

    TIM_Ultrasonic();
    ultrasonic_state = ULTRASONIC_IDLE;
}

//获取定时器3计数器值
uint32_t GetEchoTimer(void)
{
    uint32_t t = 0;
   
    if (ic_falling >= ic_rising)
    {
        t = ic_falling - ic_rising;
    }
    else
    {
        t = (0xFFFF - ic_rising) + ic_falling + (uint32_t)ic_ovf * 0x10000UL;
    }

    return t;
}

// 触发超声波测量，非阻塞
void Ultrasonic_StartMeasure(void)
{
    if (ultrasonic_state == ULTRASONIC_IDLE || ultrasonic_state == ULTRASONIC_TIMEOUT) {
        ultrasonic_state = ULTRASONIC_TRIGGERING;
        TRIG_Send = 1;   // 给控制端高电平
        Delay_us(20);    // 保持20us高电平
        TRIG_Send = 0;   // 超声波模块已开始发送8个40khz脉冲

        ic_done = 0;
        ic_ovf  = 0;
        TIM_SetCounter(Ultrasonic_TIM, 0);
        measure_start_time = GetTick(); // 记录开始时间，用于超时检测
        ultrasonic_state = ULTRASONIC_WAITING_ECHO;
    }
}

// 在主循环中更新超声波状态和距离
uint32_t Ultrasonic_Distance(void)
{
    uint32_t current_distance = last_valid_distance;// 无论是否超时，只要还在等待回波，都返回上次有效距离
    if (ultrasonic_state == ULTRASONIC_WAITING_ECHO) {
        if (ic_done) {
            uint32_t t = GetEchoTimer();
            current_distance = (uint32_t)((float)t * 34 / 2000); // us -> cm
            last_valid_distance = current_distance;
            ultrasonic_state = ULTRASONIC_IDLE;
        } else if (GetTick() - measure_start_time > 30) { // 30ms 超时
            ultrasonic_state = ULTRASONIC_TIMEOUT;
        }
    }

    return current_distance;
}

//超声波避障
void Ultrasonic_Run(void)
{
    switch (ultrasonic_avoidance_state)
    {
        case AVOIDANCE_IDLE:
            if (distance > 30)
            {
                Move_Forward();
            }
            else if (distance <= 30 && distance > 7)
            {
                Move_Backward();
            }
            else if (distance <= 7)
            {
                Clockwise_Rotation();
                avoidance_start_time = GetTick();
                ultrasonic_avoidance_state = AVOIDANCE_ROTATING;
            }
            break;

        case AVOIDANCE_ROTATING:
            if (GetTick() - avoidance_start_time >= 100) // 旋转100ms
            {
                Car_Stop();
                avoidance_start_time = GetTick();
                ultrasonic_avoidance_state = AVOIDANCE_STOPPING;
            }
            break;

        case AVOIDANCE_STOPPING:
            if (GetTick() - avoidance_start_time >= 500) // 停止500ms
            {
                ultrasonic_avoidance_state = AVOIDANCE_IDLE;
            }
            break;

        default:
            ultrasonic_avoidance_state = AVOIDANCE_IDLE;
            break;
    }
}

//定时器3终中断
void Ultrasonic_TIM_IRQHandler(void)  
{
    /* 捕获中断 */
    if (TIM_GetITStatus(Ultrasonic_TIM, TIM_IT_CC4) != RESET)
    {
        TIM_ClearITPendingBit(Ultrasonic_TIM, TIM_IT_CC4);
        static uint8_t rising_captured = 0;
        if (!rising_captured)
        {
            /* 第一次上升沿 */
            ic_rising = TIM_GetCapture4(Ultrasonic_TIM);
            ic_ovf    = 0;
            rising_captured = 1;
            TIM_OC4PolarityConfig(Ultrasonic_TIM, TIM_ICPolarity_Falling);
        }
        else
        {
            /* 下降沿 */
            ic_falling = TIM_GetCapture4(Ultrasonic_TIM);
            ic_done    = 1;
            rising_captured = 0;
            TIM_OC4PolarityConfig(Ultrasonic_TIM, TIM_ICPolarity_Rising);
        }
    }

    /* 溢出中断：扩展计时范围 */
    if (TIM_GetITStatus(Ultrasonic_TIM, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(Ultrasonic_TIM, TIM_IT_Update);
        ic_ovf++;
    }
}
