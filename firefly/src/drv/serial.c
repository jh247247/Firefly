#include<serial.h>
#include<jio.h>

const char* SERIAL_ERRMSG = "E/";
const char* SERIAL_WRNMSG = "W/";
const char* SERIAL_VRBMSG = "V/";
const char* SERIAL_DEBMSG = "D/";

void SERIAL_init(int baud) {
 GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  JIO_setMode(GPIOA,
	      .GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10,
	      .GPIO_Mode = GPIO_Mode_AF,
	      .GPIO_PuPd = GPIO_PuPd_NOPULL);

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1); // USART1 TX
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

  USART_InitTypeDef USART_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

  USART_InitStructure.USART_BaudRate = baud;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);

  USART_Cmd(USART1,ENABLE);
}

void SERIAL_put(char c) {
  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
  USART1->TDR = c;
}

unsigned int SERIAL_getDelimited(char* buf, char delimiter) {
  unsigned int len = 0;
  do {
    *buf = SERIAL_get();
    len++;
  } while(*buf++ != delimiter);
  return len;
}

void SERIAL_putString(const char* str) {
  while(*str) {
    SERIAL_put(*str++);
  }
}
