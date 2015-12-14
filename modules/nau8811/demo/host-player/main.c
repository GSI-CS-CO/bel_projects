/* C Standard Includes */
/* ==================================================================================================== */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* GSI/WR/Devices Includes */
/* ==================================================================================================== */
#include "etherbone.h"
#include "nau8811_audio_driver.h"

/* Defines */
/* ==================================================================================================== */
#define TX_TOTAL_DEVICE_BUFFER    512                                   /* Dwords */
#define TX_BUFFER_LENGTH          TX_TOTAL_DEVICE_BUFFER/2              /* Dwords */
#define TX_NEXT_PACKET_LEVEL      TX_TOTAL_DEVICE_BUFFER/2              /* Dwords */
#define WAV_HEADER_LENGTH         44                                    /* 44 bytes header information */
#define WAV_ATTENUATION           1                                     /* >1 decrease sound; <1 increase sound; 0 forbidden */
#define TX_STREAM_REG_OFFSET      NAU8811_REG_TX_IIS_STREAM_OFFSET/4    /* Transmit stream register offset */
#define FIFO_FILL_LEVEL_OFFSET    NAU8811_REG_FIFO_FILL_LEVEL_OFFSET/4  /* Fill level register offset */

/* Global */
/* ==================================================================================================== */
const char* program;
const char* devName;

/* Prototypes */
/* ==================================================================================================== */
void vHandleEBError(const char* where, eb_status_t status);

/* Function vHandleEBError(...) */
/* ==================================================================================================== */
void vHandleEBError(const char* where, eb_status_t status)
{
  fprintf(stderr, "%s: %s failed: %s\n", program, where, eb_status(status));
  exit(1);
}

/* Function main(...) */
/* ==================================================================================================== */
int main(int argc, char** argv)
{
  
  /* Helpers */
  int gotc;
  uint8_t  uLowByte  = 0;
  uint8_t  uHighByte = 0;
  int32_t  iOpt = 0;
  uint32_t uIterator = 0;
  uint32_t a_uTxBuffer[TX_BUFFER_LENGTH];
  uint32_t uPacketCounter = 0;
  uint32_t uDwordCounter = 0;
  uint32_t uFileSize = 0;
  uint32_t uFileSizeNoHeader = 0;
  bool fReachedEndOfFile = false;
  bool fFifoLevelReached = false;
  
  /* File handling */
  int rc;
  FILE *fp;
  char *p_cFileName = NULL;

  /* Etherbone */
  eb_socket_t socket;
  eb_status_t status;
  eb_format_t s_EBFormat = EB_ADDR32|EB_DATA32;
  eb_status_t s_EBStatus;
  eb_cycle_t  s_EBCycle;
  eb_data_t   s_EBData[TX_BUFFER_LENGTH];
  
  /* Process the command-line arguments */
  program = argv[0];
  while ((iOpt = getopt(argc, argv, "f:")) != -1)
  {
    switch (iOpt)
    {
      case 'f':
      {
        p_cFileName = optarg;
        break;
      }
      case '?':
      {
        fprintf(stderr, "%s: Usage %s <protocol/host/port> -f sound.file\n", program, program);
        break;
      }
      default:
      {
        fprintf(stderr, "%s: Bad getopt result\n", program);
        return NAU8811_RETURN_FAILURE_CODE;
      }
    }
  }
  
  /* Print help depending on arguments */
  if (optind + 1 != argc)
  {
    fprintf(stderr, "Usage: %s <protocol/host/port> -f sound.file\n", program);
    return 1;
  }

  /* Open EB socket and device */
  devName = argv[optind];
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32|EB_DATA32, &socket)) != EB_OK) { vHandleEBError("eb_socket_open", status); }
  if ((status = eb_device_open(socket, devName, EB_ADDR32|EB_DATA32, 3, &s_DeviceName)) != EB_OK) { vHandleEBError("eb_device_open", status); }
  
  /* Initialize device */
  if (iNAU8811_AutoInitialize())
  {
    fprintf(stderr, "%s: Initialization failed!\n", program);
    return NAU8811_RETURN_FAILURE_CODE;
  }
  
  /* Configure device */
  if (iNAU8811_ConfigureDevice())
  {
    fprintf(stderr, "%s: Configuration failed!\n", program);
    return NAU8811_RETURN_FAILURE_CODE;
  }
  
  /* Try to open the file */
  fp = fopen(p_cFileName, "rb");
  
  /* Check file pointer */
  if (fp == NULL) 
  {
    fprintf(stderr, "%s: Failed to open file %s!\n", program, p_cFileName);
    return NAU8811_RETURN_FAILURE_CODE;
  }
  else
  {
    printf("%s: Playing file: %s\n", program, p_cFileName);
  }
  
  /* Check file size */
  fseek(fp, 0, SEEK_END); /* Seek to end of file */
  uFileSize = ftell(fp); /* Get current file pointer */
  fseek(fp, 0, SEEK_SET); /* seek back to beginning of file */
  if (uFileSize != (uint32_t)-1)
  {
    printf("%s: File size (with header): %d bytes\n", program, uFileSize);
  }
  uFileSizeNoHeader = uFileSize - WAV_HEADER_LENGTH;
  
  /* Read the file header */
  for (uIterator = 0; uIterator < WAV_HEADER_LENGTH; uIterator++)
  {
    rc = getc(fp);
    if (rc == EOF)
    {
      fprintf(stderr, "%s: An error occurred while reading the file!\n", program);
      return NAU8811_RETURN_FAILURE_CODE;
    }
  }
  devName = argv[optind];
  printf("%s: Streaming audio data now ...\n", program);
  
  /* Read from until the end is reached */
  while (!fReachedEndOfFile)
  {
    /* Read one buffer size */
    uDwordCounter = 0;
    for (uIterator = 0; uIterator < TX_BUFFER_LENGTH; uIterator++)
    {
      /* Perform two dummy reads because we only care about the data for one channel */
      uLowByte = getc(fp); 
      uHighByte = getc(fp);
      uLowByte = getc(fp); 
      uHighByte = gotc = getc(fp);
      if (gotc == EOF) {
        fReachedEndOfFile = true;
        break;
      }
      a_uTxBuffer[uIterator] = (uHighByte << 24) | (uLowByte << 16) | 0x0000;
      a_uTxBuffer[uIterator] = (int32_t)a_uTxBuffer[uIterator]/WAV_ATTENUATION; /* Decrease loudness and keep the sign */
      uPacketCounter++;
      uDwordCounter++;
      /* Check if file was played */
      if (uPacketCounter*4 >= uFileSizeNoHeader)
      {
        fReachedEndOfFile = true;
        break;
      }
    }
  
    /* Open a blocked cycle (control loop behavior) */
    if ((s_EBStatus = eb_cycle_open(s_DeviceName, 0, eb_block, &s_EBCycle)) != EB_OK)
    {
      printf("\n Error: Failed to create cycle: %s\n", eb_status(s_EBStatus));
      return(NAU8811_RETURN_FAILURE_CODE);
    }
  
    /* Build (next) packet */
    if (!fReachedEndOfFile)
    {
      /* Build full packet */
      for (uIterator = 0; uIterator < TX_BUFFER_LENGTH; uIterator++)
      {
        eb_cycle_write(s_EBCycle, (eb_address_t)(p_uNAU8811_Area+TX_STREAM_REG_OFFSET), s_EBFormat, a_uTxBuffer[uIterator]);
      }
    }
    else
    {
      /* No more bytes in audio file, send the rest */
      for (uIterator = 0; uIterator < uDwordCounter; uIterator++)
      {
        eb_cycle_write(s_EBCycle, (eb_address_t)(p_uNAU8811_Area+TX_STREAM_REG_OFFSET), s_EBFormat, a_uTxBuffer[uIterator]);
      }
    }
    
    /* Close cycle */
    if ((eb_cycle_close(s_EBCycle) != EB_OK))
    {
      printf("\n Error: Failed to close cycle: %s\n", eb_status(s_EBStatus));
      return(NAU8811_RETURN_FAILURE_CODE);
    }
    
    /* Display status */
    if (uFileSize != (uint32_t)-1)
    {
      printf("\r%s: Streamed %d bytes (%d%%) ... ", program, uPacketCounter*4, (uPacketCounter*4*100)/(uFileSize-WAV_HEADER_LENGTH) );
    }
    else
    {
      printf("\r%s: Streamed %d bytes ... ", program, uPacketCounter*4 );
    }
    
    fflush(stdout);
    
    /* Wait until FIFO needs to be refilled */
    fFifoLevelReached = false;
    while (!fFifoLevelReached)
    {
      /* Open a new cycle to read the fill level */
      if ((s_EBStatus = eb_cycle_open(s_DeviceName, 0, eb_block, &s_EBCycle)) != EB_OK)
      {
        printf("\n Error: Failed to create cycle: %s\n", eb_status(s_EBStatus));
        return(NAU8811_RETURN_FAILURE_CODE);
      }
      
      /* Read the fill level register */
      eb_cycle_read(s_EBCycle, (eb_address_t)(p_uNAU8811_Area+FIFO_FILL_LEVEL_OFFSET), s_EBFormat, &s_EBData[0]);
      
      /* Close the cycle */
      if ((eb_cycle_close(s_EBCycle) != EB_OK))
      {
        printf("\n Error: Failed to close cycle: %s\n", eb_status(s_EBStatus));
        return(NAU8811_RETURN_FAILURE_CODE);
      }
      
      /* Check if fifo needs to be refilled */
      if ( (((uint32_t)s_EBData[0])&NAU8811_REG_FIFO_FILL_LEVEL_TX_MASK) < TX_NEXT_PACKET_LEVEL)
      {
        fFifoLevelReached = true;
      }
    }
  }
  
  /* Close handler properly */
  if ((status = eb_device_close(s_DeviceName)) != EB_OK) { vHandleEBError("eb_device_close", status); }
  if ((status = eb_socket_close(socket)) != EB_OK) { vHandleEBError("eb_socket_close", status); }

  /* Done */
  printf("\n");
  return NAU8811_RETURN_SUCCESS_CODE;
  
}
