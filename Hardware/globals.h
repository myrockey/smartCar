#ifndef __GLOBALS_H
#define __GLOBALS_H

void Filter_memcpy(uint8_t *dst, uint8_t *src, int size);//拷贝数据，并去除空字符
uint8_t extract_json(const char *raw, char *json_out);//从字符串中提取json

#endif /*__GLOBALS_H*/
