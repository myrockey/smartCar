#include "stm32f10x.h"                  // Device header
#include "IR_Nec.h"


/* ------------------ 用户可修改的宏 ------------------ */

#define IR_TIM_APBX       RCC_APB1PeriphClockCmd
#define IR_TIM_CLK        RCC_APB1Periph_TIM3
#define IR_GPIO_APBX      RCC_APB2PeriphClockCmd
#define IR_GPIO_CLK       RCC_APB2Periph_GPIOC       /* GPIO时钟 */
#define IR_GPIO_PORT      GPIOC	                     /* GPIO端口 */
#define IR_GPIO_PIN       GPIO_Pin_15
#define IR_TIM    	      TIM3		                /* 定时器1 */
#define IR_IN_EXTI_Port   GPIO_PortSourceGPIOC        /* 外部中断 */
#define IR_IN_EXTI_Pin    GPIO_PinSource15
#define IR_IN_EXTI_Line   EXTI_Line15
#define IR_IN_EXTI_IRQN   EXTI15_10_IRQn
#define IR_EXTI_IRQHandler EXTI15_10_IRQHandler
#define IR_TIM_IRn     TIM3_IRQn  /* TIM更新中断 */
#define IR_TIM_IRQHandler  TIM3_IRQHandler
/* --------------------------------------------------- */

#define IR_UP    0x18
#define IR_DOWN  0x52
#define IR_OK    0x1C
#define IR_LEFT  0x08
#define IR_RIGHT 0x5A

/* 全局变量 */
static volatile uint32_t ir_count = 0;
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
    // APB1 是 36MHz 但是定时器的时钟频率计算是根据 （预分频系数、倍频器 决定的）。
    /* 当预分频系数等于1时，倍频器不x2，等预分频系数不等于1时，倍频器x2 
    举例：系统时钟72MHz,APB1 为36MHz，APB2为72MHz 注意：TIM1 在APB2总线上。TIM2,TIM3,TIM4 在APB1总线上。
    1.预分频系数为 1，则 TIM3定时器的时钟频率: APB1时钟频率 * 倍频器 / 预分频系数 =  36MHz * 1 / 1 = 36MHz
    2.预分频系数为 2，则 TIM3定时器的时钟频率: APB1时钟频率 * 倍频器 / 预分频系数 =  36MHz * 2 / 2 = 36MHz
    3.预分频系数为 72，则 TIM3定时器的时钟频率: APB1时钟频率 * 倍频器 / 预分频系数 =  36MHz * 2 / 72 = 1MHz
    */
	IR_TIM_APBX(IR_TIM_CLK, ENABLE);			//开启TIM1的时钟
	
	/*配置时钟源*/
	TIM_InternalClockConfig(IR_TIM);		//选择TIM1为内部时钟，若不调用此函数，TIM默认也为内部时钟
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 0xFFFF;                 //计数周期，即ARR的值  1个技术周期的时间是 0.001ms*ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;               //预分频器，即PSC的值 频率1MHz 时间就是 1000ms/1000000 = 0.001 ms
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(IR_TIM, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM1的时基单元

	/*TIM使能*/
    TIM_Cmd(IR_TIM, DISABLE);//注意:先不开启
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

	// // /* 允许更新中断 */
    // TIM_ITConfig(IR_TIM, TIM_IT_Update, ENABLE);

	// /* TIM1 Update NVIC */
	// NVIC_InitTypeDef nvic_tim;
	// nvic_tim.NVIC_IRQChannel                   = IR_TIM_IRn;
	// nvic_tim.NVIC_IRQChannelPreemptionPriority = 2;
	// nvic_tim.NVIC_IRQChannelSubPriority        = 2;
	// nvic_tim.NVIC_IRQChannelCmd                = ENABLE;
	// NVIC_Init(&nvic_tim);

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

uint8_t IR_GetData(void)  
{ 
    uint8_t res = 0;
    uint8_t cmd = IR_GetCommand();
    switch (cmd)
    {
        case IR_UP:
            res = 1;
            break;
        case IR_DOWN:
            res = 2;
            break;
        case IR_OK:
            res = 3;
            break;
        case IR_LEFT:
            res = 4;
            break;
        case IR_RIGHT:
            res = 5;
            break;
        default:
            break;
    }
	return res;
}

/* -------------------- 查询接口 -------------------- */

void IR_TIM_IRQHandler(void)
{
    if (TIM_GetITStatus(IR_TIM, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(IR_TIM, TIM_IT_Update);
		ir_count++;//计数器累加
    }
}

//打开定时器3
static void OpenTimerForIR()  
{
   TIM_Cmd(IR_TIM, ENABLE); 
}

static void SetTimerCountForIR(uint16_t count)  
{
   TIM_SetCounter(IR_TIM,count);
}

//关闭定时器3
static void CloseTimerForIR()    
{
   TIM_Cmd(IR_TIM, DISABLE); 
}

//获取定时器3计数器值
uint32_t GetTimerCountForIR(void)
{
   uint32_t t = 0;
   //t = ir_count*1000;
   t += TIM_GetCounter(IR_TIM);
   IR_TIM->CNT = 0;  //计数器归零
   return t;
}

//外部中断函数
void IR_EXTI_IRQHandler(void)
{
	if (EXTI_GetITStatus(IR_IN_EXTI_Line) != RESET)
    {
        EXTI_ClearITPendingBit(IR_IN_EXTI_Line);
        uint32_t t = 0;

        switch (ir_state)
        {
            case 0:                 /* 空闲，等待同步头 */
                SetTimerCountForIR(0);//定时计数器清0
                OpenTimerForIR();//打开定时器
                ir_state = 1;
                break;
            case 1:                 /* 判断 Start/Repeat */
                t = GetTimerCountForIR();//获取上一次中断到此次中断的时间
                SetTimerCountForIR(0);//定时计数器清0
                if (t > 13500 - ir_diff && t < 13500 + ir_diff)       /* Start 13.5ms */
                {
                    ir_state = 2;
                }
                else if (t > 11250 - ir_diff && t < 11250 + ir_diff)  /* Repeat 11.25ms */
                {
                    ir_repeatFlag = 1;
                    CloseTimerForIR();//定时器停止
                    ir_state = 0;
                }
                else //接收出错
                {
                    ir_state = 1;   /* 状态置为1 */
                }
                break;
            case 2:                 /* 状态2，接收数据 接收 32 bit 数据 */
                t = GetTimerCountForIR();//获取上一次中断到此次中断的时间
                SetTimerCountForIR(0);//定时计数器清0
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
                    /* 校验 */
                    if ((ir_buf[0] == (uint8_t)~ir_buf[1]) &&
                        (ir_buf[2] == (uint8_t)~ir_buf[3]))
                    {
                        ir_addr = ir_buf[0];
                        ir_cmd  = ir_buf[2];
                        ir_dataFlag = 1;
                    }
                    CloseTimerForIR();//定时器停止
                    ir_state = 0;
                }
                break;
        }
    }
}
