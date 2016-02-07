// spi.h for stm32f030
// Author: Frank Duignan.
// Updates posted on http://ioprog.com and http://eleceng.dit.ie/frank

#include "stm32f0xx.h"
#include "stm32f0xx_rcc.h"
#include <stdint.h>
#include "spi.h"

/* TODO: Generalize this code for more use cases (i.e more chip selects...) */

void initSPI() {
  uint32_t drain, drain_count;

  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;         // turn on SPI1

  // GPIOA bits 5,6,7 are used for SPI1 (Alternative functions 0)
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;    // enable port A
  GPIOA->MODER &= ~(GPIO_MODER_MODER5_1|GPIO_MODER_MODER6_1|GPIO_MODER_MODER7_1);
  GPIOA->MODER |= (GPIO_MODER_MODER5_0|GPIO_MODER_MODER6_0|GPIO_MODER_MODER7_0);

  GPIOA->AFR[0] &= 0x000fffff;                  // select Alt. Function 0

  // Now configure the SPI interface
  drain = SPI1->SR;                              // dummy read of SR to clear MODF

  // enable SSM, set SSI, enable SPI, PCLK/16, LSB First Master
  SPI1->CR1 = (1<<9)|(1<<8)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2);//|(1<<1);
  SPI1->CR2 = (1<<10)|(1<<9)|(1<<8);   // configure for 8 bit operation
  //SPI1_CR2 = (1<<9)+(1<<8);       // configure for 8 bit operation

  for (drain_count = 0; drain_count < 32; drain_count++) {
    drain = transferSPI(0x00);
  }
}

uint8_t transferSPI(uint8_t data)
{
  unsigned Timeout = 1000000;
  int ReturnValue;

  while (((SPI1->SR & (1<<7))!=0)&&(Timeout--));
  SPI1->DR = data;
  Timeout = 1000000;
  while (((SPI1->SR & (1<<7))!=0)&&(Timeout--));
  ReturnValue = (SPI1->DR & 0xFF);

  return ReturnValue;
}
