#include <uid.h>

// UNDOCUMENTED FEATURE ON STM32F030, unique ID bits reside at this address.
#define UUID ((uint32_t*)0x1FFFF7A4)

uint32_t UID_get() {
  return UUID[1];
}
