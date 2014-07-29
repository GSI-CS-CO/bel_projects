/* Synopsis */
/* ==================================================================================================== */
/* Example for OneWire unit (read temperature sensor and write/read EEPROM
 * Tested on pexaria5 
 */

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
#include "w1.h"
#include "onewire.h"

/* Defines */
/* ==================================================================================================== */
#define EEPROM_PAGES           80
#define EEPROM_BITS_PER_PAGE   256
#define EEPROM_BYTES_PER_PAGE  EEPROM_BITS_PER_PAGE/8
#define ONEWIRE_TEMPERATURE_ID 0x24000003b16f6c28
#define ONEWIRE_EEPROM_ID      0xf70000005627fc43
#define TIMEOUT                2500000
#define FUNCTION_TEST          0

/* External functions */
/* ==================================================================================================== */
extern int mprintf(char const *format, ...);

/* Prototypes */
/* ==================================================================================================== */
int usleep(useconds_t usec);

/* Function usleep(...) - replacement for original function */
/* ==================================================================================================== */
int usleep(useconds_t usec)
{
  /* TBD: Ensure right sleep time (using timer?) */
#if FUNCTION_TEST==0  
  volatile uint32_t uCounter = 0;
  while (uCounter < usec*1000) { uCounter++; }
  return 0;
#endif  
}

/* Function main(...) */
/* ==================================================================================================== */  
int main (void)
{
  
  /* Helper */

  uint32_t uCounter = 0;            /* Counter for cyclic actions */
  uint32_t uEEPROMIterator = 0;     /* Iterator for EEPROM bytes */
  uint32_t uEEPROMPageIterator = 0; /* Iterator for EEPROM pages */
  int32_t iDeviceIterator = 0;      /* Iterator for each one wire device */
  int32_t iTemp = 0;                /* Temperature measurement */
  int32_t iDevicesFound = 0;        /* 1Wire devices */
  
  uint8_t uReadEEPROMData[40];      /* Read data from EEPROM */
  uint8_t uWriteEEPROMData[40];     /* Write data to EEPROM */
  
  struct w1_dev *s_W1Dev;           /* One wire structure */
  struct w1_bus s_w1_bus;           /* One wire bus structure */
 
  /* Get uart unit address */
  discoverPeriphery();

  /* Initialize uart unit */
  uart_init_hw();
  
  /* TBD: Find the right one wire instance automatically */
#if FUNCTION_TEST==0  
  pOneWire = (unsigned int*) 0x80000300;
#endif
  
  /* Check if one wire was found */
  if(pOneWire==NULL)
  {
    mprintf("main: One wire interface was not found!\n");
    return -1;
  }
  else
  {
    mprintf("main: Found one wire interface at: 0x%08x\n", pOneWire);
    
    /* Initialize w1 */
    wrpc_w1_init(); /* Will use pOneWire */
    s_w1_bus.detail = ONEWIRE_PORT;
  }
  
  /* Display welcome message */
  mprintf("main: Application started!\n");
  
  /* Wait for timeout */
  uCounter = 0;
  
  /* Start super loop */
  while(true)
  {
    asm volatile ("nop");
    uCounter++;
    if (uCounter>TIMEOUT)
    {
      /* Reset */
      uCounter = 0;
      
      /* Scan bus */
      mprintf("main: Scanning bus now ... \n");  
      iDevicesFound = w1_scan_bus(&s_w1_bus);
      mprintf("main: Devices found %d \n", iDevicesFound);  
      
      /* Iterate devices */
      for (iDeviceIterator = 0; iDeviceIterator < W1_MAX_DEVICES; iDeviceIterator++)
      {
        s_W1Dev = s_w1_bus.devs + iDeviceIterator;
     
        if (s_W1Dev->rom)
        {
          /* Show found device */
          mprintf("main: Device found [%d]:   0x%08x%08x\n", iDeviceIterator, (int)(s_W1Dev->rom >> 32), (int)s_W1Dev->rom);
          
          /* Special treatment depending on ID */
          switch (s_W1Dev->rom)
          {
    
            /* Temperature sensor */
            case ONEWIRE_TEMPERATURE_ID:
            {
              /* Get temperature */
              iTemp = w1_read_temp(s_W1Dev, 0);
              /* Show temperature in human readable format */
              mprintf("main: Temperature [%d]:    %d.%04d\n", iDeviceIterator, iTemp >> 16,(int)((iTemp & 0xffff) * 10 * 1000 >> 16));
              break;
            }
            
            /* 20Kb EEPROM */
            case ONEWIRE_EEPROM_ID:
            {
            
              /* Write 1 page */
              for (uEEPROMPageIterator=0; uEEPROMPageIterator<EEPROM_BYTES_PER_PAGE; uEEPROMPageIterator++)
              {
                uWriteEEPROMData[uEEPROMPageIterator] = (uint8_t) uEEPROMPageIterator;
              }
              for (uEEPROMPageIterator=0; uEEPROMPageIterator<1; uEEPROMPageIterator++)
              {
                w1_write_eeprom(s_W1Dev, 0, uWriteEEPROMData, EEPROM_BYTES_PER_PAGE);
              }
              
              /* Read 2 pages */
              for (uEEPROMPageIterator=0; uEEPROMPageIterator<2; uEEPROMPageIterator++)
              {
                /* Show index */
                mprintf("main: EEPROM content [%d]: Page[%d]\n", iDeviceIterator, uEEPROMPageIterator);
                mprintf("-------------------------------------------------", iDeviceIterator, uEEPROMPageIterator);

                /* Read raw data from EEPROM */
                w1_read_eeprom(s_W1Dev, EEPROM_BYTES_PER_PAGE*uEEPROMPageIterator, uReadEEPROMData, EEPROM_BYTES_PER_PAGE);
              
                /* Print each byte */
                for (uEEPROMIterator=0; uEEPROMIterator<EEPROM_BYTES_PER_PAGE; uEEPROMIterator++)
                {
                  /* New line every 10 prints */
                  if(uEEPROMIterator%10==0) { mprintf("\n"); } 
                  mprintf("0x%02x ", uReadEEPROMData[uEEPROMIterator]);
                }
                mprintf("\n");
              }
              mprintf("\n");
              break;
            }
            
            /* Unknown device */
            default:
            {
              mprintf("main: Unknown device!\n");
              mprintf("main: Try to change the \"ONEWIRE_...\" define!\n");
              break;
            }
            
          } /* switch(...) */
        } /* if (s_W1Dev->rom) */
      } /* for (iDeviceIterator = 0; ... ) */
    } /* if (uCounter>TIMEOUT) */
  } /* while(true) */
  
  /* Should never get here! */
  mprintf("main: Application finished!\n");	
  return (0);
 
}
