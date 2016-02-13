#include <jio.h>
#include <spi.h>
#include <stm32f0xx_spi.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx.h>

#define SPI_SCK_PIN
#define SPI_MOSI_PIN
#define SPI_SCK_PIN


void SPI_init() {
  uint8_t drain_count,drain;

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  // set SCK, MISO and MOSI to alternate function
  JIO_setMode(GPIOA,
              .GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7,
              .GPIO_Mode = GPIO_Mode_AF,
              .GPIO_OType = GPIO_OType_PP,
              .GPIO_Speed = GPIO_Speed_2MHz,
              .GPIO_PuPd = GPIO_PuPd_NOPULL);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_0);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_0);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_0);

  // Now configure the SPI interface
  drain = SPI1->SR;                              // dummy read of SR to clear MODF

  // enable SSM, set SSI, enable SPI, PCLK/64, LSB First Master
  SPI1->CR1 = SPI_CR1_SSM|SPI_CR1_SSI|SPI_BaudRatePrescaler_8|SPI_Mode_Master|SPI_Direction_2Lines_FullDuplex;
  SPI1->CR2 = SPI_DataSize_8b;   // configure for 8 bit operation

  SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_QF);
  SPI_Cmd(SPI1,ENABLE);
}

// shut down SPI when not being used to save power
void SPI_shutdown() {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
}

// since SPI is duplex, we have to send and receive at the same time.
/* TODO:  Buffer IO with a FIFO if needed. */
uint8_t SPI_transfer(uint8_t data) {
  while ((SPI1->SR&SPI_I2S_FLAG_TXE)==RESET); // Wait while DR register is not empty
  SPI_SendData8(SPI1,data); // Send byte to SPI
  while ((SPI1->SR & SPI_I2S_FLAG_RXNE)==RESET); // Wait to receive byte
  return SPI_ReceiveData8(SPI1); // Read byte from SPI bus
}
