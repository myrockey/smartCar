#ifndef __HC_05_H
#define __HC_05_H

#include "stm32f10x.h"                  // Device header

void HC_05_Init(void);
void HC_05_SendString(char *String);

#endif

