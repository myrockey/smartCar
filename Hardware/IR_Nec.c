#include "stm32f10x.h"                  // Device header
#include "IR_Nec.h"
#include "Timer.h"

/* ------------------ 用户可修改的宏 ------------------ */

#define IR_GPIO_APBX      RCC_APB2PeriphClockCmd
#define IR_GPIO_CLK       RCC_APB2Periph_GPIOA       /* GPIO时钟 */
#define IR_GPIO_PORT      GPIOA	                     /* GPIO端口 */
#define IR_GPIO_PIN       GPIO_Pin_8
#define IR_TIM    	      TIM1		                /* 定时器1 */
#define IR_IN_EXTI_Port   GPIO_PortSourceGPIOC        /* 外部中断 */
#define IR_IN_EXTI_Pin    GPIO_PinSource15
#define IR_IN_EXTI_Line   EXTI_Line15
#define IR_IN_EXTI_IRQN   EXTI15_10_IRQn
#define IR_EXTI_IRQHandler EXTI15_10_IRQHandler
#define IR_TIM_UPDATE_IRQn     TIM1_UP_IRQn  /* TIM更新中断 */
#define IR_TIM_UPDATE_IRQHandler  TIM1_UP_IRQHandler
#define IR_TIM_CC_IRQn     TIM1_CC_IRQn  /* TIM更新中断 */
#define IR_TIM_CC_IRQHandler  TIM1_CC_IRQHandler

#define  RX_SEQ_NUM  33
/* --------------------------------------------------- */

/* 全局变量 */
static uint8_t  cap_pol          = 0; //捕获电平类型1-上升沿或0-下降沿
static uint8_t  cap_pulse_cnt    = 0;//捕获到的计数
static volatile uint32_t ir_overflow = 0;//溢出次数
static volatile uint32_t ir_count = 0;
static volatile uint8_t  ir_state   = 0;   /* 0/1/2 */

static uint16_t rx_frame[RX_SEQ_NUM*2] = {0}; 

struct {
    uint16_t  src_data[RX_SEQ_NUM*2];
    uint16_t  repet_cnt;
    union{
        uint32_t rev;
        struct
        {
            uint32_t key_val_n:8;
            uint32_t key_val  :8;
            uint32_t addr_n   :8;
            uint32_t addr     :8;
        }_rev;
    }data;
}rx;

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
	TIM_ICInitStructure.TIM_ICFilter = 0xF;							//输入滤波器参数，可以过滤信号抖动
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;		//极性，选择为上升沿触发捕获
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;			//捕获预分频，选择不分频，每次信号都触发捕获
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;	//输入信号交叉，选择直通，不交叉
	TIM_ICInit(IR_TIM, &TIM_ICInitStructure);							//将结构体变量交给TIM_ICInit，配置TIM3的输入捕获通道

	/*选择触发源及从模式*/
	TIM_SelectInputTrigger(IR_TIM, TIM_TS_TI1FP1);					//触发源选择TI1FP1
	TIM_SelectSlaveMode(IR_TIM, TIM_SlaveMode_Reset);					//从模式选择复位
																	//即TI1产生上升沿时，会触发CNT归零
    

	// /* 允许更新中断 */
    TIM_ITConfig(IR_TIM, TIM_IT_Update, ENABLE);
    TIM_ITConfig(IR_TIM, TIM_IT_CC1, ENABLE);

	/* TIM1 Update NVIC */


	TIM_IR_NEC();
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

//读取通道1捕获的值
uint16_t IR_TIM_GetCapture1()
{
    return TIM_GetCapture1(IR_TIM);
}

void rx_rcv_init(void)
{
    ir_dataFlag     = 0;                                       //未捕获到新数据
    ir_state      = 0;                                       //非空闲状态
    ir_overflow   = 0;                                       //定时器溢出清0
    cap_pulse_cnt = 0;                                       //捕获到的计数清0
    
    memset(rx_frame,0x00,sizeof(rx_frame));
}

void IR_TIM_UPDATE_IRQHandler(void)
{
    if (TIM_GetITStatus(IR_TIM, TIM_IT_Update) != RESET)
    {
        if (ir_state == 1) //非空闲状态
        {
            ir_overflow++;
            if (ir_overflow == 3)
            {
                ir_overflow = 0;
                ir_state    = 0;
                ir_dataFlag   = 1;
            }
        }

        TIM_ClearITPendingBit(IR_TIM, TIM_IT_Update);
    }
}


void IR_TIM_CC_IRQHandler(void)
{
    static uint16_t tmp_cnt_l, tmp_cnt_h;
    if (TIM_GetITStatus(IR_TIM, TIM_IT_CC1) != RESET)
    {
        switch(cap_pol)
        {   
            case 0:/* 捕获到下降沿 */
                tmp_cnt_l = IR_TIM_GetCapture1();
                TIM_ITConfig(TIM1, TIM_IT_CC1, DISABLE);
                TIM_SlaveConfig(TIM1, 0, TIM_SlaveMode_Reset, TIM_ICPolarity_Rising);
                TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);
                cap_pol = 1;
                if (ir_state == 0)
                {
                    rx_rcv_init();
                    break;
                }
                rx_frame[cap_pulse_cnt] = ir_overflow * 20000 + tmp_cnt_l - tmp_cnt_h;
                ir_overflow = 0;
                printf("(%2d)%4d us:H\r\n", cap_pulse_cnt, rx_frame[cap_pulse_cnt]);
                cap_pulse_cnt++;
                break;
            
            case 1:/* 捕获到上升沿 */
                tmp_cnt_h = TIM_GetCapture1(TIM1);
                TIM_ITConfig(TIM1, TIM_IT_CC1, DISABLE);
                TIM_SlaveConfig(TIM1, 0, TIM_SlaveMode_Reset, TIM_ICPolarity_Falling);
                TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);
                cap_pol = 0;
                if (ir_state == 0)
                {
                    rx_rcv_init();
                    break;
                }
                rx_frame[cap_pulse_cnt] = ir_overflow * 20000 + tmp_cnt_h - tmp_cnt_l;
                ir_overflow = 0;
                printf("(%2d)%4d us:L\r\n", cap_pulse_cnt, rx_frame[cap_pulse_cnt]);
                cap_pulse_cnt++;
                break;
            
            default:
                break;
        }
        TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);
    }
}

uint8_t hx1838_data_decode(void)
{
    memcpy(rx.src_data,rx_frame,RX_SEQ_NUM*4);
    memset(rx_frame,0x00,RX_SEQ_NUM*4);   
    printf("========= rx.src[] =================\r\n");
    for(uint8_t i = 0;i<=(RX_SEQ_NUM*2);i++)
    {
        printf("[%d]%d\r\n",i,rx.src_data[i]);
    }
    printf("========= rx.rec =================\r\n");
    if(appro(rx.src_data[0],9000) && appro(rx.src_data[1],4500))                 //#1. 检测前导码
    {
        uint8_t tmp_idx = 0;
        rx.repet_cnt  = 0;                                                       //按键重复个数清0
        for(uint8_t i = 2;i<(RX_SEQ_NUM*2);i++)                                  //#2. 检测数据
        {
            if(!appro(rx.src_data[i],560))
            {
                printf("%d,err:%d != 560\r\n",i,rx.src_data[i]);
                return 0;
            }
            i++;
            if(appro(rx.src_data[i],1680))
            {
                rx.data.rev |= (0x80000000 >> tmp_idx);                          //第 tmp_idx 为置1
                tmp_idx++;
            }
            else if(appro(rx.src_data[i],560))
            {
                rx.data.rev &= ~(0x80000000 >> tmp_idx);                         //第 tmp_idx 位清0
                tmp_idx++;
            }
            else
            {
                printf("%d,err:%d != 560||1680\r\n",i,rx.src_data[i+1]);
                return 0;
            }
        }
    }
    else if(appro(rx.src_data[0],9000) && appro(rx.src_data[1],2250) && appro(rx.src_data[2],560))
    {
        rx.repet_cnt++;
        return 2;
    }
    else
    {
        printf("前导码检测错误\r\n");
        return 0;
    }
    return 1;
}

uint8_t hx1838_proc(uint8_t res)
{
    uint8_t RxData = 0;
    if(res == 0)
    {    
        return;
    }
    
    if(res == 2)
    {   
        return;
    }
    //编码转换为操作指令
    switch(rx.data._rev.key_val)
    {
        case 11:
            RxData = 1;
            break;
        case 12:
            RxData = 2;
            break;
        case 13:
            RxData = 3;
            break;
        case 14:
            RxData = 4;
            break;
        case 15:
            RxData = 5;
            break;
        default:   
            break;
        
    }
    return RxData;
}


// void HX1838_demo(void)
// {
//     hx1838_cap_start();//定时器1通道1，输入捕获
//     while(1)
//     {
//         if(cap_frame)//标记捕获到新的数据
//         {   
//             hx1838_proc(hx1838_data_decode());//解析数据
//             cap_frame = 0;
						
		
//         }
//     }
    
// }