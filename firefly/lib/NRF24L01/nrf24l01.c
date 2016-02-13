#include <stdint.h>
#include <jio.h>
#include <spi.h>
#include <delay.h>
#include <stm32f0xx.h>
#include "nrf24l01.h"

#define NRF_CE_PORT GPIOA
#define NRF_CE_PIN GPIO_Pin_4
#define NRF_CE_INIT JIO_setOut(GPIOA, .GPIO_Pin = NRF_CE_PIN);
#define NRF_CE_SET JIO_SET(NRF_CS_PORT, NRF_CS_PIN)
#define NRF_CE_CLR JIO_CLR(NRF_CS_PORT, NRF_CS_PIN)

#define NRF_CS_PORT GPIOB
#define NRF_CS_PIN GPIO_Pin_1
#define NRF_CS_INIT JIO_setOut(GPIOA, .GPIO_Pin = NRF_CS_PIN);
#define NRF_CS_SET JIO_SET(NRF_CE_PORT, NRF_CE_PIN)
#define NRF_CS_CLR JIO_CLR(NRF_CE_PORT, NRF_CE_PIN)

#define DELAY_TIME 5000

uint8_t g_p_variant = 0;
uint8_t g_payloadSize = 0;
uint8_t g_dynamicPayloads = 0;
uint64_t g_pipe0ReadingAddress;

// send a single command, always gives status.
uint8_t NRF_cmd(uint8_t reg) {
  uint8_t status;

  NRF_CS_CLR;

  status = SPI_transfer(reg);

  NRF_CS_SET;
  return status;
}

// write data to reg
uint8_t NRF_wcmd(uint8_t reg, uint8_t data) {
  uint8_t status;

  NRF_CS_CLR;

  status = SPI_transfer(reg);
  SPI_transfer(data);

  NRF_CS_SET;
  return status;
}

// write to multiple registers
uint8_t NRF_readMultibyteReg(uint8_t reg, uint8_t* buf, uint8_t len) {
  uint8_t ret;
  ret = SPI_transfer(R_REGISTER | (REGISTER_MASK & reg));
  while(len--) {
    *buf++ = SPI_transfer(0xFF);
  }
  return ret;
}

// write to multiple registers
uint8_t NRF_writeMultibyteReg(uint8_t reg, const uint8_t* buf, uint8_t len) {
  uint8_t ret;
  ret = SPI_transfer(W_REGISTER | (REGISTER_MASK & reg));
  while(len--) {
    SPI_transfer(*buf++);
  }
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

  NRF_CE_CLR;
  NRF_CS_SET;

  /* TODO:  different delay for reset vs power up.*/
  delay(DELAY_TIME);

  NRF_writeReg(SETUP_RETR, (0b0100 << ARD) | (0b1111 << ARC));

  NRF_setPALevel(RF24_PA_MAX);

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

void NRF_startWrite( const void* buf, uint8_t len )
{
  // Transmitter power-up
  NRF_writeReg(CONFIG, ( NRF_readReg(CONFIG) | (1<<PWR_UP) ) & ~(1<<PRIM_RX) );
  delay(DELAY_TIME);

  // Send the payload
  NRF_writePayload( buf, len );

  // Allons!
  NRF_CE_SET;
  delay(DELAY_TIME);
  NRF_CE_CLR;
}

uint8_t NRF_write( const void* buf, uint8_t len )
{
  uint8_t result = 0;

  // Begin the write
  NRF_startWrite(buf,len);

  // ------------
  // At this point we could return from a non-blocking write, and then call
  // the rest after an interrupt

  // Instead, we are going to block here until we get TX_DS (transmission completed and ack'd)
  // or MAX_RT (maximum retries, transmission failed).  Also, we'll timeout in case the radio
  // is flaky and we get neither.

  // IN the end, the send should be blocking.  It comes back in 60ms worst case, or much faster
  // if I tighted up the retry logic.  (Default settings will be 1500us.
  // Monitor the send
  uint8_t observe_tx;
  uint8_t status;
  do
  {
    status = NRF_readMultibyteReg(OBSERVE_TX,&observe_tx,1);
  }
  while( ! ( status & ( (1<<TX_DS) | (1<<MAX_RT) ) ) ); /* TODO:  timeout... */

  // The part above is what you could recreate with your own interrupt handler,
  // and then call this when you got an interrupt
  // ------------

  /* TODO: check if transfer worked... */

  // NOTE: NO ACK IN THIS IMPLEMENTATION

  // Yay, we are done.

  // Power down
  NRF_powerDown();

  // Flush buffers (Is this a relic of past experimentation, and not needed anymore??)
  NRF_flushTx();

  return (status&(1<<TX_DS));
}
