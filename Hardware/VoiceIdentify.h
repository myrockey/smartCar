#ifndef __ASRPRO_H
#define __ASRPRO_H

extern uint8_t Serial_GetRxFlag(void);
extern uint8_t Serial_GetRxData(void);

void VoiceIdentify_Init(void);
void VoiceIdentify_SendString(char *String);

#endif

