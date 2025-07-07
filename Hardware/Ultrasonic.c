#include "Ultrasonic.h"
#include "motor.h"

uint16_t msHcCount = 0;//定时器计数
extern int distance;//距障碍物距离

//定时器3设置
void ultrasonic_NVIC()
{
    NVIC_InitTypeDef NVIC_InitStructure;
    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;             
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;       
    NVIC_Init(&NVIC_InitStructure);
}

//IO口初始化 及其他初始化
void Ultrasonic_Init(void)
{  
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;   
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

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);   

    TIM_DeInit(TIM3);
    TIM_TimeBaseStructure.TIM_Period = (1000-1); 
    TIM_TimeBaseStructure.TIM_Prescaler = (72-1); 
    TIM_TimeBaseStructure.TIM_ClockDivision= TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);          
        
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);  
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);

    ultrasonic_NVIC();
    
    TIM_Cmd(TIM3,DISABLE);     
}

//打开定时器3
static void OpenTimerForHc()  
{
   TIM_SetCounter(TIM3,0);
   msHcCount = 0;
   TIM_Cmd(TIM3, ENABLE); 
}

//关闭定时器3
static void CloseTimerForHc()    
{
   TIM_Cmd(TIM3, DISABLE); 
}

//获取定时器3计数器值
uint32_t GetEchoTimer(void)
{
   uint32_t t = 0;
   t = msHcCount*1000;
   t += TIM_GetCounter(TIM3);
   TIM3->CNT = 0;  //计数器归零
   Delay_ms(50);
   return t;
}
 
//通过定时器3计数器值推算距离
float Ultrasonic_GetLength(void )
{
   uint32_t t = 0;
   int i = 0;
   float lengthTemp = 0;
   float sum = 0;
   uint8_t j = 0;
    
   while(i != 5)  //测量五次取平均
   {
        for(j = 0;j < 5;j++)
        {
            TRIG_Send = 1;   //给控制端高电平
            Delay_us(20);
            TRIG_Send = 0;  //超声波模块已开始发送8个40khz脉冲
        }

        while(ECHO_Reci == 0);   //若ECHO_Reci为低电平，则一直循环，直到为高电平。  

        OpenTimerForHc();       //此时说明检测到高电平，开启定时器，开始计时。       

        while(ECHO_Reci == 1); //若ECHO_Reci为高电平，则一直循环，直到为低电平。 

        CloseTimerForHc();   //此时说明检测到低电平，关闭定时器，停止计时 

        t = GetEchoTimer();        //获取定时器时间       
        lengthTemp = ((float)t*0.034/2); //数据处理，转换成cm
        sum += lengthTemp;  //五次测得数据累加
        i++;
    }
   
    lengthTemp = sum/5.0; //取平均
    return lengthTemp;
}

//超声波避障
void Ultrasonic_Run(void)
{
    //大于30cm还可以往前走
    if(distance > 30)
    {
        Move_Forward();
    }
    //小于30cm还可以往后退
    if(distance <= 30)
    {
        Move_Backward();
    }
    
    //慢慢转避让
    if(distance <= 7)
    { 
        Clockwise_Rotation();
        Delay_ms(100);
        
        Car_Stop();
        Delay_ms(500);  
    }
}

//定时器3终中断
void TIM3_IRQHandler(void)  
{
   if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  
   {
       TIM_ClearITPendingBit(TIM3, TIM_IT_Update  ); 
       msHcCount++; //计数器开始加
   }
}
 