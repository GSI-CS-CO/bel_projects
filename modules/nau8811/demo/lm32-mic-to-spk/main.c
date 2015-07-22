/* C Standard Includes */
/* ==================================================================================================== */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* GSI LM32 Includes */
/* ==================================================================================================== */
#include "irq.h"
#include "uart.h"
#include "mini_sdb.h"
#include "aux.h"
#include "nau8811_audio_driver.h"

/* Defines */
/* ==================================================================================================== */
#define HW_FIFO_SIZE 256 /* HW_FIFO_SIZE * 32Bits */

/* Externals (avoid warnings) */
/* ==================================================================================================== */
extern int mprintf(char const *format, ...);

/* Prototypes */
/* ==================================================================================================== */
void vFlushRxFifo(void);

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
  
  /* Helper */
  uint32_t uData = 0;
  
  /* Get uart unit address */
  discoverPeriphery();

  /* Initialize uart unit */
  uart_init_hw();

  /* Initialize nau8811 driver address */
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
  
  /* Flush reception fifo */
  vFlushRxFifo();
  
  /* Start transmission by writing data into the transmit fifo */
  vNAU8811_TransmitStream (uData);
  
  /* Loop back audio data */
  while(true)
  {
    vNAU8811_TransmitStream (uData);
    vNAU8811_ReceiveStream (&uData);
  }
  
  /* Should never get here */
  return 0;
  
}
