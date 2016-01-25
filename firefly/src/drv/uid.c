#include <uid.h>
#define UUID ((uint32_t*)0x40015800)

uint32_t UID_get() {
  return UUID[0]^UUID[1]^UUID[2];
}
