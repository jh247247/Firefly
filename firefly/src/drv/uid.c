#include <uid.h>

// UNDOCUMENTED FEATURE ON STM32F030, unique ID bits reside at this address.
#define UUID ((uint32_t*)0x1FFFF7AC)

uint32_t UID_get() {
  // take into account all 96 bits.
  return (*UUID)^(*UUID+4)^(*UUID+8);
}
