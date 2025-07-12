#include "stm32f10x.h"
#include "globals.h"
#include "String.h"

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
//字符串中保留取json
void extract_json(const char *raw, char *json_out) {
    const char *start = strchr(raw, '{');  // 找到第一个 '{'
    const char *end   = strrchr(raw, '}'); // 找到最后一个 '}'

    if (start && end && end > start) {
        size_t len = end - start + 1;
        strncpy(json_out, start, len);
        json_out[len] = '\0'; // 手动加结束符
    } else {
        json_out[0] = '\0'; // 没找到，返回空字符串
    }
}
