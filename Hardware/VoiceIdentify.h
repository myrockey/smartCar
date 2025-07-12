#ifndef __ASRPRO_H
#define __ASRPRO_H

extern uint8_t Serial_GetRxFlag(void);
extern uint8_t Serial_GetRxData(void);

void VoiceIdentify_Init(void);
void VoiceIdentify_SendString(char *String);
void VoiceIdentify_SendByte(uint8_t Byte);
#endif

