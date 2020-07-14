#include <Arduino.h>
#include "Sd2Card.h"
#include <D:\Temp\Arduino\MY_TFT_GPS\Adafruit_ST7735.h>

//------------------------------------------------------------------------------
// functions for hardware SPI
/** Send a byte to the card */
static void spiSend(uint8_t b) {
  SPDR = b;
  while (!(SPSR & (1 << SPIF)));
  //SPSR &= ~(1 << SPIF);
}

/** Receive a byte from the card */
static  uint8_t spiRec(void) {
  SPDR = 0xff;
  while(!(SPSR & (1 << SPIF)));
  //SPSR &= ~(1 << SPIF);
  return SPDR;
}

// send command and return error code.  Return zero for OK
uint8_t Sd2Card::cardCommand(uint8_t cmd, uint32_t arg) {
  
  // send command
  spiSend(cmd | 0x40);

  // send argument
  for (int8_t s = 24; s >= 0; s -= 8) spiSend(arg >> s);

  // send CRC
  uint8_t crc = (cmd == CMD0 ? 0x95 : 0xff);//0XFF;
  if (cmd == CMD8) crc = 0X87; // correct crc for CMD8 with arg 0X1AA
  spiSend(crc);

  // wait for response
  for (uint8_t i = 0; ((status_ = spiRec()) & 0X80) && i < 10/*!=0XFF*/; i++)
  ;
  return status_;
}

void Sd2Card::chipSelectHigh(void) {
  digitalWrite(chipSelectPin_, HIGH);
}

void Sd2Card::chipSelectLow(void) {
  digitalWrite(chipSelectPin_, LOW);
}

uint8_t Sd2Card::init(uint8_t chipSelectPin) {
  errorCode_ = type_ = 0;
  chipSelectPin_ = chipSelectPin;
  // 16-bit init start time allows over a minute
  uint16_t t0 = (uint16_t)millis();
  uint32_t arg;

  // set pin modes
  pinMode(chipSelectPin_, OUTPUT);
  digitalWrite(chipSelectPin_, HIGH);

  // SS must be in output mode even it is not chip select
  digitalWrite(SS_PIN, HIGH); // disable any SPI device using hardware SS pin
 
  for (uint8_t i = 0; i < 10; i++) spiSend(0XFF);
            
  chipSelectLow();

  // command to go idle in SPI mode
  while ((status_ = cardCommand(CMD0, 0)) != R1_IDLE_STATE) {
    if (((uint16_t)(millis() - t0)) > SD_INIT_TIMEOUT) {char s[16];
      error(SD_CARD_ERROR_CMD0);
      goto fail;
    }
  }
  
  // check SD version
  if ((cardCommand(CMD8, 0x1AA) & R1_ILLEGAL_COMMAND)) {
    type(SD_CARD_TYPE_SD1);
  } else {
    // only need last byte of r7 response
    for (uint8_t i = 0; i < 4; i++) status_ = spiRec();
    if (status_ != 0XAA) {
      type(SD_CARD_TYPE_SD1);//error(SD_CARD_ERROR_CMD8);
      //goto fail;
    }
    type(SD_CARD_TYPE_SD2);
  }
  // initialize card and send host supports SDHC if SD2
  arg = type() == SD_CARD_TYPE_SD2 ? 0X40000000 : 0;

  while ((status_ = cardAcmd(ACMD41, arg)) != R1_READY_STATE) {
    // check for timeout
    if (((uint16_t)millis() - t0) > SD_INIT_TIMEOUT) {
      error(SD_CARD_ERROR_ACMD41);
      goto fail;
     }
  }
  // if SD2 read OCR register to check for SDHC card
  if (type() == SD_CARD_TYPE_SD2) {
    if (cardCommand(CMD58, 0)) {
      error(SD_CARD_ERROR_CMD58);
      goto fail;
    }
    if ((spiRec() & 0XC0) == 0XC0) type(SD_CARD_TYPE_SDHC);
    // discard rest of ocr - contains allowed voltage range
    for (uint8_t i = 0; i < 3; i++) spiRec();
  }
  
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}

uint8_t Sd2Card::readData(int32_t block, short offset, short count, uint8_t* dst) {
  if (count == 0) return true;
  if ((count + offset) > 512)
    goto fail;
  short offset_;
  //if (!inBlock_ || block != block_ || offset < offset_) {
  //  block_ = block;
    // use address if not SDHC card
    if (type()!= SD_CARD_TYPE_SDHC) block <<= 9;
    chipSelectLow();
    if (cardCommand(CMD17, block)) {
      error(SD_CARD_ERROR_CMD17);
      goto fail;
    }
    if (!waitStartBlock())
      goto fail;
   
    //offset_ = 0;
    //inBlock_ = 1;
  //}
  //else chipSelectLow();
  // skip data before offset
  for (offset_=0; offset_ < offset; offset_++) {
    spiRec();
  }
  // transfer data
  for (short i = 0; i < count; i++) {
    dst[i] = spiRec();
  }

  //offset_ += count;
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}

/*void Sd2Card::readEnd(void) {
  //if (inBlock_) {
      // skip data and crc
    while (offset_++ < 512) spiRec();
    chipSelectHigh();
    inBlock_ = 0;
  //}
}*/

uint8_t Sd2Card::waitStartBlock(void) {
  uint16_t t0 = millis();
  while ((status_ = spiRec()) == 0XFF) {
    if (((uint16_t)millis() - t0) > SD_READ_TIMEOUT) {
      error(SD_CARD_ERROR_READ_TIMEOUT);
      goto fail;
    }
  }
  if (status_ != DATA_START_BLOCK) {
    error(SD_CARD_ERROR_READ);char s[16];
    goto fail;
  }
  return true;

 fail:
  chipSelectHigh();
  return false;
}

uint8_t Sd2Card::waitNotBusy(uint16_t timeoutMillis) {
  uint16_t t0 = millis();
  do {
    if (spiRec() == 0XFF) return true;
  }
  while (((uint16_t)millis() - t0) < timeoutMillis);
  return false;
}
