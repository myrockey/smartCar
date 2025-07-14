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

#define IR_UP    0x18
#define IR_DOWN  0x52
#define IR_OK    0x1C
#define IR_LEFT  0x08
#define IR_RIGHT 0x5A

#define  RX_SEQ_NUM  33
/* --------------------------------------------------- */

/* 全局变量 */
static volatile uint8_t  cap_pol          = 0; //捕获电平类型1-上升沿或0-下降沿
static volatile uint8_t  cap_pulse_cnt    = 0;//捕获到的计数
static volatile uint32_t ir_overflow = 0;//溢出次数
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
    /* 5. 使能从模式 */
    TIM_SelectMasterSlaveMode(TIM1, TIM_MasterSlaveMode_Enable);

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

	TIM_IR_NEC();
}

void TIM_SET_ITConfig(uint16_t TIM_ICPolarity)
{
    /* 1. 关闭 CC1 中断 */
    TIM_ITConfig(TIM1, TIM_IT_CC1, DISABLE);

    /* 2. 配置通道 1：上升沿捕获 */
    TIM_ICInitTypeDef  TIM_ICInitStructure;
    TIM_ICInitStructure.TIM_Channel     = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity  = TIM_ICPolarity;   // ← 这里设置极性
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter    = 0xF;
    TIM_ICInit(TIM1, &TIM_ICInitStructure);

    /* 3. 触发源 TI1FP1（已含上升沿极性） */
    TIM_SelectInputTrigger(TIM1, TIM_TS_TI1FP1);

    /* 4. 从模式 Reset */
    TIM_SelectSlaveMode(TIM1, TIM_SlaveMode_Reset);

    /* 5. 使能从模式 */
    TIM_SelectMasterSlaveMode(TIM1, TIM_MasterSlaveMode_Enable);

    /* 6. 重新使能 CC1 中断 */
    TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);
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

//声明
uint8_t hx1838_data_decode(void);

uint8_t IR_GetCommand(void)  
{ 
    uint8_t res = hx1838_data_decode();
    //准确获取数据
    if(res == 1)
    {
        ir_cmd = rx.data._rev.key_val;
    }
    rx.data.rev = 0;
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

//读取通道1捕获的值
uint16_t IR_TIM_GetCapture1()
{
    return TIM_GetCapture1(IR_TIM);
}

void rx_rcv_init(void)
{
    ir_dataFlag     = 0;                                       //未捕获到新数据
    ir_state      = 1;                                       //非空闲状态
    ir_overflow   = 0;                                       //定时器溢出清0
    cap_pulse_cnt = 0;                                       //捕获到的计数清0
    
    memset(rx_frame,0x00,sizeof(rx_frame));
}

void IR_TIM_UPDATE_IRQHandler(void)
{
    if (TIM_GetITStatus(IR_TIM, TIM_IT_Update) != RESET)
    {
        if (ir_state != 0) //非空闲状态
        {
            ir_overflow++;
            if (ir_overflow == 2)//溢出1次，就够了，说明超过20ms,已收到数据
            {
                ir_overflow = 0;
                ir_state    = 0;
                ir_dataFlag = 1;
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
                TIM_SET_ITConfig(TIM_ICPolarity_Rising);

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
                TIM_SET_ITConfig(TIM_ICPolarity_Falling);
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

uint8_t appro(int num1,int num2)
{
    return (abs(num1-num2) < 300);
}

uint8_t hx1838_data_decode(void)
{
    memcpy(rx.src_data,rx_frame,RX_SEQ_NUM*4);
    memset(rx_frame,0x00,RX_SEQ_NUM*4);   
    printf("========= rx.src[] =================\r\n");
    for(uint8_t i = 0;i<=(RX_SEQ_NUM*2-1);i++)
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