#include <stdint.h>
#include <jio.h>
#include <spi.h>
#include <delay.h>
#include <stm32f0xx.h>
#include "nrf24l01.h"

#define putstr(s) SERIAL_putString(s)
#define put(s) SERIAL_put(s)

#ifndef PRINT_HEX_8b
#define PRINT_HEX_8b(c) {const char lt[] = "0123456789ABCDEF";  \
    put(lt[(c&0xF0)>>4]); put(lt[c&0xF]);}
#endif

#define NRF_CE_PORT GPIOA
#define NRF_CE_PIN GPIO_Pin_4
#define NRF_CE_INIT JIO_setOut(NRF_CE_PORT, .GPIO_Pin = NRF_CE_PIN);
#define NRF_CE_SET JIO_SET(NRF_CE_PORT, NRF_CE_PIN)
#define NRF_CE_CLR JIO_CLR(NRF_CE_PORT, NRF_CE_PIN)

#define NRF_CS_PORT GPIOB
#define NRF_CS_PIN GPIO_Pin_1
#define NRF_CS_INIT JIO_setOut(NRF_CS_PORT, .GPIO_Pin = NRF_CS_PIN);
#define NRF_CS_SET JIO_SET(NRF_CS_PORT, NRF_CS_PIN)
#define NRF_CS_CLR JIO_CLR(NRF_CS_PORT, NRF_CS_PIN)

#define NRF_IRQ_PORT GPIOA
#define NRF_IRQ_PIN GPIO_Pin_2
#define NRF_IRQ_INIT JIO_setIn(NRF_IRQ_PORT, .GPIO_Pin = NRF_IRQ_PIN);
#define NRF_IRQ_READ JIO_GET(NRF_IRQ_PORT, NRF_IRQ_PIN)

#define MAX_PAYLOAD_SIZE 32

// minimum delay that CS has to be high is about 17us between commands.
#define DELAY_TIME 15

uint8_t NRF_readStatus() {
  uint8_t ret;

  delay(DELAY_TIME);

  NRF_CS_CLR;

  ret = SPI_transfer(NOP);

  NRF_CS_SET;
  return ret;
}

uint8_t NRF_cmd(uint8_t reg) {
  uint8_t ret;

  delay(DELAY_TIME);

  NRF_CS_CLR;
  SPI_transfer(reg);
  ret = SPI_transfer(0xFF);
  NRF_CS_SET;
  return ret;
}

inline uint8_t NRF_readReg(uint8_t reg) {
  return NRF_cmd(R_REGISTER | (REGISTER_MASK & (reg)));
}

inline uint8_t NRF_writeReg(uint8_t reg, uint8_t data) {
  return NRF_wcmd(W_REGISTER | (REGISTER_MASK & (reg)), (data));
}


// write data to reg
uint8_t NRF_wcmd(uint8_t reg, uint8_t data) {
  uint8_t status;

  delay(DELAY_TIME);

  NRF_CS_CLR;
  status = SPI_transfer(reg);
  SPI_transfer(data);
  NRF_CS_SET;
  return status;
}

// write to multiple registers
uint8_t NRF_readMultibyteReg(uint8_t reg, uint8_t* buf, uint8_t len) {
  uint8_t ret;
  delay(DELAY_TIME);
  NRF_CS_CLR;
  ret = SPI_transfer(R_REGISTER | (REGISTER_MASK & reg));
  while(len--) {
    *buf++ = SPI_transfer(0xFF);
  }
  NRF_CS_SET;
  delay(DELAY_TIME);
  return ret;
}

// write to multiple registers
uint8_t NRF_writeMultibyteReg(uint8_t reg, const uint8_t* buf, uint8_t len) {
  uint8_t ret;
  delay(DELAY_TIME);
  NRF_CS_CLR;
  ret = SPI_transfer(W_REGISTER | (REGISTER_MASK & reg));
  while(len--) {
    SPI_transfer(*buf++);
  }
  NRF_CS_SET;
  delay(DELAY_TIME);
  return ret;
}

// set power amplifier level
void NRF_setPALevel(nrf24_pa_dbm_e level)
{
  uint8_t setup = NRF_readReg(RF_SETUP) ;
  setup &= ~((1<<RF_PWR_LOW) | (1<<RF_PWR_HIGH)) ;

  // switch uses RAM (evil!)
  if ( level == RF24_PA_MAX )
    {
      setup |= ((1<<RF_PWR_LOW) | (1<<RF_PWR_HIGH)) ;
    }
  else if ( level == RF24_PA_HIGH )
    {
      setup |= (1<<RF_PWR_HIGH) ;
    }
  else if ( level == RF24_PA_LOW )
    {
      setup |= (1<<RF_PWR_LOW);
    }
  else if ( level == RF24_PA_MIN )
    {
      // nothing
    }
  else if ( level == RF24_PA_ERROR )
    {
      // On error, go to maximum PA
      setup |= ((1<<RF_PWR_LOW) | (1<<RF_PWR_HIGH)) ;
    }

  NRF_writeReg( RF_SETUP, setup ) ;
}

/* TODO: remove delays...  */
void NRF_init() {
  NRF_CE_INIT;
  NRF_CS_INIT;
  NRF_IRQ_INIT;

  NRF_CE_CLR;
  NRF_CS_SET;

  /* TODO:  different delay for reset vs power up.*/
  delay(DELAY_TIME);

  NRF_writeReg(SETUP_RETR, (0b0100 << ARD) | (0b1111 << ARC));
  delay(DELAY_TIME);


  // Disable dynamic payloads, to match dynamic_payloads_enabled setting
  NRF_writeReg(DYNPD,0);

  // set to defaults
  NRF_writeReg(RF_SETUP, 0x00); /* TODO: make datarate 2mbps for lower range... */
  NRF_setPALevel(RF24_PA_MAX);

  // Reset current status
  // Notice reset and flush is the last thing we do
  NRF_writeReg(STATUS,(1<<RX_DR) | (1<<TX_DS) | (1<<MAX_RT) );

  // Set up default configuration.  Callers can always change it later.
  // This channel should be universally safe and not bleed over into adjacent
  // spectrum.
  NRF_setChannel(76);

  // Flush buffers
  NRF_flushRx();
  NRF_flushTx();

  // disable auto ack...
  NRF_writeReg(EN_AA, 0);
  NRF_writeReg(CONFIG, 0x0E);

  // enable P0, should do more at some stage
  NRF_writeReg(EN_RXADDR, (1<<ERX_P0));
  NRF_writeReg(RX_PW_P0,MAX_PAYLOAD_SIZE);
}

void NRF_startListening(void)
{
  NRF_writeReg(CONFIG, NRF_readReg(CONFIG) | (1<<PWR_UP) | (1<<PRIM_RX));
  NRF_writeReg(STATUS, (1<<RX_DR) | (1<<TX_DS) | (1<<MAX_RT) );

  // Flush buffers
  NRF_flushTx();
  NRF_flushRx();

  // Go!
  NRF_CE_SET;

  // wait for the radio to come up (130us actually only needed)
  /* TODO:  */
  //delayMicroseconds(130);
}

void NRF_stopListening(void)
{
  NRF_CE_CLR;
  NRF_flushTx();
  NRF_flushRx();
}

// BENCHMARK: 1.2ms for 32 byte transmit, cpu side.
uint8_t NRF_write( const uint8_t* buf, uint8_t len ) {
  uint8_t i;
  // set len to the minimum of max payload and the given len
  len = len>MAX_PAYLOAD_SIZE ? MAX_PAYLOAD_SIZE : len;

  NRF_CE_CLR;

  // set into writing mode
  NRF_writeReg(CONFIG, (NRF_readReg(CONFIG) | (1<<PWR_UP)) & ~(1<<PRIM_RX));

  delay(DELAY_TIME);

  NRF_CS_CLR;
  SPI_transfer(W_TX_PAYLOAD); // start transferring payload
  for(i = 0; i < MAX_PAYLOAD_SIZE; i++) {
    if(i < len) {
      SPI_transfer(*(buf+i));
    } else {
      SPI_transfer(0x00); // have to send blanks, since payload size is fixed
    }
  }
  delay(DELAY_TIME);
  NRF_CS_SET;

  NRF_CE_SET;
  uint8_t status = NRF_readStatus();
  while(!(status & (1<<TX_DS))) {
    status = NRF_readStatus();
  }

  NRF_CE_CLR;
  /* TODO: Power down? */
  NRF_startListening();
}

// tells us if there is a packet available on one of the listening pipes
uint8_t NRF_available() {
  return (NRF_readStatus() & (1<<RX_DR));
}

// confirmed working 16-02-2016 JH
// assumes that the NRF is already in reading mode, checks channel 0 for a packet and reads it into a given buffer.
// limits the write to the buffer size.
char NRF_read(uint8_t* buf, uint8_t l) {

  uint8_t len = NRF_readReg(RX_PW_P0); /* TODO: handle multiple channels? */

  delay(DELAY_TIME);
  // choose the minimum of the given buffer length and the packet size
  len = l>len ? len : l;
  if(!(NRF_available())) {
    return -1;
  }

  delay(DELAY_TIME);

  NRF_CS_CLR;
  delay(100);
  SPI_transfer(R_RX_PAYLOAD); // read rx payload
  delay(100);
  while(len > 0) {
    *buf++ = SPI_transfer(0xFF);
    len--;
    delay(100);
  }

  NRF_CS_SET;
  delay(DELAY_TIME);

  NRF_writeReg(STATUS, (1<<RX_DR));
  return len;
}

void NRF_powerUp() {
  NRF_writeReg(CONFIG,NRF_readReg(CONFIG) & ~(1<<PWR_UP));
  NRF_CE_CLR;
}
void NRF_powerDown() {
  NRF_writeReg(CONFIG,NRF_readReg(CONFIG) | (1<<PWR_UP));
}

#define PRINT_BIT(reg,name) putstr(#name); putstr(reg&(1<<name)?" = 1 ":" = 0 ")

void print_address_register(const char* name, uint8_t reg, uint8_t qty)
{
  putstr(name);
  putstr("  ");

  while (qty--)
    {
      uint8_t buffer[5];
      NRF_readMultibyteReg(reg++,buffer,sizeof buffer);

      putstr(" 0x");
      uint8_t* bufptr = buffer + sizeof buffer;
      while( --bufptr >= buffer ) {
        PRINT_HEX_8b((*bufptr));
      }
    }
  putstr("\n");
}

void print_byte_register(const char* name, uint8_t reg, uint8_t qty)
{
  putstr(name);
  putstr("  ");

  while (qty--) {
    putstr(" 0x");
    PRINT_HEX_8b(NRF_readReg(reg));
    reg++;
    delay(DELAY_TIME);
  }
  putstr("\n");
}

void NRF_printStatus() {

  print_byte_register("CONFIG",CONFIG,1);
  print_byte_register("EN_AA",EN_AA,1);
  print_byte_register("EN_RXADDR",EN_RXADDR,1);
  print_byte_register("SETUP_AW",SETUP_AW,1);
  print_byte_register("SETUP_RETR",SETUP_RETR,1);
  print_byte_register("RF_CH",RF_CH,1);
  print_byte_register("RF_SETUP",RF_SETUP,1);
  print_byte_register("RF_PWR",RF_PWR,1);

  uint8_t status = NRF_readStatus();

  putstr("STATUS = ");
  PRINT_HEX_8b(status);
  putstr(" ");
  PRINT_BIT(status,RX_DR);
  PRINT_BIT(status,TX_DS);
  PRINT_BIT(status,MAX_RT);
  PRINT_BIT(status,TX_FULL);
  /* TODO: RX_P_NO */
  putstr("\n");

  print_byte_register("OBSERVE_TX",OBSERVE_TX,1);
  print_byte_register("RPD",RPD,1);

  print_address_register("RX_ADDR_P0-1",RX_ADDR_P0,2);
  print_byte_register("RX_ADDR_P2-5",RX_ADDR_P2,4);

  print_address_register("TX_ADDR",TX_ADDR,1);

  print_byte_register("RX_PW_P0-6",RX_PW_P0,6);

  print_byte_register("FIFO_STATUS",FIFO_STATUS,1);
  print_byte_register("DYNPD",DYNPD,1);
  print_byte_register("FEATURE",FEATURE,1);
}

