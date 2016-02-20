#include <stm32f0xx.h>
#include <jio.h>
#include <led.h>
#include <uid.h>
#include <serial.h>
#include <spi.h>
#include <nrf24l01.h>

int resetCount __attribute__((__section__(".persistent"),used));

#define PRINT_HEX_8b(c) {const char lt[] = "0123456789ABCDEF";  \
    SERIAL_put(lt[(c&0xF0)>>4]); SERIAL_put(lt[c&0xF]);}

#define SWITCH_INIT JIO_setIn(GPIOA, .GPIO_Pin = GPIO_Pin_1,    \
                              .GPIO_PuPd = GPIO_PuPd_UP)
#define SWITCH_READ !JIO_GET(GPIOA, GPIO_Pin_1)


//reset chip
void chip_init(void) {

  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

  /* Set HCLK, PCLK1, and PCLK2 to SCLK (these are default */
  RCC_HCLKConfig(RCC_SYSCLK_Div1);
  //RCC_PCLK1Config(RCC_HCLK_Div1);
  //RCC_PCLK2Config(RCC_HCLK_Div1);

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);
  //  SystemCoreClockUpdate();

  // what to do when reset for the first time
  if(resetCount) {
    IWDG_Enable();
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    IWDG_SetReload(156); /* TODO: Make this programmable by user... */
    IWDG_ReloadCounter();
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
  }

  LED_Init();
  SWITCH_INIT;

  SERIAL_init(115200);
  SPI_init();
  NRF_init();


  resetCount++;
  // scatter the signal strength just in case we are out of range at minimum
  NRF_setPALevel(resetCount&0x03);
}

void chip_sleep() {
  NRF_deinit();
  SPI_shutdown();
  PWR_BackupAccessCmd(DISABLE);
  PWR_PVDCmd(DISABLE);
  // although standby is lower power (by quite a bit) we need to have some volatile memory.
  // Havent hacked the proc to find one yet.
  //PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
  PWR_EnterSTANDBYMode();
}

int main(void) {
  char iter = 100;
  uint8_t buf[32];

  chip_init();

  uint32_t* uid = (uint32_t*)buf;
  *uid = UID_get();
  buf[4] = SWITCH_READ | ((resetCount&0x03)<<2); /* TODO: add more to this byte */
  buf[5] = 0; /* TODO: read battery life */
  buf[6] = 0; /* TODO: read battery life */
  buf[7] = 0; /* TODO: read temperature */

  // wait until reload value flag is reset, then reload value can be read.
  while(IWDG->SR&IWDG_FLAG_RVU) __asm__("");
  buf[8] = IWDG->RLR&0xFF;
  buf[9] = 0|((IWDG->RLR&0xF00)>>8);


  if(NRF_write(buf,10)) {
    SERIAL_putString("WRITE FAIL!\n");
    chip_sleep();
  }

  while(iter) {
    iter--;

    if(NRF_available()) {
      break;
    }
  }
  // iterations gone to 0, read timeout!
  if(iter == 0) {
    SERIAL_putString("READ FAIL!\n");
    chip_sleep();
  }

  NRF_read(buf,9);

  NRF_deinit();
  SPI_shutdown();

  // determine what to do based on rec'd packet

  while(SWITCH_READ) {
    UID_flash();
  }

  chip_sleep();
}
