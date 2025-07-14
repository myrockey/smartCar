#include "stm32f10x.h"                  // Device header
#include "Timer.h"

/* 舵机定时器*/
void TIM_Servo(void)
{
	SG90_TIM_APBX(SG90_TIM_CLK, ENABLE);			//开启TIM1的时钟

	/*配置时钟源*/
	TIM_InternalClockConfig(SG90_TIM);		//选择TIM1为内部时钟，若不调用此函数，TIM默认也为内部时钟
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 20000 - 1;                 //计数周期，即ARR的值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;               //预分频器，即PSC的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(SG90_TIM, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM1的时基单元
	/*TIM使能*/
	TIM_Cmd(SG90_TIM, ENABLE);			//使能TIM1，定时器开始运行
}

/*红外遥控定时器 */
void TIM_IR_NEC(void)
{
	TIM_Init_Servo();
}

/* 超声波模块-定时器 */
void TIM_Ultrasonic(void)
{
	// APB1 是 36MHz 但是定时器的时钟频率计算是根据 （预分频系数、倍频器 决定的）。
    /* 当预分频系数等于1时，倍频器不x2，等预分频系数不等于1时，倍频器x2 
    举例：系统时钟72MHz,APB1 为36MHz，APB2为72MHz 注意：TIM1 在APB2总线上。TIM2,TIM3,TIM4 在APB1总线上。
    1.预分频系数为 1，则 TIM3定时器的时钟频率: APB1时钟频率 * 倍频器 / 预分频系数 =  36MHz * 1 / 1 = 36MHz
    2.预分频系数为 2，则 TIM3定时器的时钟频率: APB1时钟频率 * 倍频器 / 预分频系数 =  36MHz * 2 / 2 = 36MHz
    3.预分频系数为 72，则 TIM3定时器的时钟频率: APB1时钟频率 * 倍频器 / 预分频系数 =  36MHz * 2 / 72 = 1MHz
    */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;  
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  

    TIM_DeInit(Ultrasonic_TIM);
    TIM_TimeBaseStructure.TIM_Period = (1000-1); 
    TIM_TimeBaseStructure.TIM_Prescaler = (72-1); 
    TIM_TimeBaseStructure.TIM_ClockDivision= TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(Ultrasonic_TIM, &TIM_TimeBaseStructure);          
    
    TIM_ClearFlag(Ultrasonic_TIM, TIM_FLAG_Update);  
    TIM_ITConfig(Ultrasonic_TIM, TIM_IT_Update, ENABLE);
	TIM_Cmd(Ultrasonic_TIM, DISABLE);//注意:先不开启
	
    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = Ultrasonic_TIM_IRQn;             
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;       
    NVIC_Init(&NVIC_InitStructure);
}

/*-------------------------------------------------*/
/*函数名：定时器3使能30s定时                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM_WIFI_ENABLE_30S(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;             //定义一个设置定时器的变量
	NVIC_InitTypeDef NVIC_InitStructure;                           //定义一个设置中断的变量
	
	WIFI_TIM_APBX(WIFI_TIM_CLK, ENABLE);           //使能TIM3时钟	
	TIM_DeInit(WIFI_TIM);                                              //定时器3寄存器恢复默认值	
	TIM_TimeBaseInitStructure.TIM_Period = 60000-1; 	           //设置自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 36000-1;             //设置定时器预分频数
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;    //1分频
	TIM_TimeBaseInit(WIFI_TIM, &TIM_TimeBaseInitStructure);            //设置TIM3
	
	TIM_ClearITPendingBit(WIFI_TIM, TIM_IT_Update);                    //清除溢出中断标志位
	TIM_ITConfig(WIFI_TIM, TIM_IT_Update, ENABLE);                     //使能TIM3溢出中断    
	TIM_Cmd(WIFI_TIM, ENABLE);                                         //开TIM3                          
	
	NVIC_InitStructure.NVIC_IRQChannel = WIFI_TIM_IRQn;                //设置TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;      //抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;             //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                //中断通道使能
	NVIC_Init(&NVIC_InitStructure);                                //设置中断
}
/*-------------------------------------------------*/
/*函数名：定时器3使能2s定时                        */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM_WIFI_ENABLE_2S(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;             //定义一个设置定时器的变量
	NVIC_InitTypeDef NVIC_InitStructure;                           //定义一个设置中断的变量
	
	WIFI_TIM_APBX(WIFI_TIM_CLK, ENABLE);           //使能TIM3时钟	
	TIM_DeInit(WIFI_TIM);                                              //定时器3寄存器恢复默认值	
	TIM_TimeBaseInitStructure.TIM_Period = 20000-1; 	           //设置自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 7200-1;              //设置定时器预分频数
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;    //1分频
	TIM_TimeBaseInit(WIFI_TIM, &TIM_TimeBaseInitStructure);            //设置TIM3
	
	TIM_ClearITPendingBit(WIFI_TIM, TIM_IT_Update);                    //清除溢出中断标志位
	TIM_ITConfig(WIFI_TIM, TIM_IT_Update, ENABLE);                     //使能TIM3溢出中断    
	TIM_Cmd(WIFI_TIM, ENABLE);                                         //开TIM3                          
	
	NVIC_InitStructure.NVIC_IRQChannel = WIFI_TIM_IRQn;                //设置TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;      //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;             //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                //中断通道使能
	NVIC_Init(&NVIC_InitStructure);                                //设置中断
}
