#include <stm32f0xx.h>
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include <jio.h>
#include <led.h>
#include <uid.h>
#include <serial.h>
#include <spi.h>
#include <nrf24l01.h>


#define PRINT_HEX_8b(c) {const char lt[] = "0123456789ABCDEF";  \
    SERIAL_put(lt[(c&0xF0)>>4]); SERIAL_put(lt[c&0xF]);}



//reset chip
void chip_init(void) {
  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

  /* Set HCLK, PCLK1, and PCLK2 to SCLK (these are default */
  RCC_HCLKConfig(RCC_SYSCLK_Div4);
  //RCC_PCLK1Config(RCC_HCLK_Div1);
  //RCC_PCLK2Config(RCC_HCLK_Div1);

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);
  //  SystemCoreClockUpdate();
  LED_Init();

  SERIAL_init(115200);
  SPI_init();


  JIO_setOut(GPIOB, .GPIO_Pin = GPIO_Pin_1);

  NRF_init();
  NRF_openReadingPipe(0, 0xe7e7e7e7e7);
  NRF_openWritingPipe(0xe7e7e7e7e7);
  NRF_startListening();
  NRF_powerUp();
  NRF_writeReg(CONFIG, MASK_TX_DS | EN_CRC | CRCO | PWR_UP);


}

int main(void) {
  char rec;
  uint8_t buf[5];
  chip_init();
  while(1) {
    //rec = NRF_readReg(CONFIG);
    //PRINT_HEX_8b(rec);
    //SERIAL_put('\n');
    //NRF_writeReg(CONFIG, 0x0F);

    //SERIAL_logVerbose("Transmitting...");
    //NRF_write("HELLO", 5);
    //SERIAL_logVerbose("DONE");

    //NRF_printStatus();

    //    SERIAL_put('\n');

    if(NRF_read(buf, 3) == 0xFF) {
      continue;
    }

    UID_flash();
    PRINT_HEX_8b(buf[0]);
    PRINT_HEX_8b(buf[1]);
    PRINT_HEX_8b(buf[2]);
    SERIAL_put('\n');
    NRF_flushRx();
    //NRF_printStatus();
  }
}
