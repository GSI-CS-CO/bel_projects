/* C Standard Includes */
/* ==================================================================================================== */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* GSI LM32 Includes */
/* ==================================================================================================== */
#include "irq.h"
#include "uart.h"
#include "timer.h"
#include "mini_sdb.h"
#include "aux.h"
#include "nau8811_audio_driver.h"

/* Defines */
/* ==================================================================================================== */
#define FLOAT_PI                      3.14159265358979323846f
#define PATTERN_START                 0x51AB24EF /* For testing purpose (looped) */
#define PATTERN_MODIFIER              0x12345678 /* For testing purpose (looped) */
#define SAMPLE_FREQUENCY              48000      /* 48kHz */
#define HW_FIFO_SIZE                  256        /* HW_FIFO_SIZE * 32Bits */
#define START_FREQUENCY               100        /* 100Hz */
#define END_FREQUENCY                 24000      /* 24kHz */
#define STEP_FREQUENCY                1          /* 1Hz */
#define ITERATIONS_PER_STEP           50         /* Delay for frequency change */
#define MAX_PACKET_SIZE_250HZ         192
#define MAX_PACKET_SIZE_500HZ         96
#define MAX_PACKET_SIZE_1KHZ          48
#define MAX_PACKET_SIZE_2KHZ          24
#define MAX_PACKET_SIZE_4KHZ          12
#define MAX_PACKET_SIZE_8KHZ          6
#define MAX_PACKET_SIZE_24KHZ         2
#define GEN_SQUARE_WAVE               0
#define GEN_TRIANGLE_WAVE             1
#define GEN_SINE_WAVE                 2
#define GEN_LINE                      3
#define SIGNAL_MODE_FIXED_FREQUENCY   0          /* Range: -1 ... +1, long calculation time */
#define SIGNAL_MODE_VARIOUS_FREQUENCY 1          /* Range: 0 ... +1, short calculation time */

/* Customize */
/* ==================================================================================================== */
#define GEN_MULTIPLIER                0x0fffffff
#define GEN_WAVEFORM                  GEN_SINE_WAVE
#define MAX_PACKET_SIZE               MAX_PACKET_SIZE_2KHZ
#define SIGNAL_MODE                   SIGNAL_MODE_VARIOUS_FREQUENCY

/* Externals (avoid warnings) */
/* ==================================================================================================== */
extern int mprintf(char const *format, ...);

/* Prototypes */
/* ==================================================================================================== */
float fpSine(float x);
void  vFlushRxFifo(void);
void  vFixedFrequency(void);
void  vVariousFrequency(void);

/* Sine functions */
/* ==================================================================================================== */
float fpSine(float x)
{
  /* Helpers */
  float s = 1.0;
  float d = x / (2*FLOAT_PI);
  long d2 = d;
  
  /* Quadrant */
  x = x - d2*2*FLOAT_PI;
  if (x < 0) { x = x + 2*FLOAT_PI; }
  if (x > FLOAT_PI) { x = x - FLOAT_PI; s = -1.0; }
  if (x > FLOAT_PI/2) x = FLOAT_PI-x;
  
  /* Source: http://forum.devmaster.net/t/fast-and-accurate-sine-cosine/9648 */
  return ((((-1.0/5040*x*x) + 1.0/120)*x*x - 1.0/6)*x*x + 1)*x*s;
}

/* Cordic functions */
/* ==================================================================================================== */
/* pi*2^30 => mulhi(x,pi4) = x*pi/4 */
static const uint32_t pi4 = UINT32_C(3373259426);

/* Compute x*y/2^32 */
static inline uint32_t mulhi(uint32_t a, uint32_t b) {
  uint32_t hi   = (a>>16)*(b>>16);
  uint32_t mid0 = (a>>16)*(b&0xFFFF);
  uint32_t mid1 = (b>>16)*(a&0xFFFF);
  return hi + ((mid0+mid1)>>16);
}

static inline uint32_t div6(uint32_t x) {
  return mulhi(x, UINT32_C(0xAAAAAAAB)) >> 2;
}

/* Input: signed a is interpretted as w=a*pi/2^31
 * Returns: cos(w)*(2^31-1), sin(w)*(2^31-1)
 * Accurate to 27 bits
 */
void cordic(int32_t a, int32_t* sin, int32_t* cos) {
  /* We decompose a into three components
   *  sign+mux:  2 bits
   *  table:     6 bits
   *  fraction: 24 bits
   */
  
  /* Table of sin(x*pi/2^7)*(2^31-1) */
  static const int32_t sin_table[65] = {
    INT32_C(         0), INT32_C(  52701887),
    INT32_C( 105372028), INT32_C( 157978697),
    INT32_C( 210490206), INT32_C( 262874923),
    INT32_C( 315101294), INT32_C( 367137860),
    INT32_C( 418953276), INT32_C( 470516330),
    INT32_C( 521795963), INT32_C( 572761285),
    INT32_C( 623381597), INT32_C( 673626408),
    INT32_C( 723465451), INT32_C( 772868706),
    INT32_C( 821806413), INT32_C( 870249095),
    INT32_C( 918167571), INT32_C( 965532978),
    INT32_C(1012316784), INT32_C(1058490807),
    INT32_C(1104027236), INT32_C(1148898640),
    INT32_C(1193077990), INT32_C(1236538675),
    INT32_C(1279254515), INT32_C(1321199780),
    INT32_C(1362349204), INT32_C(1402677999),
    INT32_C(1442161874), INT32_C(1480777044),
    INT32_C(1518500249), INT32_C(1555308767),
    INT32_C(1591180425), INT32_C(1626093615),
    INT32_C(1660027308), INT32_C(1692961061),
    INT32_C(1724875039), INT32_C(1755750016),
    INT32_C(1785567395), INT32_C(1814309215),
    INT32_C(1841958164), INT32_C(1868497585),
    INT32_C(1893911493), INT32_C(1918184580),
    INT32_C(1941302224), INT32_C(1963250500),
    INT32_C(1984016188), INT32_C(2003586778),
    INT32_C(2021950483), INT32_C(2039096240),
    INT32_C(2055013722), INT32_C(2069693341),
    INT32_C(2083126253), INT32_C(2095304369),
    INT32_C(2106220351), INT32_C(2115867625),
    INT32_C(2124240379), INT32_C(2131333571),
    INT32_C(2137142926), INT32_C(2141664947),
    INT32_C(2144896909), INT32_C(2146836865),
    INT32_C(2147483647)
  };
  
  /* Lookup table for high bits */
  uint32_t am = (a >> 24) & 0x3F;
  uint32_t cm = sin_table[64-am];
  uint32_t sm = sin_table[   am];
  
  /* Use Taylor series for low bits */
  uint32_t al = a & UINT32_C(0xFFFFFF); /* low 24-bits */
  uint32_t wl = mulhi(al<<3, pi4); /* put it into proper form */
  
  /* wl is in the form of wl*2^32
   * cos(w) = 1 - w^2/2!
   * sin(w) = w - w^3/3!
   * As long as w < 2^-7, w^4/4! < 2^-32
   */
  
  uint32_t wl2 = mulhi(wl,wl);
  uint32_t cl  = (wl2 >> 1); //  - mulhi(wl2, wl2)/30; // for more precision
  uint32_t sl  = wl - div6(mulhi(wl2, wl));
  
  /* Compose table and taylor */
  int32_t s = sm - mulhi(sm,cl) + mulhi(cm,sl);
  int32_t c = cm - mulhi(cm,cl) - mulhi(sm,sl);

  /* Extract high-bits into masks */
  int32_t a31 = (a<<0) >> 31;
  int32_t a30 = (a<<1) >> 31;
  
  /* Swap cos and sin ? */
  int32_t m = (c^s) & a30;
  s ^= m;
  c ^= m;

  /* Flip the sign of sin/cos? */
  int32_t cf = a31^a30;
  s = (s^a31); //-a31
  c = (c^cf);  //-cf
  
  *sin = s;
  *cos = c;
}

/* Function vFlushRxFifo(...) */
void vFlushRxFifo(void)
/* ==================================================================================================== */
{
  /* Flush RX FIFO */
  if (iNAU8811_CleanRxFifo(HW_FIFO_SIZE))
  {
    mprintf("main: iNAU8811_CleanRxFifo() failed!\n");
  }
  else
  {
    mprintf("main: iNAU8811_CleanRxFifo() succeeded!\n");
  }
}

/* Function main(...) */
/* ==================================================================================================== */  
int main (void)
{
  
  /* Get uart unit address */
  discoverPeriphery();

  /* Initialize uart unit */
  uart_init_hw();

  /* Initialize nau8811 driver address */
  mprintf("\n\n\n");
  
  if(iNAU8811_AutoInitialize())
  {
    mprintf("main: iNAU8811_AutoInitialize() failed!\n");
    while(true) {};
  }
  else
  {
    mprintf("main: iNAU8811_AutoInitialize() succeeded!\n");
    if(iNAU8811_ConfigureDevice())
    {
      mprintf("main: iNAU8811_ConfigureDevice() failed!\n");
      while(true) {};
    }
    else
    {
      mprintf("main: iNAU8811_ConfigureDevice() succeeded!\n");
    }
  }
  
  /* Display welcome message */
  mprintf("main: Application started ...\n");
  
#if 0
  /* Debug break */
  while(true) {};
#endif

#if SIGNAL_MODE == SIGNAL_MODE_FIXED_FREQUENCY
  vFixedFrequency();
#endif
#if SIGNAL_MODE == SIGNAL_MODE_VARIOUS_FREQUENCY
  vVariousFrequency();
#endif
  
  /* Should never get here! */
  mprintf("main: Application finished!\n");
  while(true) {};
  return (0);
  
}

/* Function vFixedFrequency(...) */
/* ==================================================================================================== */  
void vFixedFrequency(void)
{
  /* Helper */
  uint32_t uCounter = 0;
  uint32_t uPackets = MAX_PACKET_SIZE;
  uint32_t uTestPattern = PATTERN_START;  
  uint32_t uPacketIterator = 0;
  uint32_t uGetStatusRegister = 0;
  uint32_t a_uTxTestFrame[MAX_PACKET_SIZE];
  uint32_t a_uRxTestFrame[MAX_PACKET_SIZE];
  float    fp_SineX = 0.0f;
  
  /* Fill random data packet */
  for (uPacketIterator=0; uPacketIterator<uPackets; uPacketIterator++)
  {
#if GEN_WAVEFORM == GEN_SQUARE_WAVE
    if (uPacketIterator<(uPackets/2))  { a_uTxTestFrame[uPacketIterator] = GEN_MULTIPLIER; }
    else                               { a_uTxTestFrame[uPacketIterator] = -1*GEN_MULTIPLIER; }
#endif
  
#if GEN_WAVEFORM == GEN_TRIANGLE_WAVE
    if      (uPacketIterator<(uPackets/4)) { a_uTxTestFrame[uPacketIterator] = uPacketIterator*(GEN_MULTIPLIER/(uPackets/2)); }
    else if (uPacketIterator<(uPackets/2)) { a_uTxTestFrame[uPacketIterator] = a_uTxTestFrame[(uPackets/4)-1] - (uPacketIterator-(uPackets/4)-1)*(GEN_MULTIPLIER/(uPackets/2)); }
    else                                   { a_uTxTestFrame[uPacketIterator] = a_uTxTestFrame[uPacketIterator-(uPackets/2)] *-1; }
#endif

#if GEN_WAVEFORM == GEN_SINE_WAVE
    fp_SineX = GEN_MULTIPLIER*fpSine((((FLOAT_PI*2)/(SAMPLE_FREQUENCY))*(SAMPLE_FREQUENCY/MAX_PACKET_SIZE))*uPacketIterator);
    a_uTxTestFrame[uPacketIterator] = (int32_t) fp_SineX;
#endif

#if GEN_WAVEFORM == GEN_LINE
  a_uTxTestFrame[uPacketIterator] = GEN_MULTIPLIER;
#endif
    
    /* Print wave */
    mprintf("main: Data 0x%08x = %d \n", a_uTxTestFrame[uPacketIterator], a_uTxTestFrame[uPacketIterator]);
  } 
  
  /* Display signal type and frequency */
#if GEN_WAVEFORM == GEN_SQUARE_WAVE  
  mprintf("main: Generating square wave signal with f=%dHz now ...\n", (SAMPLE_FREQUENCY/MAX_PACKET_SIZE));
#endif
#if GEN_WAVEFORM == GEN_TRIANGLE_WAVE 
  mprintf("main: Generating triangle wave signal with f=%dHz now ...\n", (SAMPLE_FREQUENCY/MAX_PACKET_SIZE));
#endif
#if GEN_WAVEFORM == GEN_SINE_WAVE 
  mprintf("main: Generating sine wave signal with f=%dHz now ...\n", (SAMPLE_FREQUENCY/MAX_PACKET_SIZE));
#endif
#if GEN_WAVEFORM == GEN_LINE 
  mprintf("main: Generating line with f=%dHz now ...\n", (SAMPLE_FREQUENCY/MAX_PACKET_SIZE));
#endif
  
  /* Flush reception fifo */
  vFlushRxFifo();
  
  /* Send data */
  while(true)
  {
    /* Send data */
    if (iNAU8811_TransmitData(a_uTxTestFrame,uPackets))
    {
      mprintf("main: Transmit error!\n"); 
      while(true) {};
    }
    
    /* Wait until transmit fifo is empty */
    do
    {
      iNAU8811_GetParameter(eStatusRegister, &uGetStatusRegister);
    } while(!(uGetStatusRegister&NAU8811_REG_STATUS_IIS_TX_FIFO_EMPTY));
    
  } 
}

/* Function vVariousFrequency(...) */
/* ==================================================================================================== */ 
void vVariousFrequency(void)
{
  /* Helper */
  uint32_t uFrequency = START_FREQUENCY;
  uint32_t uSignalValue = 0;
  uint32_t uValue = 0;
  uint32_t *p_uValue;
  uint32_t uIterator = 0;
  uint32_t uDirection = 0;
  volatile uint32_t uStepCnt = 0;
  uint32_t uPhase = 0;
  int32_t  iSine = 0;
  int32_t  iCosine = 0;
    
  /* Flush reception fifo */
  vFlushRxFifo();
  
  /* Step through all frequencies */
  while(true)
  {
  
#if 1
    /* Vary frequency */
    if (uFrequency > END_FREQUENCY)
    {
      uFrequency = START_FREQUENCY;
      uDirection = 0;
    }
    else if (uFrequency < START_FREQUENCY)
    {
      uFrequency = END_FREQUENCY;
      uDirection = 1;
    }
    else
    {
      if (uStepCnt == ITERATIONS_PER_STEP)
      {
        if(uDirection==0) { uFrequency = uFrequency + STEP_FREQUENCY; }
        else              { uFrequency = uFrequency - STEP_FREQUENCY; }
        uStepCnt = 0;
      }
      else
      {
        uStepCnt++;
      }
      /* Test */
      if (uFrequency == END_FREQUENCY)
      {
        uDirection = 1;
      }
      else if (uFrequency == START_FREQUENCY)
      {
        uDirection = 0;
      }
    }
#endif

#if GEN_WAVEFORM == GEN_SINE_WAVE
    uPhase += 2*SAMPLE_FREQUENCY*uFrequency;
    cordic(uPhase, &iSine, &iCosine);
    uValue = iSine;
    /* Transmit value */
    vNAU8811_TransmitStream(uValue);
#else
    /* Generate values for frequency x */
    for (uIterator = 0; uIterator < (SAMPLE_FREQUENCY/uFrequency); uIterator++)
    {
  #if GEN_WAVEFORM == GEN_SQUARE_WAVE
      if (uIterator<(SAMPLE_FREQUENCY/uFrequency)/2) { uValue = GEN_MULTIPLIER; }
      else                                           { uValue = 0; }
  #endif
  #if GEN_WAVEFORM == GEN_TRIANGLE_WAVE
      if      (uIterator<(((SAMPLE_FREQUENCY/uFrequency)/2))) { uValue = uIterator*(GEN_MULTIPLIER/((SAMPLE_FREQUENCY/uFrequency)/2)); }
      else                                                    { uValue = GEN_MULTIPLIER-((uIterator-((SAMPLE_FREQUENCY/uFrequency)/4)*2))*(GEN_MULTIPLIER/((SAMPLE_FREQUENCY/uFrequency)/2)); }
  #endif
    /* Transmit value */
    vNAU8811_TransmitStream(uValue);
    }
#endif
  }
}
