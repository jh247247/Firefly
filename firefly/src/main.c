#include <stm32f0xx.h>
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include <jio.h>
#include <led.h>
#include <uid.h>
#include <serial.h>
#include <spi.h>

//reset chip
void chip_init(void) {
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  //  SystemCoreClockUpdate();
  LED_Init();

  SERIAL_init(115200);
  SPI_init();
}

int main(void) {
  char rec;
  chip_init();
  while(1) {
    UID_flash();
    SERIAL_logVerbose("Test SPI");
    rec = SPI_transfer("A");
    SERIAL_logVerbose(rec);
  }
}
