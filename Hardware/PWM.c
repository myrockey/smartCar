#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "Timer.h"

/**
  * 函    数：PWM初始化 电机
  * 参    数：无
  * 返 回 值：无
  */
void PWM_Init_Motor(void)
{
	/*开启时钟*/
	TB6612_TIM_APBX(TB6612_TIM_CLK, ENABLE);			//开启TIM2的时钟
	TB6612_GPIO_APBX(TB6612_GPIO_CLK, ENABLE);			//开启GPIOA的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = TB6612_GPIO_PIN_PWMA | TB6612_GPIO_PIN_PWMB;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(TB6612_GPIO_PORT, &GPIO_InitStructure);							//将PA2引脚初始化为复用推挽输出	
																	//受外设控制的引脚，均需要配置为复用模式
	
	/*配置时钟源*/
	TIM_InternalClockConfig(TB6612_TIM);		//选择TIM2为内部时钟，若不调用此函数，TIM默认也为内部时钟
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;                 //计数周期，即ARR的值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 36 - 1;               //预分频器，即PSC的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TB6612_TIM, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM2的时基单元
	
	/*输出比较初始化*/ 
	TIM_OCInitTypeDef TIM_OCInitStructure;							//定义结构体变量
	TIM_OCStructInit(&TIM_OCInitStructure);                         //结构体初始化，若结构体没有完整赋值
	                                                                //则最好执行此函数，给结构体所有成员都赋一个默认值
	                                                                //避免结构体初值不确定的问题
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;               //输出比较模式，选择PWM模式1
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       //输出极性，选择为高，若选择极性为低，则输出高低电平取反
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   //输出使能
	TIM_OCInitStructure.TIM_Pulse = 0;								//初始的CCR值
	TIM_OC2Init(TB6612_TIM, &TIM_OCInitStructure);                        //将结构体变量交给TIM_OC2Init，配置TIM2的输出比较通道2
	TIM_OC1Init(TB6612_TIM, &TIM_OCInitStructure);                        //将结构体变量交给TIM_OC1Init，配置TIM2的输出比较通道1
	
	/*TIM使能*/
	TIM_Cmd(TB6612_TIM, ENABLE);			//使能TIM2，定时器开始运行
}

/**
  * 函    数：PWM初始化 舵机
  * 参    数：无
  * 返 回 值：无
  */
void PWM_Init_Servo(void)
{
	/*开启时钟*/
	SG90_GPIO_APBX(SG90_GPIO_CLK, ENABLE);			//开启GPIOA的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = SG90_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SG90_GPIO_PORT, &GPIO_InitStructure);							//将PA2引脚初始化为复用推挽输出	
																	//受外设控制的引脚，均需要配置为复用模式

	/*输出比较初始化*/ 
	TIM_OCInitTypeDef TIM_OCInitStructure;							//定义结构体变量
	TIM_OCStructInit(&TIM_OCInitStructure);                         //结构体初始化，若结构体没有完整赋值
	                                                                //则最好执行此函数，给结构体所有成员都赋一个默认值
	                                                                //避免结构体初值不确定的问题
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;               //输出比较模式，选择PWM模式1
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       //输出极性，选择为高，若选择极性为低，则输出高低电平取反
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   //输出使能
	TIM_OCInitStructure.TIM_Pulse = 2500;								//初始的CCR值
	TIM_OC4Init(SG90_TIM, &TIM_OCInitStructure);                        //将结构体变量交给TIM_OC4Init，配置TIM2的输出比较通道4
	
	TIM_OC1PreloadConfig(SG90_TIM, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(SG90_TIM, ENABLE);

    /* 高级定时器需要使能主输出 */
    TIM_CtrlPWMOutputs(SG90_TIM, ENABLE);

	TIM_Servo();
}

/* TIM1 和 TIM2 各自的通道是独立的，能同时使用 通道1 */
/**
  * 函    数：PWM设置CCR
  * 参    数：Compare 要写入的CCR的值，范围：0~100
  * 返 回 值：无
  * 注意事项：CCR和ARR共同决定占空比，此函数仅设置CCR的值，并不直接是占空比
  *           占空比Duty = CCR / (ARR + 1)
  */
void PWM_SetCompare2_Motor(uint16_t Compare)
{
	TIM_SetCompare2(TB6612_TIM, Compare);		//设置CCR2的值
}

void PWM_SetCompare1_Motor(uint16_t Compare)
{
	TIM_SetCompare1(TB6612_TIM, Compare);		//设置CCR4的值
}

void PWM_SetCompare4_Servo(uint16_t Compare)
{
	TIM_SetCompare4(SG90_TIM, Compare);		//设置CCR4的值
}
