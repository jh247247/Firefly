#include <stm32f0xx.h>
#include <jio.h>
#include <led.h>
#include <uid.h>
#include <serial.h>
#include <spi.h>
#include <nrf24l01.h>


#define PRINT_HEX_8b(c) {const char lt[] = "0123456789ABCDEF";  \
    SERIAL_put(lt[(c&0xF0)>>4]); SERIAL_put(lt[c&0xF]);}


void RESET_Handler() {

}

//reset chip
void chip_init(void) {
  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

  /* Set HCLK, PCLK1, and PCLK2 to SCLK (these are default */
  RCC_HCLKConfig(RCC_SYSCLK_Div1);
  //RCC_PCLK1Config(RCC_HCLK_Div1);
  //RCC_PCLK2Config(RCC_HCLK_Div1);

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);
  //  SystemCoreClockUpdate();
  LED_Init();

  SERIAL_init(115200);
  SPI_init();

  NRF_init();
  NRF_startListening();

  IWDG_Enable();
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_SetPrescaler(IWDG_Prescaler_256);
  IWDG_SetReload(156); /* TODO: Make this programmable by user... */
  IWDG_ReloadCounter();
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
}

void chip_sleep() {
  PWR_BackupAccessCmd(DISABLE);
  PWR_PVDCmd(DISABLE);
  // although standby is lower power (by quite a bit) we need to have some volatile memory.
  // Havent hacked the proc to find one yet.
  PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
}

int main(void) {
  char rec;
  uint8_t buf[5];

  chip_init();


  NRF_write("HELLO",5);

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
    if(NRF_available()) {
      break;
    }
    /* TODO: timeout for read */
  }

  NRF_read(buf,5);

  NRF_powerDown();

  UID_flash();
  PRINT_HEX_8b(buf[0]);
  PRINT_HEX_8b(buf[1]);
  PRINT_HEX_8b(buf[2]);

  chip_sleep();
}
