#include <led.h>
#include <uid.h>
#include <delay.h>

#define PREAMBLE_BITS 4
#define DATA_BITS 8
#define DELAY_TICKS 2000
#define LED_OVERHEAD 100

#define RED_ENABLE
#define GREEN_ENABLE
//#define BLUE_ENABLE

typedef union {
  uint32_t i;
  uint8_t b[4];
} UID_Bytes;

void UID_preamble() {
  JIO_SET(LED_PORT,
#ifdef RED_ENABLE
          LED_RED_PIN+
#endif // RED_ENABLE
#ifdef GREEN_ENABLE
          LED_GREEN_PIN+
#endif // GREEN_ENABLE
#ifdef BLUE_ENABLE
          BLUE_ENABLE+
#endif // BLUE_ENABLE
          0);
  // we want to send 10 bits per channel, 2 preamble bits and 8 data bits
  for(int i = 0; i < PREAMBLE_BITS; i++) {
    delay(DELAY_TICKS+LED_OVERHEAD);
    JIO_FLP(LED_PORT,
#ifdef RED_ENABLE
            LED_RED_PIN+
#endif // RED_ENABLE
#ifdef GREEN_ENABLE
            LED_GREEN_PIN+
#endif // GREEN_ENABLE
#ifdef BLUE_ENABLE
            BLUE_ENABLE+
#endif // BLUE_ENABLE
            0);
  }
  LED_ALL_OFF;

}

void UID_flash() {
  UID_Bytes id;
  id.i = UID_get();

  delay((DELAY_TICKS+LED_OVERHEAD)*PREAMBLE_BITS);
  UID_preamble();
  for(int i = 0; i < DATA_BITS; i++) {
#ifdef RED_ENABLE
    if(id.b[0]&(1<<i)) {
      LED_ON(RED);
    } else {
      LED_OFF(RED);
    }
#endif // RED_ENABLE

#ifdef GREEN_ENABLE
    if(id.b[1]&(1<<i)) {
      LED_ON(GREEN);
    } else {
      LED_OFF(GREEN);
    }
#endif // GREEN_ENABLE

#ifdef BLUE_ENABLE
    if(id.b[2]&(1<<i)) {
      LED_ON(BLUE);
    } else {
      LED_OFF(BLUE);
    }
#endif // BLUE_ENABLE

    delay(DELAY_TICKS);
  }
  LED_ALL_OFF;
  UID_preamble();
  delay((DELAY_TICKS+LED_OVERHEAD)*PREAMBLE_BITS);
}
