#include <stm32f0xx.h>
#include "stm32f0xx_gpio.h"

void JIO_Init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef GPIO_InitStruct) {
  GPIO_Init(GPIOx, &GPIO_InitStruct);
}
