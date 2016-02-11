#include <stm32f0xx.h>
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include <jio.h>
#include <led.h>
#include <uid.h>
#include <serial.h>

//reset chip
void chip_init(void) {
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  //  SystemCoreClockUpdate();
  LED_Init();

  SERIAL_init(115200);
}

int main(void) {
  int del = 5000;
  int cyc = 0;
  unsigned char col = 0;
  chip_init();
  while(1) {
    UID_flash();
    SERIAL_logVerbose("Hello world!");
  }
}
