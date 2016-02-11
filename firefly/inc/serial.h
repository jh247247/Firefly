#ifndef SERIAL_H
#define SERIAL_H
#include<stm32f0xx.h>

extern const char* SERIAL_ERRMSG;
extern const char* SERIAL_WRNMSG;
extern const char* SERIAL_VRBMSG;
extern const char* SERIAL_DEBMSG;

// this is for the STM32F030, so we want to use PA9 and PA10 alternate functions
void SERIAL_init(int baud);
#define SERIAL_put(c) while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); USART1->TDR = c
#define SERIAL_get() USART1->RDR

unsigned int SERIAL_getDelimited(char* buf, char delimiter);
void SERIAL_putString(const char* str);

#define SERIAL_logError(m) SERIAL_putString(SERIAL_ERRMSG); SERIAL_putString(m); SERIAL_put('\n')
#define SERIAL_logWarning(m) SERIAL_putString(SERIAL_WRNMSG); SERIAL_putString(m); SERIAL_put('\n')
#define SERIAL_logVerbose(m) SERIAL_putString(SERIAL_VRBMSG); SERIAL_putString(m); SERIAL_put('\n')

#define check(A, M) if(!(A)) {SERIAL_logError(M);}

#ifdef DEBUG
#define SERIAL_logDebug(m) SERIAL_putString(SERIAL_DEBMSG); SERIAL_putString(m); SERIAL_put('\n')
#else
// define empty macro so debug stuff doesn't get printed.
#define SERIAL_logDebug(m)
#endif

#endif /* SERIAL_H */
