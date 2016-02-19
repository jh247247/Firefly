#include <led.h>
#include <jio.h>


typedef struct {
  unsigned char accumulator;
  unsigned char brightness;
  int pin;
} LED_Properties;

LED_Properties gLED[3] = {{.pin = LED_RED_PIN},
			  {.pin = LED_GREEN_PIN},
			  {.pin = LED_BLUE_PIN}};

/* TODO: setup timer for led shutdown? */
/* TODO: setup timer for led software pwm */

inline void LED_Init() {
  JIO_setOut(LED_PORT, .GPIO_Pin = (LED_GREEN_PIN | LED_RED_PIN | LED_BLUE_PIN));
}

/**
 *  \brief Set the status of the selected colour
 */

void LED_SetColor_varg(LED_Attr a) {
  if(a.color == ALL) {
    gLED[RED].brightness = a.brightness;
    gLED[GREEN].brightness = a.brightness;
    gLED[BLUE].brightness = a.brightness;
  } else if(a.color != NONE){
    gLED[a.color].brightness = a.brightness;
  }
}

inline void LED_UpdateInternal(LED_Properties* l) {
  if(l->accumulator++ < l->brightness) {
    JIO_SET(LED_PORT, l->pin);
  } else {
    JIO_CLR(LED_PORT, l->pin);
  }
}

void LED_Update() {
  LED_UpdateInternal(&gLED[RED]);
  LED_UpdateInternal(&gLED[GREEN]);
  LED_UpdateInternal(&gLED[BLUE]);
}
