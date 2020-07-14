#include "Adafruit_ST7735.h"

extern void *memset(void *, int, long); 


typedef struct TMyRGB{
  unsigned char rgb1[256];  // 16 * 16
  unsigned char rgb2[64];   // 8  * 8
  unsigned char rgb3[64];   // 8  * 8
} MyRGB, *pMyRGB;
static pMyRGB myRGB;


typedef enum _nj_result {
  NJ_OK = 0,        // no error, decoding successful
  NJ_NO_JPEG,       // not a JPEG file
  NJ_UNSUPPORTED,   // unsupported format
  NJ_OUT_OF_MEM,    // out of memory
  NJ_INTERNAL_ERR,  // internal error
  NJ_SYNTAX_ERROR,  // syntax error
  __NJ_FINISHED,    // used internally, will never be reported
} nj_result_t;

void njInit(void);
char njDecode(void* jpeg, int size);
int njGetWidth(void);
int njGetHeight(void);
int njIsColor(void);
int njGetImageSize(void);
void njConvert(int, int);


#define njAllocMem malloc
#define njFreeMem  free
#define njFillMem  memset
#define njCopyMem  memcpy


typedef struct _nj_cmp {
  unsigned char ssx, ssy;
  unsigned char width, height;
  unsigned char stride;
  unsigned char qtsel;
  unsigned char actabsel, dctabsel;
  short dcpred;
} nj_component_t;

static const unsigned char qtab[2][64] PROGMEM = {// For Adobe CS3 JPEG compacting level 3,4,5 - Medium
  {12,8,8,8,9,8,12,9,9,12,17,11,10,11,17,21,15,12,12,15,21,24,19,19,21,19,19,24,17,12,12,12,12,
  12,12,17,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12},
  {13,11,11,13,14,13,16,14,14,16,20,14,14,14,20,20,14,14,14,14,20,17,12,12,12,12,12,17,17,12,12,12,
  12,12,12,17,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12}};
typedef struct _nj_code {//__declspec(align(3)) alignment must be power of two
  unsigned char bits_cnt, code;
  /*void set(unsigned char b, unsigned char c, short cn)
  { int x = 0;for (; !(cn & 1); ++x, cn >>= 1);
    bits_cnt = ((b - 1) << 4) | x;
    code = c;
  }*/
  unsigned char getBits() { return 1 + (bits_cnt >> 4); }//1 + ((bits_cnt & 0xf0) >> 4);}
  short getCnt(){return 1 << (bits_cnt & 0x0f);}
} nj_vlc_code_t;
static const unsigned char/*nj_vlc_code_t*/ PROGMEM vlctab[502]/*[12]*/ = {// For Adobe CS3 JPEG compacting level 3,4,5 - Medium
2,14,26,140,//0,12,24,138,
30,  3,  //2,   3 ,   16384,                      (12+12+114+111)*2+4=502
45,  0,  //3,   0 ,   8192,
45,  1,  //3,   1 ,   8192,
45,  2,  //3,   2 ,   8192,
45,  4,  //3,   4 ,   8192,
45,  5,  //3,   5 ,   8192,
60,  6,  //4,   6 ,   4096,
75,  7,  //5,   7 ,   2048,
90,  8,  //6,   8 ,   1024,
105, 9,  //7,   9 ,   512,
120, 10, //8,   10,   256,
135, 11,//};//9,   11,   128,// };
//static const unsigned char/*nj_vlc_code_t*/ vlctab1[/*12*/] PROGMEM = {// For Adobe CS3 JPEG compacting level 3,4,5 - Medium
30,  1,//2,   1 ,   16384,
45,  0,//3,   0 ,   8192  ,
45,  2,//3,   2 ,   8192  ,
45,  3,//3,   3 ,   8192  ,
45,  4,//3,   4 ,   8192  ,
45,  5,//3,   5 ,   8192  ,
60,  6,//4,   6 ,   4096  ,
75,  7,//5,   7 ,   2048  ,
90,  8,//6,   8 ,   1024  ,
105, 9,//7,   9 ,   512 ,
120,10,//8,   10,   256 ,
135,11,//};//9,   11,   128  };*/
//static const unsigned char/*nj_vlc_code_t*/ vlctab2[/*114*/] PROGMEM = {// For Adobe CS3 JPEG compacting level 3,4,5 - Medium
30,  1,//2 ,  1 ,    16384,
45,  0,//3 ,  0 ,    8192,
45,  2,//3 ,  2 ,    8192,
45,  17,//3 ,  17,    8192,
45,  3,//3 ,  3 ,    8192,
60,  4,//4 ,  4 ,    4096,
75,  33,//5 ,  33,    2048,
75,  18,//5 ,  18,    2048,
75,  49,//5 ,  49,    2048,
90,  5,//6 ,  5 ,    1024,
90,  65,//6 ,  65,    1024,
105,  81,//7 ,  81,    512,
105,  97,//7 ,  97,    512,
105,  19,//7 ,  19,    512,
105,  34,//7 ,  34,    512,
120,  113,//8 ,  113,   256,
120,  129,//8 ,  129,   256,
135,  50,//9 ,  50 ,   128,
135,  6,//9 ,  6  ,   128,
135,  20,//9 ,  20 ,   128,
135,  145,//9 ,  145,   128,
135,  161,//9 ,  161,   128,
150,  177,//10,  177,   64,
150,  66,//10,  66 ,   64,
150,  35,//10,  35 ,   64,
150,  36,//10,  36 ,   64,
150,  21,//10,  21 ,   64,
150,  82,//10,  82 ,   64,
150,  193,//10,  193,   64,
165,  98,//11,  98 ,   32,
165,  51,//11,  51 ,   32,
165,  52,//11,  52 ,   32,
165,  114,//11,  114,   32,
165,  130,//11,  130,   32,
165,  209,//11,  209,   32,
180,  67,//12,  67 ,   16,
180,  7,//12,  7  ,   16,
180,  37,//12,  37 ,   16,
180,  146,//12,  146,   16,
180,  83,//12,  83 ,   16,
180,  240,//12,  240,   16,
180,  225,//12,  225,   16,
180,  241,//12,  241,   16,
195,  99,//13,  99 ,   8,
195,  115,//13,  115,   8,
195,  53,//13,  53 ,   8,
195,  22,//13,  22 ,   8,
195,  162,//13,  162,   8,
210,  178,//14,  178,   4,
210,  131,//14,  131,   4,
210,  38,//14,  38 ,   4,
225,  68,//15,  68 ,   2,
225,  147,//15,  147,   2,
225,  84,//15,  84 ,   2,
225,  100,//15,  100,   2,
225,  69,//15,  69 ,   2,
225,  194,//15,  194,   2,
225,  163,//15,  163,   2,
225,  116,//15,  116,   2,
225,  54,//15,  54 ,   2,
225,  23,//15,  23 ,   2,
225,  210,//15,  210,   2,
225,  85,//15,  85 ,   2,
240,  226,//16,  226,   1,
240,  101,//16,  101,   1,
240,  242,//16,  242,   1,
240,  179,//16,  179,   1,
240,  132,//16,  132,   1,
240,  195,//16,  195,   1,
240,  211,//16,  211,   1,
240,  117,//16,  117,   1,
240,  227,//16,  227,   1,
240,  243,//16,  243,   1,
240,  70,//16,  70 ,   1,
240,  39,//16,  39 ,   1,
240,  148,//16,  148,   1,
240,  164,//16,  164,   1,
240,  133,//16,  133,   1,
240,  180,//16,  180,   1,
240,  149,//16,  149,   1,
240,  196,//16,  196,   1,
240,  212,//16,  212,   1,
240,  228,//16,  228,   1,
240,  244,//16,  244,   1,
240,  165,//16,  165,   1,
240,  181,//16,  181,   1,
240,  197,//16,  197,   1,
240,  213,//16,  213,   1,
240,  229,//16,  229,   1,
240,  245,//16,  245,   1,
240,  86,//16,  86 ,   1,
240,  102,//16,  102,   1,
240,  118,//16,  118,   1,
240,  134,//16,  134,   1,
240,  150,//16,  150,   1,
240,  166,//16,  166,   1,
240,  182,//16,  182,   1,
240,  198,//16,  198,   1,
240,  214,//16,  214,   1,
240,  230,//16,  230,   1,
240,  246,//16,  246,   1,
240,  55,//16,  55 ,   1,
240,  71,//16,  71 ,   1,
240,  87,//16,  87 ,   1,
240,  103,//16,  103,   1,
240,  119,//16,  119,   1,
240,  135,//16,  135,   1,
240,  151,//16,  151,   1,
240,  167,//16,  167,   1,
240,  183,//16,  183,   1,
240,  199,//16,  199,   1,
240,  215,//16,  215,   1,
240,  231,//16,  231,   1,
240,  247,// };//16,  247,   1};*/
//static const nj_vlc_code_t vlctab3[111] PROGMEM = {// For Adobe CS3 JPEG compacting level 3,4,5 - Medium
30,  1,//2 ,  1  ,    16384,
30,  0,//2 ,  0  ,    16384,
45,  2,//3 ,  2  ,    8192,
45,  17,//3 ,  17 ,    8192,
60,  3,//4 ,  3  ,    4096,
75,  33,//5 ,  33 ,    2048,
75,  49,//5 ,  49 ,    2048,
90,  18,//6 ,  18 ,    1024,
90,  4,//6 ,  4  ,    1024,
90,  65,//6 ,  65 ,    1024,
90,  81,//6 ,  81 ,    1024,
105,  97,//7 ,  97 ,    512,
105,  113,//7 ,  113,    512,
105,  34,//7 ,  34 ,    512,
105,  19,//7 ,  19 ,    512,
120,  5,//8 ,  5  ,    256,
120,  50,//8 ,  50 ,    256,
120,  129,//8 ,  129,    256,
135,  145,//9 ,  145,    128,
135,  20,//9 ,  20 ,    128,
135,  161,//9 ,  161,    128,
135,  177,//9 ,  177,    128,
150,  66,//10,  66 ,    64,
150,  35,//10,  35 ,    64,
150,  193,//10,  193,    64,
150,  82,//10,  82 ,    64,
150,  209,//10,  209,    64,
165,  240,//11,  240,    32,
165,  51,//11,  51 ,    32,
165,  36,//11,  36 ,    32,
165,  98,//11,  98 ,    32,
165,  225,//11,  225,    32,
165,  114,//11,  114,    32,
180,  130,//12,  130,    16,
180,  146,//12,  146,    16,
180,  67,//12,  67 ,    16,
180,  83,//12,  83 ,    16,
180,  21,//12,  21 ,    16,
180,  99,//12,  99 ,    16,
180,  115,//12,  115,    16,
195,  52,//13,  52 ,    8,
195,  241,//13,  241,    8,
195,  37,//13,  37 ,    8,
195,  6,//13,  6  ,    8,
195,  22,//13,  22 ,    8,
195,  162,//13,  162,    8,
195,  178,//13,  178,    8,
210,  131,//14,  131,    4,
210,  7,//14,  7  ,    4,
210,  38,//14,  38 ,    4,
210,  53,//14,  53 ,    4,
210,  194,//14,  194,    4,
210,  210,//14,  210,    4,
225,  68,//15,  68 ,    2,
225,  147,//15,  147,    2,
225,  84,//15,  84 ,    2,
225,  163,//15,  163,    2,
225,  23,//15,  23 ,    2,
240,  100,//16,  100,    1,
240,  69,//16,  69 ,    1,
240,  85,//16,  85 ,    1,
240,  54,//16,  54 ,    1,
240,  116,//16,  116,    1,
240,  101,//16,  101,    1,
240,  226,//16,  226,    1,
240,  242,//16,  242,    1,
240,  179,//16,  179,    1,
240,  132,//16,  132,    1,
240,  195,//16,  195,    1,
240,  211,//16,  211,    1,
240,  117,//16,  117,    1,
240,  227,//16,  227,    1,
240,  243,//16,  243,    1,
240,  70,//16,  70 ,    1,
240,  148,//16,  148,    1,
240,  164,//16,  164,    1,
240,  133,//16,  133,    1,
240,  180,//16,  180,    1,
240,  149,//16,  149,    1,
240,  196,//16,  196,    1,
240,  212,//16,  212,    1,
240,  228,//16,  228,    1,
240,  244,//16,  244,    1,
240,  165,//16,  165,    1,
240,  181,//16,  181,    1,
240,  197,//16,  197,    1,
240,  213,//16,  213,    1,
240,  229,//16,  229,    1,
240,  245,//16,  245,    1,
240,  86,//16,  86 ,    1,
240,  102,//16,  102,    1,
240,  118,//16,  118,    1,
240,  134,//16,  134,    1,
240,  150,//16,  150,    1,
240,  166,//16,  166,    1,
240,  182,//16,  182,    1,
240,  198,//16,  198,    1,
240,  214,//16,  214,    1,
240,  230,//16,  230,    1,
240,  246,//16,  246,    1,
240,  39,//16,  39 ,    1,
240,  55,//16,  55 ,    1,
240,  71,//16,  71 ,    1,
240,  87,//16,  87 ,    1,
240,  103,//16,  103,    1,
240,  119,//16,  119,    1,
240,  135,//16,  135,    1,
240,  151,//16,  151,    1,
240,  167,//16,  167,    1,
240,  183,//16,  183,    1,
240,  199};//16,  199,   1 };

typedef struct _nj_ctx {
  nj_result_t error;
  unsigned char *pos;
  short size;
  short length;
  short width, height;
  short mbwidth, mbheight;
  short mbsizex, mbsizey;
  char ncomp;
  nj_component_t comp[3];//21*3=63
  unsigned short qtused, qtavail;
  int32_t buf, bufbits;
  short block[64];
  unsigned short rstinterval;
} nj_context_t;
static nj_context_t nj;

static const char njZZ[64] PROGMEM = { 0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18,
11, 4, 5, 12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35,
42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51, 58, 59, 52, 45,
38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63 };

unsigned char njClip(int x) {
  return (x < 0) ? 0 : ((x > 0xFF) ? 0xFF : (unsigned char)x);
}

#define W1 2841
#define W2 2676
#define W3 2408
#define W5 1609
#define W6 1108
#define W7 565

inline void mm32(short* px, short s)
{
  *px++ = s;
  *px = (s < 0 ? 0xffff:0);
}

void njRowIDCT(short* blk) {
  int32_t x0, x1, x2, x3, x4, x5, x6, x7, x8;
  mm32((short*)&x1, blk[4]);x1 <<= 11;
  mm32((short*)&x2, blk[6]);
  mm32((short*)&x3, blk[2]);
  mm32((short*)&x4, blk[1]);
  mm32((short*)&x5, blk[7]);
  mm32((short*)&x6, blk[5]);
  mm32((short*)&x7, blk[3]);

  if (!(x1 | x2 | x3 | x4 | x5 | x6 | x7))
  { blk[0] = blk[1] = blk[2] = blk[3] = blk[4] = blk[5] = blk[6] = blk[7] = blk[0] << 3;
    return;
  }   
  mm32((short*)&x0,blk[0]);x0 = (x0 << 11) + 128;
  x8 = W7 * (x4 + x5);
  x4 = x8 + (W1 - W7) * x4;
  x5 = x8 - (W1 + W7) * x5;
  x8 = W3 * (x6 + x7);
  x6 = x8 - (W3 - W5) * x6;
  x7 = x8 - (W3 + W5) * x7;
  x8 = x0 + x1;
  x0 -= x1;
  x1 = W6 * (x3 + x2);
  x2 = x1 - (W2 + W6) * x2;
  x3 = x1 + (W2 - W6) * x3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;
  x7 = x8 + x3;
  x8 -= x3;
  x3 = x0 + x2;
  x0 -= x2;
  x2 = (181 * (x4 + x5) + 128) >> 8;
  x4 = (181 * (x4 - x5) + 128) >> 8;
  blk[0] = (x7 + x1) >> 8;
  blk[1] = (x3 + x2) >> 8;
  blk[2] = (x0 + x4) >> 8;
  blk[3] = (x8 + x6) >> 8;
  blk[4] = (x8 - x6) >> 8;
  blk[5] = (x0 - x4) >> 8;
  blk[6] = (x3 - x2) >> 8;
  blk[7] = (x7 - x1) >> 8;
}

void njColIDCT(short* blk, unsigned char *out, unsigned char stride) {
  int32_t x0, x1, x2, x3, x4, x5, x6, x7, x8;
  mm32((short*)&x1, blk[8 * 4]);x1 <<= 8;
  mm32((short*)&x2, blk[8 * 6]);
  mm32((short*)&x3, blk[8 * 2]);
  mm32((short*)&x4, blk[8]);
  mm32((short*)&x5, blk[8 * 7]);
  mm32((short*)&x6, blk[8 * 5]);
  mm32((short*)&x7, blk[8 * 3]);
  if (!(x1 | x2 | x3 | x4 | x5 | x6 | x7))
  {
    x1 = (int32_t)njClip(((blk[0] + 32) >> 6) + 128);
    for (x0 = 8; x0; --x0) {
      *out = (unsigned char)x1;
      out += stride;
    }
    return;
  }
  mm32((short*)&x0, blk[0]); x0 = (x0 << 8) + 8192;
  x8 = W7 * (x4 + x5) + 4;
  x4 = (x8 + (W1 - W7) * x4) >> 3;
  x5 = (x8 - (W1 + W7) * x5) >> 3;
  x8 = W3 * (x6 + x7) + 4;
  x6 = (x8 - (W3 - W5) * x6) >> 3;
  x7 = (x8 - (W3 + W5) * x7) >> 3;
  x8 = x0 + x1;
  x0 -= x1;
  x1 = W6 * (x3 + x2) + 4;
  x2 = (x1 - (W2 + W6) * x2) >> 3;
  x3 = (x1 + (W2 - W6) * x3) >> 3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;
  x7 = x8 + x3;
  x8 -= x3;
  x3 = x0 + x2;
  x0 -= x2;
  x2 = (181 * (x4 + x5) + 128) >> 8;
  x4 = (181 * (x4 - x5) + 128) >> 8;
  *out = njClip(((x7 + x1) >> 14) + 128);  out += stride;
  *out = njClip(((x3 + x2) >> 14) + 128);  out += stride;
  *out = njClip(((x0 + x4) >> 14) + 128);  out += stride;
  *out = njClip(((x8 + x6) >> 14) + 128);  out += stride;
  *out = njClip(((x8 - x6) >> 14) + 128);  out += stride;
  *out = njClip(((x0 - x4) >> 14) + 128);  out += stride;
  *out = njClip(((x3 - x2) >> 14) + 128);  out += stride;
  *out = njClip(((x7 - x1) >> 14) + 128);  
}

#define njThrow(e) do { nj.error = e; return; } while (0)
#define njCheckError() do { if (nj.error) return; } while (0)

#define BLOCKSZ 512
#define BUFSZ 132
#define HALFBUFSZ 66
extern unsigned char buf[BUFSZ];
extern int SDread(unsigned char*,int);

void CheckFileBuf()
{
  int fill = nj.pos - &buf[0];
  if (fill >= HALFBUFSZ)  //66          We must read from file and fill buf...
  {
    int i = 0, tail = BUFSZ - fill + 2;
    unsigned char *pb = &buf[0];
    unsigned char *pe = nj.pos-2;//&buf[fill - 2];
    for (; i < tail; i++)        //132
      *pb++ = *pe++;
     /*readed +=*/ SDread(pb, fill - 2);   //2 ta peshba memonem, [-to 2 ya];    
    nj.pos = &buf[2];
  }
 }

int32_t njShowBits(unsigned char bits) {
  unsigned char newbyte;
  if (!bits) return 0;
   while (nj.bufbits < bits) {
    if (nj.size <= 0) {
      nj.buf = (nj.buf << 8) | 0xFF;
      nj.bufbits += 8;
      continue;
    }
    newbyte = *nj.pos++;CheckFileBuf();
    nj.size--;
    nj.bufbits += 8;
    nj.buf = (nj.buf << 8) | newbyte;
    if (newbyte == 0xFF) {
      if (nj.size) {
        unsigned char marker = *nj.pos++;CheckFileBuf();
       nj.size--;
        switch (marker) {
        case 0x00:
        case 0xFF:
          break;
        case 0xD9: nj.size = 0; break;
        default:
          if ((marker & 0xF8) != 0xD0)
            nj.error = NJ_SYNTAX_ERROR;
          else {
            nj.buf = (nj.buf << 8) | marker;
            nj.bufbits += 8;
          }
        }
      }
      else
        nj.error = NJ_SYNTAX_ERROR;
    }
  }
  return (nj.buf >> (nj.bufbits - bits)) & ((1 << bits) - 1);
}

void njSkipBits(unsigned char bits) {
  if (nj.bufbits < bits)
    (void) njShowBits(bits);
  nj.bufbits -= bits;
}

int32_t njGetBits(unsigned char bits) {
  int32_t res = njShowBits(bits);
  njSkipBits(bits);
  return res;
}

void njByteAlign(void) {
  nj.bufbits &= 0xF8;
}

void njSkip(short count) {
  nj.pos += count;                //count byte read
  nj.size -= count;
  nj.length -= count;
  if (nj.size < 0) nj.error = NJ_SYNTAX_ERROR;
  CheckFileBuf();
 }

unsigned short njDecode16(unsigned char *pos) {
  return (pos[0] << 8) | pos[1];                //2 byte read
}

void njDecodeLength(void) {
  if (nj.size < 2) njThrow(NJ_SYNTAX_ERROR);
  nj.length = njDecode16(nj.pos);
  if (nj.length > nj.size) njThrow(NJ_SYNTAX_ERROR);
  njSkip(2);
}

void njDecodeSOF(void) {
  int ssxmax = 0, ssymax = 0;unsigned char i;
  nj_component_t* c;
  njDecodeLength();
  njCheckError();
  if (nj.length < 9) njThrow(NJ_SYNTAX_ERROR);
  if (nj.pos[0] != 8) njThrow(NJ_UNSUPPORTED);
  nj.height = njDecode16(nj.pos + 1);
  nj.width = njDecode16(nj.pos + 3);
  if (!nj.width || !nj.height) njThrow(NJ_SYNTAX_ERROR);
  nj.ncomp = nj.pos[5];
  njSkip(6);
  switch (nj.ncomp) {
  case 1:
  case 3:
    break;
  default:
    njThrow(NJ_UNSUPPORTED);
  }
  if (nj.length < (nj.ncomp * 3)) njThrow(NJ_SYNTAX_ERROR);
  for (i = 0, c = nj.comp; i < nj.ncomp; ++i, ++c) {
    if (!(c->ssx = nj.pos[1] >> 4)) njThrow(NJ_SYNTAX_ERROR);
    if (c->ssx & (c->ssx - 1)) njThrow(NJ_UNSUPPORTED);  // non-power of two
    if (!(c->ssy = nj.pos[1] & 15)) njThrow(NJ_SYNTAX_ERROR);
    if (c->ssy & (c->ssy - 1)) njThrow(NJ_UNSUPPORTED);  // non-power of two
    if ((c->qtsel = nj.pos[2]) & 0xFC) njThrow(NJ_SYNTAX_ERROR);
    njSkip(3);
    nj.qtused |= 1 << c->qtsel;
    if (c->ssx > ssxmax) ssxmax = c->ssx;
    if (c->ssy > ssymax) ssymax = c->ssy;
  }
  if (nj.ncomp == 1) {
    c = nj.comp;
    c->ssx = c->ssy = ssxmax = ssymax = 1;
  }
  nj.mbsizex = ssxmax << 3;
  nj.mbsizey = ssymax << 3;
  nj.mbwidth = (nj.width + nj.mbsizex - 1) / nj.mbsizex;
  nj.mbheight = (nj.height + nj.mbsizey - 1) / nj.mbsizey;
  for (i = 0, c = nj.comp; i < nj.ncomp; ++i, ++c) {
    c->width = (nj.width * c->ssx + ssxmax - 1) / ssxmax;
    c->height = (nj.height * c->ssy + ssymax - 1) / ssymax;
    c->stride = nj.mbwidth * c->ssx << 3;
    if (((c->width < 3) && (c->ssx != ssxmax)) || ((c->height < 3) && (c->ssy != ssymax))) njThrow(NJ_UNSUPPORTED);
  }
  njSkip(nj.length);
 }

void njDecodeDRI(void) {
  njDecodeLength();
  njCheckError();
  if (nj.length < 2) njThrow(NJ_SYNTAX_ERROR);
  nj.rstinterval = njDecode16(nj.pos);
  njSkip(nj.length);
}

unsigned short myFindVlc(unsigned char n_vlctab, unsigned short value)
{ 
unsigned char ch[2];
unsigned short sumCntPrev;
uint16_t pvlctab = (uint16_t)&vlctab[0];

  ch[0] = pgm_read_byte_near(pvlctab+n_vlctab);
  pvlctab += (ch[0] << 1);
  
  ch[0] = pgm_read_byte_near(pvlctab);
  sumCntPrev = (1 << (ch[0] & 0x0f));//bits_cnt;
     
  for (;value >= sumCntPrev;)
  { 
    pvlctab += 2;
    ch[0] = pgm_read_byte_near(pvlctab);
    sumCntPrev += (1 << (ch[0] & 0x0f));//bits_cnt;
  }
  ch[1] = pgm_read_byte_near(pvlctab+1);
  return (ch[0] << 8) | ch[1];
}

int32_t njGetVLC(unsigned char n_vlctab, unsigned char* code) {
  int32_t value = njShowBits(16);
  uint16_t vlc = myFindVlc(n_vlctab, value);
  short bits = (short)(1 + (vlc >> 12));
  if (!bits) { nj.error = NJ_SYNTAX_ERROR; return 0; }

  njSkipBits(bits);
  value = vlc & 0xff;
  if (code) *code = (unsigned char)value;
  bits = value & 15;

  if (!bits) return 0;
  value = njGetBits(bits);
  
  if (value < (1 << (bits - 1)))
    value += ((-1) << bits) + 1;
 
  return value;
}

void njDecodeBlock(nj_component_t* c, unsigned char* out, unsigned char strd)
{
  unsigned char code = 0, ch;
  int32_t value, coef = 0;
  njFillMem(nj.block, 0, sizeof(nj.block));

  c->dcpred += njGetVLC(c->dctabsel, 0);
  nj.block[0] = (c->dcpred) * pgm_read_byte_near((uint16_t)&qtab[c->qtsel][0]);

  do {
    value = njGetVLC(c->actabsel, &code);
    if (!code) break;  // EOB
    if (!(code & 0x0F) && (code != 0xF0)) njThrow(NJ_SYNTAX_ERROR);
    coef += (code >> 4) + 1;
    if (coef > 63) njThrow(NJ_SYNTAX_ERROR);
    ch = pgm_read_byte_near((uint16_t)&njZZ[coef]);
    nj.block[(int)ch] = value * pgm_read_byte_near((uint16_t)&qtab[c->qtsel][coef]);
    //sprintf(sdbg," DcdBl: %x %x %x %x\n",code,value,ch,nj.block[(int)ch]);Serial.println(sdbg);
  } while (coef < 63);

  for (coef = 0; coef < 64; coef += 8)
    njRowIDCT(&nj.block[coef]);

  for (coef = 0; coef < 8; ++coef)// 8*8 rect durnashba pur mekunand;
    njColIDCT(&nj.block[coef], &out[coef], strd);//c->stride
}

void njDecodeScan(void) {
  short i, mbx, mby, sbx, sby, k, rstcount = nj.rstinterval, nextrst = 0;
  nj_component_t* c;
  MyRGB rgb;
  myRGB = &rgb;

  njDecodeLength();
  njCheckError();

  if (nj.length < (4 + 2 * nj.ncomp)) njThrow(NJ_SYNTAX_ERROR);
  if (nj.pos[0] != nj.ncomp) njThrow(NJ_UNSUPPORTED);
  njSkip(1);
  for (i = 0, c = nj.comp; i < nj.ncomp; ++i, ++c) {
    //if (nj.pos[0] != c->cid) njThrow(NJ_SYNTAX_ERROR);
    if (nj.pos[1] & 0xEE) njThrow(NJ_SYNTAX_ERROR);
    c->dctabsel = nj.pos[1] >> 4;
    c->actabsel = (nj.pos[1] & 1) | 2;
    njSkip(2);
  }
  if (nj.pos[0] || (nj.pos[1] != 63) || nj.pos[2]) njThrow(NJ_UNSUPPORTED);
  njSkip(nj.length);
  for (mbx = mby = 0;;) {
    for (i = 0, c = nj.comp; i < nj.ncomp; ++i, ++c){
      k = 0;
      for (sby = 0; sby < c->ssy; ++sby) {
        for (sbx = 0; sbx < c->ssx; ++sbx) {
          if(0==i)
            njDecodeBlock(c, &myRGB->rgb1[128*(k/2)+(k%2)*8], 16);
           else if (1 == i)
              njDecodeBlock(c, &myRGB->rgb2[0], 8);
          else//2
            njDecodeBlock(c, &myRGB->rgb3[0], 8);
          njCheckError();
          ++k;
        }
      }
    }
    njConvert(mbx, mby);
    if (++mbx >= nj.mbwidth) {
      mbx = 0;
      if (++mby >= nj.mbheight) break;
    }
    if (nj.rstinterval && !(--rstcount)) {
      njByteAlign();
      i = njGetBits(16);
      if (((i & 0xFFF8) != 0xFFD0) || ((i & 7) != nextrst)){ njThrow(NJ_SYNTAX_ERROR);}
      nextrst = (nextrst + 1) & 7;
      rstcount = nj.rstinterval;
      for (i = 0; i < 3; ++i)
        nj.comp[i].dcpred = 0;
    }
  }
  nj.error = __NJ_FINISHED;
}

void njConvert(int mbx, int mby){
  int x, yy;
  unsigned char *py = &myRGB->rgb1[0];

  //idbg++;
  x=mbx<<4;yy=mby<<4;

  tft.setAddrWindow(x,yy,x+15,yy+15);
 
  for (yy = 0; yy<16; ++yy) {
    register int yb = (yy >> 1) * 8;
    for (x = 0; x < 16; ++x) {
      register int32_t y = yb + (x >> 1);
      int32_t pcb = myRGB->rgb2[y];
      int32_t pcr = myRGB->rgb3[y];

      y = (int32_t)py[x] << 8;

      register int32_t cb = pcb - 128;
      register int32_t cr = pcr - 128;
      unsigned char r = njClip((y + 359 * cr + 128) >> 8);
      unsigned char g = njClip((y - 88 * cb - 183 * cr + 128) >> 8);
      unsigned char b = njClip((y + 454 * cb + 128) >> 8);
      tft.pushColor(tft.Color565(r,g,b));
    }
    py += 16;
  }
}

void njInit(void) {
  njFillMem(&nj, 0, sizeof(nj_context_t));
}

char njDecode(short size) {
  nj.pos = (unsigned char*)&buf[2];//1 ash size ash....
  nj.size = size & 0x7FFFFFFF;
  if (nj.size < 2)
  { 
    return NJ_NO_JPEG;
  }
   while (!nj.error) {
    if ((nj.size < 2))
    {
      return NJ_SYNTAX_ERROR;
    }
    njSkip(2);
    switch (nj.pos[-1]) {
    case 0xC0: njDecodeSOF();  break;
    //case 0xC4: njDecodeDHT();  break;
    //case 0xDB: njDecodeDQT();  break; // 64 ta o'qish
    case 0xDD: njDecodeDRI();  break;
    case 0xDA: njDecodeScan(); break;
    default:
     return NJ_UNSUPPORTED;
    }
  }
  if (nj.error != __NJ_FINISHED) return nj.error;
  nj.error = NJ_OK;
  return nj.error;
}

int njGetWidth(void) { return nj.width; }
int njGetHeight(void) { return nj.height; }
int njIsColor(void) { return (nj.ncomp != 1); }
int njGetImageSize(void) { return nj.width * nj.height * nj.ncomp; }
