/* C Standard Includes */
/* ==================================================================================================== */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* GSI LM32 Includes */
/* ==================================================================================================== */
#include "uart.h"
#include "timer.h"
#include "mini_sdb.h"
#include "aux.h"

/* Defines */
/* ==================================================================================================== */
#define TIMEOUT 250000

/* External functions */
/* ==================================================================================================== */
extern int mprintf(char const *format, ...);

/* Function main(...) */
/* ==================================================================================================== */  
int main (void)
{
  
  /* Helper */
  uint32_t uCounter = 0;
  
  /* Get uart unit address */
  discoverPeriphery();

  /* Initialize uart unit */
  uart_init_hw();
  
  /* Display welcome message */
  mprintf("main: Hello Word!\n");	
  
  /* Wait for timeout */
  uCounter = 0;
  
  while(true)
  {
    asm volatile ("nop");
    uCounter++;
    if (uCounter>TIMEOUT)
    {
      mprintf("main: Idle loop ...\n");	
      uCounter = 0;
    }
  }
  
  /* Should never get here! */
  mprintf("main: Application finished!\n");	
  return (0);
 
}
