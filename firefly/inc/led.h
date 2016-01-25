#ifndef LED_H
#define LED_H

typedef enum {RED = 0, GREEN, BLUE, ALL, NONE} LedColor;

#define LED_FULL_BRIGHTNESS 255

#define LED_PORT GPIOA

#define LED_GREEN_OFFSET 10
#define LED_RED_OFFSET 13
#define LED_BLUE_OFFSET 14

#define LED_GREEN_PIN (1<<LED_GREEN_OFFSET)
#define LED_RED_PIN (1<<LED_RED_OFFSET)
#define LED_BLUE_PIN (1<<LED_BLUE_OFFSET)

#define LED_ON(c) JIO_SET(LED_PORT,LED_##c##_PIN)
#define LED_OFF(c) JIO_CLR(LED_PORT,LED_##c##_PIN)

typedef struct {
  LedColor color;
  unsigned char brightness;
} LED_Attr;


void LED_Init();
#define LED_SetColor(...) LED_SetColor_varg((LED_Attr){.color = NONE, .brightness = 0, __VA_ARGS__});
void LED_Update();

#endif /* LED_H */
