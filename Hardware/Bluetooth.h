#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

extern uint8_t Serial_GetRxFlag(void);
extern uint8_t Serial_GetRxData(void);

void Bluetooth_Init(void);
void Bluetooth_SendString(char *String);

#endif
