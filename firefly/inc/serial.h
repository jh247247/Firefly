#ifndef SERIAL_H
#define SERIAL_H
#include<stm32f0xx.h>


// this is for the STM32F030, so we want to use PA9 and PA10 alternate functions
void SERIAL_init(int baud);
#define SERIAL_put(c) USART_SendData(USART1,c);
#define SERIAL_get() USART_ReceiveData(USART1);

unsigned int SERIAL_getDelimited(char* buf, char delimiter);
void SERIAL_putString(char* str);

#define SERIAL_logError(m)
#define SERIAL_logWarning(m)
#define SERIAL_logVerbose(m)

#ifdef DEBUG
#define SERIAL_logDebug(m)
#else
#define SERIAL_logDebug(m)
#endif

#endif /* SERIAL_H */
