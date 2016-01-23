#include <stm32f0xx.h>                                                                  //code for stm32f030
#include "stm32f0xx_rcc.h"                                                              //clock routing used
#include "stm32f0xx_gpio.h"                                                             //gpio modules used
#include <jio.h>

//hardware set-up
#define LED_PORT                        GPIOA                                           //led on gpioa
#define LED_C                           (1<<0)                                          //led cathod on pin 0
#define LED_A                           (1<<10)                                          //led anode on pin 1

//global defines
//define cpu speed
#define F_CPU                           (SystemCoreClock)                       //cpu frequency
#define NOP()                           __NOP()                                         //remap nop()

//software delays
#define DLY_MS                          5 //50 cycles = 1ms @ 1Mhz - estimated. calibrate for your chip / compiler setting
#define delay_ms(ms)            delay((ms) * DLY_MS * ((F_CPU) / 1000000ul))//delay for milli-seconds

//software delays - not to be used for precision delays
void delay(uint32_t dly) {
  while (dly--) NOP();                                                          //waste some time
}

//reset chip
void chip_init(void) {
  //enable power to gpiox
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);                   //for led
  SystemCoreClockUpdate();                                                      //update SystemCoreClock / F_CPU
}

int main(void) {
  int del = 1;

  chip_init();
  JIO_setOut(LED_PORT, .GPIO_Pin = LED_A);
  JIO_SET(LED_PORT, LED_A);
  JIO_setOut(LED_PORT, .GPIO_Pin = LED_C);
  JIO_CLR(LED_PORT, LED_C);
  while(1) {
    JIO_FLP(LED_PORT, LED_A | LED_C);
    delay_ms(del++);
    if(del == 500) {
      del = 1;
    }
  }
}
