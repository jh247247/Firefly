//software delays - not to be used for precision delays
void delay(uint32_t dly) {
  while (dly-- != 0) __NOP();
}
