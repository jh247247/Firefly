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

const uint8_t child_pipe[] = {RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2, RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5};
const uint8_t child_payload_size[] = {RX_PW_P0, RX_PW_P1, RX_PW_P2, RX_PW_P3, RX_PW_P4, RX_PW_P5};
const uint8_t child_pipe_enable[] = {ERX_P0, ERX_P1, ERX_P2, ERX_P3, ERX_P4, ERX_P5};

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

/* TODO: REDUCE ME (or at least make the CPU sleep for this time...) */

#define DELAY_TIME 1000

uint8_t g_p_variant = 0;
uint8_t g_payloadSize = 32;
uint8_t g_dynamicPayloads = 0;
uint64_t g_pipe0ReadingAddress;

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

uint8_t NRF_readReg(uint8_t reg) {
  return NRF_cmd(R_REGISTER | (REGISTER_MASK & (reg)));
}

uint8_t NRF_writeReg(uint8_t reg, uint8_t data) {
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
  NRF_CS_CLR;
  delay(DELAY_TIME);
  ret = SPI_transfer(R_REGISTER | (REGISTER_MASK & reg));
  delay(50);
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
  NRF_CS_CLR;
  delay(DELAY_TIME);
  ret = SPI_transfer(W_REGISTER | (REGISTER_MASK & reg));
  delay(50);
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

uint8_t NRF_setDataRate(rf24_datarate_e speed)
{
  uint8_t result = 0;
  uint8_t setup = NRF_readReg(RF_SETUP) ;

  // HIGH and LOW '00' is 1Mbs - our default
  //wide_band = 0;
  setup &= ~((1<<RF_DR_LOW) | (1<<RF_DR_HIGH)) ;
  if( speed == RF24_250KBPS ) {
    // Must set the RF_DR_LOW to 1; RF_DR_HIGH (used to be RF_DR) is already 0
    // Making it '10'.
    //wide_band = 0;
    setup |= (1<< RF_DR_LOW ) ;
  } else {
    // Set 2Mbs, RF_DR (RF_DR_HIGH) is set 1
    // Making it '01'
    if ( speed == RF24_2MBPS ) {
      //  wide_band = 1;
      setup |= (1<<RF_DR_HIGH);
    } else {
      // 1Mbs
      //wide_band = 0;
    }
  }

  NRF_writeReg(RF_SETUP,setup);

  // Verify our result
  if ( NRF_readReg(RF_SETUP) == setup ) {
    result = 1;
  } else {
    //wide_band = 0;
  }

  return result;
}

void NRF_setCRCLength(rf24_crclength_e length)
{
  uint8_t config = NRF_readReg(CONFIG) & ~( (1<<CRCO) | (1<<EN_CRC)) ;

  // switch uses RAM (evil!)
  if ( length == RF24_CRC_DISABLED )
    {
      // Do nothing, we turned it off above.
    }
  else if ( length == RF24_CRC_8 )
    {
      config |= (1<<EN_CRC);
    }
  else
    {
      config |= (1<<EN_CRC);
      config |= (1<< CRCO );
    }
  NRF_writeReg( CONFIG, config ) ;
}

uint8_t NRF_writePayload(const void* buf, uint8_t len) {
  uint8_t status;

  const uint8_t* current = (const uint8_t*)buf;

  uint8_t data_len = len > g_payloadSize ? len : g_payloadSize;
  uint8_t blank_len = g_dynamicPayloads ? 0 : g_payloadSize - data_len;

  NRF_flushTx();

  NRF_CS_CLR;
  status = SPI_transfer( W_TX_PAYLOAD );
  while ( data_len-- )
    SPI_transfer(*current++);
  while ( blank_len-- )
    SPI_transfer(0);
  NRF_CS_SET;

  return status;
}

void NRF_setAutoAck(uint8_t enable)
{
  if ( enable )
    NRF_writeReg(EN_AA, 0b111111);
  else
    NRF_writeReg(EN_AA, 0);
}

void NRF_init() {
  NRF_CE_INIT;
  NRF_CS_INIT;
  NRF_IRQ_INIT;

  NRF_CE_CLR;
  NRF_CS_SET;

  /* TODO:  different delay for reset vs power up.*/
  delay(DELAY_TIME);

  NRF_writeReg(SETUP_RETR, (0b0100 << ARD) | (0b1111 << ARC));

  NRF_setPALevel(RF24_PA_MIN);

  if( NRF_setDataRate( RF24_250KBPS ) )
    {
      g_p_variant = 1;
    }

  NRF_setDataRate( RF24_1MBPS ) ;

  NRF_setCRCLength( RF24_CRC_16 ) ;

  // Disable dynamic payloads, to match dynamic_payloads_enabled setting
  NRF_writeReg(DYNPD,0);

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

  NRF_setAutoAck(0);

  NRF_writeReg(RX_ADDR_P2, 0xC3);
  NRF_writeReg(RX_ADDR_P3, 0xC4);
  NRF_writeReg(RX_ADDR_P4, 0xC5);
  NRF_writeReg(RX_ADDR_P5, 0xC6);

  NRF_writeReg(CONFIG, NRF_readReg(CONFIG) | MASK_TX_DS | 0x0E);

}

void NRF_startListening(void)
{
  NRF_writeReg(CONFIG, NRF_readReg(CONFIG) | (1<<PWR_UP) | (1<<PRIM_RX));
  NRF_writeReg(STATUS, (1<<RX_DR) | (1<<TX_DS) | (1<<MAX_RT) );

  // Restore the pipe0 adddress, if exists
  if (g_pipe0ReadingAddress)
    NRF_writeMultibyteReg(RX_ADDR_P0, (const uint8_t*)(&g_pipe0ReadingAddress), 5);

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

void NRF_openWritingPipe(uint64_t value)
{
  // Note that AVR 8-bit uC's store this LSB first, and the NRF24L01(+)
  // expects it LSB first too, so we're good.

  NRF_writeMultibyteReg(RX_ADDR_P0, (uint8_t*)(&value), 5);
  NRF_writeMultibyteReg(TX_ADDR, (uint8_t*)(&value), 5);

  const uint8_t max_payload_size = 32;
  NRF_writeReg(RX_PW_P0, g_payloadSize>max_payload_size ? max_payload_size : g_payloadSize);
}


void NRF_openReadingPipe(uint8_t child, uint64_t address)
{
  // If this is pipe 0, cache the address.  This is needed because
  // openWritingPipe() will overwrite the pipe 0 address, so
  // startListening() will have to restore it.
  if (child == 0)
    g_pipe0ReadingAddress = address;

  if (child <= 6)
    {
      // For pipes 2-5, only write the LSB
      if ( child < 2 )
        NRF_writeMultibyteReg(child_pipe[child], (const uint8_t*)(&address), 5);
      else
        NRF_writeMultibyteReg(child_pipe[child], (const uint8_t*)(&address), 1);

      NRF_writeReg(child_payload_size[child],g_payloadSize);

      // Note it would be more efficient to set all of the bits for all open
      // pipes at once.  However, I thought it would make the calling code
      // more simple to do it this way.
      NRF_writeReg(EN_RXADDR, NRF_readReg(EN_RXADDR) | (1<<child_pipe_enable[child]));
    }
}

uint8_t NRF_write( const void* buf, uint8_t len )
{

}

// confirmed working 16-02-2016 JH
// assumes that the NRF is already in reading mode, checks channel 0 for a packet and reads it into a given buffer.
// limits the write to the buffer size.
char NRF_read(uint8_t* buf, uint8_t l) {

  uint8_t len = NRF_readReg(RX_PW_P0); /* TODO: handle multiple channels? */
  // status gets optimized away?
  uint8_t status = NRF_readStatus();

  // choose the minimum of the given buffer length and the packet size
  len = l>len ? len : l;
  if(!(status & (1<<RX_DR))) {
    return -1;
  }

  /* TODO: remove this delay at some point, or at least reduce it. */
  delay(DELAY_TIME);

  NRF_CS_CLR;

  SPI_transfer(R_RX_PAYLOAD);
  while(len > 0) {
    *buf++ = SPI_transfer(0xFF);
    len--;
  }

  NRF_CS_SET;

  delay(DELAY_TIME);

  NRF_writeReg(STATUS, (1<<RX_DR));
  return len;
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
    delay(500);
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
