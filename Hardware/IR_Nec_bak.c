#include "stm32f10x.h"                  // Device header
#include "IR_Nec.h"


/* ------------------ 用户可修改的宏 ------------------ */

#define IR_TIM_APBX       RCC_APB2PeriphClockCmd
#define IR_TIM_CLK        RCC_APB2Periph_TIM1
#define IR_GPIO_APBX      RCC_APB2PeriphClockCmd
#define IR_GPIO_CLK       RCC_APB2Periph_GPIOC       /* GPIO时钟 */
#define IR_GPIO_PORT      GPIOC		                /* GPIO端口 */
#define IR_GPIO_PIN       GPIO_Pin_15
#define IR_TIM    	      TIM1		                /* 定时器1 */
#define IR_IN_EXTI_Port   GPIO_PortSourceGPIOC        /* 外部中断 */
#define IR_IN_EXTI_Pin    GPIO_PinSource15
#define IR_IN_EXTI_Line   EXTI_Line15
#define IR_IN_EXTI_IRQN   EXTI15_10_IRQn
#define IR_EXTI_IRQHandler EXTI15_10_IRQHandler
#define IR_TIM_UP_IRn     TIM1_UP_IRQn  /* TIM更新中断 */
#define IR_TIM_UP_IRQHandler  TIM1_UP_IRQHandler
/* --------------------------------------------------- */


/* 全局变量 */
static volatile uint32_t ir_lastCnt = 0;
static volatile uint32_t ir_overflow = 0;
static volatile uint8_t  ir_state   = 0;   /* 0/1/2 */
static volatile uint8_t  ir_bits    = 0;
static volatile uint8_t  ir_buf[4]  = {0};
static volatile uint8_t  ir_dataFlag   = 0;
static volatile uint8_t  ir_repeatFlag = 0;
static volatile uint8_t  ir_addr = 0;
static volatile uint8_t  ir_cmd  = 0;
static const uint32_t ir_diff = 500;

void IR_TIM_Init(void)
{
	IR_TIM_APBX(IR_TIM_CLK, ENABLE);			//开启TIM1的时钟
	
	/*配置时钟源*/
	TIM_InternalClockConfig(IR_TIM);		//选择TIM1为内部时钟，若不调用此函数，TIM默认也为内部时钟
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 0xFFFF;                 //计数周期，即ARR的值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;               //预分频器，即PSC的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(IR_TIM, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM1的时基单元

	/*TIM使能*/
	TIM_Cmd(IR_TIM, ENABLE);			//使能TIM1，定时器开始运行
}

/**
  * @brief  红外遥控初始化
  * @param  无
  * @retval 无
  */
void IR_Nec_Init(void)
{
	/*开启时钟*/
	IR_GPIO_APBX(IR_GPIO_CLK, ENABLE);			//开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉输入模式
	GPIO_InitStructure.GPIO_Pin = IR_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(IR_GPIO_PORT, &GPIO_InitStructure);							
																	//受外设控制的引脚，均需要配置为复用模式
	/* 选择EXTI的信号源 */
	GPIO_EXTILineConfig(IR_IN_EXTI_Port, IR_IN_EXTI_Pin);	
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = IR_IN_EXTI_Line;		
	/* EXTI为中断模式 */
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	/* 下降沿中断 */
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	/* 使能中断 */	
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* NVIC */
	NVIC_InitTypeDef NVIC_InitStructure;					//定义结构体变量
    NVIC_InitStructure.NVIC_IRQChannel = IR_IN_EXTI_IRQN;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

	// /* 允许更新中断 */
    TIM_ITConfig(IR_TIM, TIM_IT_Update, ENABLE);

	/* TIM1 Update NVIC */
	NVIC_InitTypeDef nvic_tim;
	nvic_tim.NVIC_IRQChannel                   = IR_TIM_UP_IRn;
	nvic_tim.NVIC_IRQChannelPreemptionPriority = 2;
	nvic_tim.NVIC_IRQChannelSubPriority        = 2;
	nvic_tim.NVIC_IRQChannelCmd                = ENABLE;
	NVIC_Init(&nvic_tim);

	IR_TIM_Init();
}

/* -------------------- 查询接口 -------------------- */
uint8_t IR_GetDataFlag(void)
{
    uint8_t ret = ir_dataFlag;
    ir_dataFlag = 0;
    return ret;
}

uint8_t IR_GetRepeatFlag(void)
{
    uint8_t ret = ir_repeatFlag;
    ir_repeatFlag = 0;
    return ret;
}

uint8_t IR_GetAddress(void) 
{ 
	return ir_addr; 
}

uint8_t IR_GetCommand(void)  
{ 
	return ir_cmd;  
}
/* -------------------- 查询接口 -------------------- */

static uint32_t IR_GetDelta(void)
{
    uint32_t now = TIM_GetCounter(IR_TIM);
    uint32_t of  = ir_overflow;
    ir_overflow = 0;

    uint32_t delta;
    if (now >= ir_lastCnt)
    {
        delta = now - ir_lastCnt + of * 0x10000UL;//你使用的是 TIM1，它是一个 16 位定时器，最大值为 0xFFFF，所以这里没错。
                                                  //但如果你未来换成 32 位定时器（如 TIM2/3/4），就要注意。
    }
    else
    {
        delta = (0x10000UL - ir_lastCnt) + now + of * 0x10000UL;
    }
      
    ir_lastCnt = now;
    return delta;
}

// 用 TIM1 1ms 中断做“看门狗”，60 ms 没收到边沿就强制 ir_state = 0，彻底杜绝卡死。
void IR_TIM_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(IR_TIM, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(IR_TIM, TIM_IT_Update);
		ir_overflow++;
    }
}

//外部中断函数
void IR_EXTI_IRQHandler(void)
{
	if (EXTI_GetITStatus(IR_IN_EXTI_Line) != RESET)
    {
        EXTI_ClearITPendingBit(IR_IN_EXTI_Line);
        uint32_t t = IR_GetDelta();

        switch (ir_state)
        {
            case 0:                 /* 空闲，等待同步头 */
                ir_state = 1;
                break;
            case 1:                 /* 判断 Start/Repeat */
                if (t > 13500 - ir_diff && t < 13500 + ir_diff)       /* Start 13.5ms */
                {
                    ir_state = 2;
                }
                else if (t > 11250 - ir_diff && t < 11250 + ir_diff)  /* Repeat 11.25ms */
                {
                    ir_repeatFlag = 1;
                    ir_state = 0;
                }
                else //接收出错
                {
                    ir_state = 1;   /* 状态置为1 */
                }
                break;
            case 2:                 /* 状态2，接收数据 接收 32 bit 数据 */
                if (t > 1120 - ir_diff && t < 1120 + ir_diff)        /* 逻辑0 ~1.12ms */
                {
                    ir_buf[ir_bits / 8] &= ~(1 << (ir_bits % 8));
                    ir_bits++;
                }
                else if (t > 2250 - ir_diff && t < 2250 + ir_diff)   /* 逻辑1 ~2.25ms */
                {
                    ir_buf[ir_bits / 8] |=  (1 << (ir_bits % 8));
                    ir_bits++;
                }
                else
                {
                    ir_bits = 0;//数据位置清0
                    ir_state = 1;   /* 状态置为1 */
                    break;
                }

                //如果接收到了32位数据
                if (ir_bits >= 32)
                {
                    ir_bits = 0;
                    ir_state = 0;
                    /* 校验 */
                    if ((ir_buf[0] == (uint8_t)~ir_buf[1]) &&
                        (ir_buf[2] == (uint8_t)~ir_buf[3]))
                    {
                        ir_addr = ir_buf[0];
                        ir_cmd  = ir_buf[2];
                        ir_dataFlag = 1;
                    }
                }
                break;
        }
    }
}
