#include "stm32f10x.h"
#include "globals.h"

//拷贝数据，并去除空字符
void Filter_memcpy(uint8_t *dst, uint8_t *src, int size)
{
    int i = 0;
    for(i = 0; i < size; i++)
    {
        if(src[i] != '\0'){
            dst[i] = src[i];
        }
    }
}
