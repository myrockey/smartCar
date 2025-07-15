#include "stm32f10x.h"                  // Device header
#include "IR_Nec.h"
#include "Timer.h"

/* ------------------ 用户可修改的宏 ------------------ */

#define IR_GPIO_APBX      RCC_APB2PeriphClockCmd
#define IR_GPIO_CLK       RCC_APB2Periph_GPIOA       /* GPIO时钟 */
#define IR_GPIO_PORT      GPIOA	                     /* GPIO端口 */
#define IR_GPIO_PIN       GPIO_Pin_8
#define IR_TIM    	      TIM1		                /* 定时器1 */
#define IR_TIM_UPDATE_IRQn     TIM1_UP_IRQn  /* TIM更新中断 */
#define IR_TIM_UPDATE_IRQHandler  TIM1_UP_IRQHandler
#define IR_TIM_CC_IRQn     TIM1_CC_IRQn  /* TIM更新中断 */
#define IR_TIM_CC_IRQHandler  TIM1_CC_IRQHandler
/* --------------------------------------------------- */

#define IR_UP    0x18
#define IR_DOWN  0x52
#define IR_OK    0x1C
#define IR_LEFT  0x08
#define IR_RIGHT 0x5A

/* 全局变量 */
static volatile uint32_t ir_count = 0;//中断溢出次数
static volatile uint32_t ir_lastCnt = 0;//上次的计数
static volatile uint32_t ir_currentCnt = 0;//本次的计数
static volatile uint8_t  ir_state   = 0;   /* 0/1/2 */
static volatile uint8_t  ir_bits    = 0;
static volatile uint8_t  ir_buf[4]  = {0};
static volatile uint8_t  ir_dataFlag   = 0;
static volatile uint8_t  ir_repeatFlag = 0;
static volatile uint8_t  ir_addr = 0;
static volatile uint8_t  ir_cmd  = 0;
static const uint32_t ir_diff = 500;

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
    /*输入捕获初始化*/
	TIM_ICInitTypeDef TIM_ICInitStructure;							//定义结构体变量
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;				//选择配置定时器通道1
	TIM_ICInitStructure.TIM_ICFilter = 0x03;//IC2F=0011 配置输入滤波器 8个定时器时钟周期滤波
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;		//极性，选择为下降沿触发捕获
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;			//捕获预分频，选择不分频，每次信号都触发捕获
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;	//输入信号交叉，选择直通，不交叉
	TIM_ICInit(IR_TIM, &TIM_ICInitStructure);							//将结构体变量交给TIM_ICInit，配置TIM3的输入捕获通道

    TIM_ClearFlag(IR_TIM, TIM_FLAG_Update | TIM_FLAG_CC1);; //清除中断和捕获标志位事件
	// /* 允许更新中断 */
    TIM_ITConfig(IR_TIM, TIM_IT_Update, ENABLE);
    TIM_ITConfig(IR_TIM, TIM_IT_CC1, ENABLE);

	/* TIM1 Update NVIC */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = IR_TIM_CC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

	TIM_IR_NEC();

    GPIO_SetBits(IR_GPIO_PORT, IR_GPIO_PIN);//置高电平
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

void IR_TIM_UPDATE_IRQHandler(void)
{
    if (TIM_GetITStatus(IR_TIM, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(IR_TIM, TIM_IT_Update);
        if(ir_state != 0){
		    ir_count++;//溢出计数累加
        }
    }
}


//打开定时器3
static void OpenTimerForIR()  
{
    ir_count = 0;//溢出次数清0
    ir_lastCnt = 0;//上次读数
    ir_currentCnt = 0;//本次读数
}

//关闭定时器3
static void CloseTimerForIR()    
{

}

//由于不直接对定时器的CNT清0
void SetTimerCountForIR(uint16_t count)
{
    // ir_count = count;//溢出次数清0
    // ir_lastCnt = 0;//上次读数
    // ir_currentCnt = 0;//本次读数
}

//获取定时器的计数器值
uint32_t GetTimerCountForIR(void)
{
   uint32_t t = 0;
   ir_currentCnt = ir_count*20000 + TIM_GetCounter(IR_TIM);
   t = ir_currentCnt - ir_lastCnt;
   ir_lastCnt = ir_currentCnt;  //上次读数
   return t;
}

//输入捕获中断函数
void IR_TIM_CC_IRQHandler(void)
{
    //下降沿捕获触发中断
	if(TIM_GetITStatus(IR_TIM,TIM_IT_CC1)!=RESET)
    {
	    TIM_ClearITPendingBit(IR_TIM,TIM_IT_CC1);
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
