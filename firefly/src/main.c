#include <stm32f0xx.h>
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include <jio.h>
#include <led.h>
#include <uid.h>
#include <serial.h>
#include <spi.h>
#include <nrf24l01.h>

#define PRINT_HEX_8b(c) {const char lt[17] = "01234567890ABCDEF"; char b[3]; \
    b[0] = lt[c&0x0F]; b[1] = lt[(c&0xF0)>>4]; b[2] = '\0';		\
    SERIAL_logVerbose(b);}

//reset chip
void chip_init(void) {
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  //  SystemCoreClockUpdate();
  LED_Init();

  SERIAL_init(115200);
  SPI_init();
  NRF_init();
}

int main(void) {
  char rec;
  chip_init();
  while(1) {
    UID_flash();
    SERIAL_logVerbose("Status: ");
    PRINT_HEX_8b(NRF_readStatus());
    SERIAL_logVerbose("Transmitting...");
    NRF_write("HELLO",5);
    SERIAL_logVerbose("DONE");
    delay(500000);
  }
}
