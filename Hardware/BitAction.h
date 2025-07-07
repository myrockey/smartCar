#ifndef __BIT_ACTION_H
#define __BIT_ACTION_H

/*
现在我们就可以用位操作的方法来控制 GPIO 的输入和输出了，
其中宏参数 n 表示具体是哪一个IO 口，n(0,1,2…16)。这里面包含了端口 A~G ，并不是每个单片机型号都有这么多端口，
使用这部分代码时，要查看你的单片机型号，如果是 64pin 的则最多只能使用到 C 端口
*/
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x02000000+((addr &0x00FFFFFF)<<5)+(bitnum<<2))

// 把一个地址转换成一个指针
#define MEM_ADDR(addr) *((volatile unsigned long *)(addr))
// 把位带别名区地址转换成指针
#define BIT_ADDR(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))

#define GPIOA_ODR_Addr (GPIOA_BASE+12) //0x4001080C
#define GPIOB_ODR_Addr (GPIOB_BASE+12) //0x40010C0C
#define GPIOC_ODR_Addr (GPIOC_BASE+12) //0x4001100C
#define GPIOD_ODR_Addr (GPIOD_BASE+12) //0x4001140C
#define GPIOE_ODR_Addr (GPIOE_BASE+12) //0x4001180C
#define GPIOF_ODR_Addr (GPIOF_BASE+12) //0x40011A0C
#define GPIOG_ODR_Addr (GPIOG_BASE+12) //0x40011C0C



#define GPIOA_IDR_Addr (GPIOA_BASE+8) //0x40010808
#define GPIOB_IDR_Addr (GPIOB_BASE+8) //0x40010C08
#define GPIOC_IDR_Addr (GPIOC_BASE+8) //0x40011008
#define GPIOD_IDR_Addr (GPIOD_BASE+8) //0x40011408
#define GPIOE_IDR_Addr (GPIOE_BASE+8) //0x40011808
#define GPIOF_IDR_Addr (GPIOF_BASE+8) //0x40011A08
#define GPIOG_IDR_Addr (GPIOG_BASE+8) //0x40011E08

// 单独操作 GPIO 的某一个 IO 口，n(0,1,2...16),n 表示具体是哪一个 IO 口

#define PAout(n) BIT_ADDR(GPIOA_ODR_Addr,n) //输出
#define PAin(n)  BIT_ADDR(GPIOA_IDR_Addr,n) //输入

#define PBout(n) BIT_ADDR(GPIOB_ODR_Addr,n) //输出
#define PBin(n)  BIT_ADDR(GPIOB_IDR_Addr,n) //输入

#define PCout(n) BIT_ADDR(GPIOC_ODR_Addr,n) //输出
#define PCin(n)  BIT_ADDR(GPIOC_IDR_Addr,n) //输入

#define PDout(n) BIT_ADDR(GPIOD_ODR_Addr,n) //输出
#define PDin(n)  BIT_ADDR(GPIOD_IDR_Addr,n) //输入

#define PEout(n) BIT_ADDR(GPIOE_ODR_Addr,n) //输出
#define PEin(n)  BIT_ADDR(GPIOE_IDR_Addr,n) //输入

#define PFout(n) BIT_ADDR(GPIOF_ODR_Addr,n) //输出
#define PFin(n)  BIT_ADDR(GPIOF_IDR_Addr,n) //输入

#define PGout(n) BIT_ADDR(GPIOG_ODR_Addr,n) //输出
#define PGin(n)  BIT_ADDR(GPIOG_IDR_Addr,n) //输入

#endif
