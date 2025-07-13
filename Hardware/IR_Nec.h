#ifndef __IR_NEC_H
#define __IR_NEC_H

void IR_Nec_Init(void);                 // 初始化
uint8_t IR_GetDataFlag(void);
uint8_t IR_GetRepeatFlag(void);
uint8_t IR_GetAddress(void);
uint8_t IR_GetCommand(void);

#endif
