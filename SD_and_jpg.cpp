#include "SD2Card.h"
#define sd_cs  4
Sd2Card card;

// this function determines the minimum of two numbers
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))

#include <D:\Temp\Arduino\MY_TFT_GPS\Adafruit_ST7735.h>

extern char njDecode(short);

#define BLOCKSZ 512
#define BUFSZ 132
#define HALFBUFSZ 66
unsigned char buf[BUFSZ];
short jpgsize=2723;//3662;
short OffsetInSDBlock = 0;
int32_t SDBlockNum = 2561;//26820009;//total blocks:31260672
char SD_Inited;


void SD_Setup()
{
  SD_Inited = true;

  pinMode(SPI_MISO_PIN, INPUT);
  if(!card.init(sd_cs)) 
    SD_Inited = false;
  else {
  }
}

int SDread(unsigned char *pbuf,int cnt)
{ if(!SD_Inited)return 0;
  if(OffsetInSDBlock>=BLOCKSZ)
  { ++SDBlockNum;
    OffsetInSDBlock = BLOCKSZ-OffsetInSDBlock;
  }
  int lend = OffsetInSDBlock+cnt;
  int tail = lend - BLOCKSZ;
  if(tail<=0)
  { if(card.readData(SDBlockNum,OffsetInSDBlock,cnt,pbuf))
    { OffsetInSDBlock += cnt;
      return cnt;
    }
    return 0;
  }
  
  int fst = BLOCKSZ - OffsetInSDBlock;
  if(card.readData(SDBlockNum++,OffsetInSDBlock,fst,pbuf))
  { pbuf += fst;
    fst = cnt - fst;
    OffsetInSDBlock = 0;
    if(card.readData(SDBlockNum,OffsetInSDBlock,fst,pbuf))
    { OffsetInSDBlock = fst;
      return cnt;
    }else return BLOCKSZ - OffsetInSDBlock;
  }
  else return 0;
}

//====================================================================================
//   Main loop
//====================================================================================
 void JPG_loop() {

  if(SD_Inited)
  { 
    if(card.readData(SDBlockNum,OffsetInSDBlock,BUFSZ,buf))
    { jpgsize = (buf[0]<<8) | buf[1];
      OffsetInSDBlock = BUFSZ;
      char r = njDecode(jpgsize);//to TFT ret NJ_OK if all right;
    }
  }
}
