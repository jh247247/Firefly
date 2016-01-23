#include <stm32f0xx.h>									//code for stm32f030
#include "stm32f0xx_rcc.h"								//clock routing used
#include "stm32f0xx_gpio.h"								//gpio modules used

//hardware set-up
#define LED_PORT			GPIOA						//led on gpioa
#define LED_C				(1<<13)						//led cathod on pin 0
#define LED_A				(1<<10)						//led anode on pin 1
#define LED_DLY				500							//duration for led flips, in ms

//global defines
//define cpu speed
#define F_CPU				(SystemCoreClock)			//cpu frequency
#define NOP()				__NOP()						//remap nop()

//port operations
#define PIN_SET(port, pins)	port->ODR |= (pins)			//set pins on port
#define PIN_CLR(port, pins)	port->ODR &=~(pins)			//clear pins on port
#define PIN_FLP(port, pins)	port->ODR ^= (pins)			//flip pins on port
#define PIN_GET(port, pins) (port->IDR & (pins))		//read pins on port

//software delays
#define DLY_MS				50							//50 cycles = 1ms @ 1Mhz - estimated. calibrate for your chip / compiler setting
#define delay_ms(ms)		delay((ms) * DLY_MS * ((F_CPU) / 1000000ul))//delay for milli-seconds

//global variables

//set pins as output on port
void PIN_OUT(GPIO_TypeDef* port, uint16_t pins) {
	GPIO_InitTypeDef        GPIO_InitStruct;

	/* Configure pins in output pushpull mode */
	GPIO_InitStruct.GPIO_Pin = pins;					//GPIO_Pin_0..15 + GPIO_Pin_All
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;			//GPIO_Mode_Out_PP -> push-pull; GPIO_Mode_Out_OD -> open drain
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;			//pins as in push-pull mode
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;		//GPIO_Speed_50Mhz -> 50Mhz; GPIO_Speed_2Mhz -> 2Mhz; GPIO_Speed_10Mhz -> 10Mhz
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;		//no pull-up/down
	GPIO_Init(port, &GPIO_InitStruct);					//initialize the pins
}

//set pins as input on port
void PIN_IN(GPIO_TypeDef* port, uint16_t pins) {
	GPIO_InitTypeDef        GPIO_InitStruct;

	/* Configure pins in input mode */
	GPIO_InitStruct.GPIO_Pin = pins;					//GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN/*_FLOATING*/;	//GPIO_Mode_IN_FLOATING -> input, floating; GPIO_Mode_IN_IPU/IPD -> input, pull-up / pull-down
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;			//push-pull / irrelevant
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; 		//GPIO_Speed_50Mhz -> 50Mhz; GPIO_Speed_2Mhz -> 2Mhz; GPIO_Speed_10Mhz -> 10Mhz
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;		//no pull-up/down
	GPIO_Init(port, &GPIO_InitStruct);					//initialize the pins
}

//software delays - not to be used for precision delays
void delay(uint32_t dly) {
	while (dly--) NOP();								//waste some time
}

//reset chip
void chip_init(void) {
	//enable power to gpiox
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);			//for led
	SystemCoreClockUpdate();							//update SystemCoreClock / F_CPU
}

int main(void) {
	chip_init();										//reset the chip
	PIN_OUT(LED_PORT, LED_A);
	PIN_SET(LED_PORT, LED_A);		//led1 as output, set led_a
	PIN_OUT(LED_PORT, LED_C);
	PIN_CLR(LED_PORT, LED_C);
    while(1) {
    	PIN_FLP(LED_PORT, LED_A | LED_C);				//flip the led
    	delay_ms(LED_DLY);								//waste some time
    }
}
