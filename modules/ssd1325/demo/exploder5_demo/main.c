/* C Standard Includes */
/* ==================================================================================================== */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* GSI LM32 Includes */
/* ==================================================================================================== */
#include "irq.h"
#include "uart.h"
#include "mini_sdb.h"
#include "aux.h"
#include "ssd1325_serial_driver.h"

/* Timer defines */
/* ==================================================================================================== */
#define DELAY               2500000/100
#define EB_REG_IP           0x00000018
#define EP_REG_MACH         0x00000024
#define EP_REG_MACL         0x00000028
#define UTC_LOW             0x00000008
#define UTC_HIGH            0x0000000C

/* Externals (avoid warnings) */
/* ==================================================================================================== */
extern int mprintf(char const *format, ...);
  
/* Prototypes */
/* ==================================================================================================== */
void vUpdateDisplay(void);
void vConfigureDisplay(void);

/* Function vConfigureDisplay(...) */
/* ==================================================================================================== */  
void vConfigureDisplay(void)
{
  
  /* Find display */
  if (iSSD1325_AutoInitialize()) { mprintf("main: iSSD1325_AutoInitialize() failed!\n"); while(true) {}; }
  else                           { mprintf("main: iSSD1325_AutoInitialize() succeeded!\n"); }
  
  /* Reset display */
  if (iSSD1325_ResetDevice()) { mprintf("main: iSSD1325_ResetDevice() failed!\n"); while(true) {}; }
  else                        { mprintf("main: iSSD1325_ResetDevice() succeeded!\n"); }
  
  /* Configure display */
  if (iSSD1325_ConfigureScreen()) { mprintf("main: iSSD1325_ConfigureScreen() failed!\n"); while(true) {}; }
  else                            { mprintf("main: iSSD1325_ConfigureScreen() succeeded!\n"); }
  
  /* Clear display */
  if (iSSD1325_ClearScreen()) { mprintf("main: iSSD1325_ClearScreen() failed!\n"); while(true) {}; }
  else                        { mprintf("main: iSSD1325_ClearScreen() succeeded!\n"); }
  
}

/* Function vUpdateDisplay(...) */
/* ==================================================================================================== */  
void vUpdateDisplay(void)
{

  /* Helper */
  static uint32_t uDemoState = 0;  
  uint32_t uDisplayCounter;
  uint32_t uDisplayIPShift;
  uint32_t uUTCLow = 0;
  uint32_t uUTCHigh = 0;
  uint32_t uOldIP = 0;
  uint32_t *p_uEtherBoneBase = (uint32_t*) 0x80060700;    //p_uEtherBoneBase = find_device(0x68202b22);
  uint32_t *p_ep_base = (uint32_t*) 0x80060100;           //p_ep_base        = find_device(0x650c2d4f);
  uint32_t *p_uPPSGen = (uint32_t*) 0x80060300;           //p_uPPSGen        = find_device(0xde0d8ced);
  static uint8_t myIP[4];
  static uint8_t myMAC[6];
  char cFormatLine[21];
  
  /* Display demo */
  switch(uDemoState)
  {
  
    /* Display IP and MAC */
    case 0:
    {
            
      /* Get MAC and IP */
      uOldIP = myIP[0]<<24 | myIP[1]<<16 | myIP[2]<<8 | myIP[3];
      myMAC[5] = (uint8_t) ((*(p_ep_base+(EP_REG_MACL>>2))&0xff));
      myMAC[4] = (uint8_t) (((*(p_ep_base+(EP_REG_MACL>>2))&0xff00))>>8);
      myMAC[3] = (uint8_t) (((*(p_ep_base+(EP_REG_MACL>>2))&0xff0000))>>16);
      myMAC[2] = (uint8_t) (((*(p_ep_base+(EP_REG_MACL>>2))&0xff000000))>>24);
      myMAC[1] = (uint8_t) ((*(p_ep_base+(EP_REG_MACH>>2))&0xff));
      myMAC[0] = (uint8_t) ((*(p_ep_base+(EP_REG_MACH>>2))&0xff00)>>8);
      myIP[3] = (uint8_t) ((*(p_uEtherBoneBase+(EB_REG_IP>>2))&0xff));
      myIP[2] = (uint8_t) (((*(p_uEtherBoneBase+(EB_REG_IP>>2))&0xff00))>>8);
      myIP[1] = (uint8_t) (((*(p_uEtherBoneBase+(EB_REG_IP>>2))&0xff0000))>>16);
      myIP[0] = (uint8_t) (((*(p_uEtherBoneBase+(EB_REG_IP>>2))&0xff000000))>>24); 
      
      /* IP */
      iSSD1325_PrintString("IP-Address:", 0, 0);
      
      /* Check if IP is valid */
      if (myIP[0]==192 && myIP[1]==168 && myIP[2]==0 && myIP[3]==100)
      {
        iSSD1325_PrintString("Running BOOTP ...", 0, 1);
      }
      /* Check if IP changed */
      else if (uOldIP != (uint32_t) ((myIP[0]<<24 | myIP[1]<<16 | myIP[2]<<8 | myIP[3])))
      {
        uDisplayIPShift = 0;
        for(uDisplayCounter=0; uDisplayCounter<4; uDisplayCounter++)
        {
          if(uDisplayCounter<3)
          {
            sprintf(cFormatLine, "%u.", myIP[uDisplayCounter] );
          }
          else
          {
            sprintf(cFormatLine, "%u", myIP[uDisplayCounter] );
          }
          iSSD1325_PrintString(cFormatLine, uDisplayIPShift, 1);
          if(myIP[uDisplayCounter]<10)       {uDisplayIPShift+=2;}
          else if(myIP[uDisplayCounter]<100) {uDisplayIPShift+=3;}
          else                               {uDisplayIPShift+=4;}
          iSSD1325_PrintString("      ", uDisplayIPShift, 1);
        }
      }
      
      /* MAC */
      iSSD1325_PrintString("MAC-Address:", 0, 2);
      for(uDisplayCounter=0; uDisplayCounter<6; uDisplayCounter++)
      {
        if(uDisplayCounter<5)
        {
          sprintf(cFormatLine, "%02x:", myMAC[uDisplayCounter] );
        }
        else
        {
          sprintf(cFormatLine, "%02x", myMAC[uDisplayCounter] );
        } 
        iSSD1325_PrintString(cFormatLine, uDisplayCounter*3, 3);
      }
      
      /* Done */
      uDemoState = 1;
      break;
    }
    
    
    /* Display White Rabbit state */
    case 1:
    {
    
      /* Get the current time */
      uUTCLow = (*(p_uPPSGen+1))*8; /* Nanoseconds (1ns) since 1970 (needs to be multiplied by 8) */
      uUTCHigh = *(p_uPPSGen+2);    /* Seconds since 1970 */
      
      /* Format and print the raw time */
      iSSD1325_PrintString("---------------------", 0, 4);
      iSSD1325_PrintString("White Rabbit Time:", 0, 5);
      sprintf(cFormatLine, "0x%08x%08x", (unsigned int)uUTCHigh, (unsigned int)uUTCLow);
      iSSD1325_PrintString(cFormatLine, 0, 6);
      
      /* Calculate HH:MM:SS */
      uint32_t uSeconds = uUTCHigh;
      uint32_t uDispHours = ((uSeconds/(3600))%24);
      uint32_t uDispRemainder = uSeconds%3600;
      uint32_t uDispMinutes = uDispRemainder/60;
      uint32_t uDispSeconds = uDispRemainder%60;
      
      /* Calculate MS:US:NS */
      uint32_t uNanoSeconds = uUTCLow;
      uint32_t uDispNanoSeconds = uNanoSeconds%1000;
      uint32_t uDispMicroSeconds = ((uNanoSeconds-uDispNanoSeconds)%1000000)/1000;
      uint32_t uDispMilliSeconds = ((uNanoSeconds-uDispMicroSeconds-uDispNanoSeconds)%1000000000)/1000000;
      
      /* Format and print the time string */
      sprintf(cFormatLine, "%02d:%02d:%02d:%03d:%03d:%03d", (int)uDispHours, (int)uDispMinutes, (int)uDispSeconds, (int)uDispMilliSeconds, (int)uDispMicroSeconds, (int)uDispNanoSeconds);
      iSSD1325_PrintString(cFormatLine, 0, 7);

      /* Done */
      uDemoState = 0;
      break;
    
    }
    
  }
}

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
  
  /* Initialize ssd1325 driver address */
  vConfigureDisplay();
  
  /* Display welcome message */
  mprintf("main: Application started ...\n");	
  
  /* Wait fo interrupt */
  mprintf("main: Updating display now ...\n");
  uCounter = 0;
  while(true)
  {
    asm volatile ("nop");
    uCounter++;
    if (uCounter>DELAY)
    {
      vUpdateDisplay();
      uCounter = 0;
    }  
  }
  
  /* Should never get here! */
  mprintf("main: Application finished!\n");
  while(true);
  return (0);
  
}
