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
#define TX_SPI_DATA_REG      NAU8811_REG_TX_SPI_DATA_OFFSET/4
#define TX_IIS_CONFIG_REG    NAU8811_REG_CONTROL_OFFSET/4
#define MAX_LOUDNESS         0x3f
#define MAX_LOUDNESS_PERCENT 100

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
  uint32_t uTxBuffer = 0;
  uint32_t uLoudness = 0;
  int32_t  iOpt = 0;

  /* Etherbone */
  eb_socket_t socket;
  eb_status_t status;
  eb_format_t s_EBFormat = EB_ADDR32|EB_DATA32;
  eb_status_t s_EBStatus;
  eb_cycle_t  s_EBCycle;
  
  /* Process the command-line arguments */
  program = argv[0];
  while ((iOpt = getopt(argc, argv, "l:")) != -1)
  {
    switch (iOpt)
    {
      case 'l':
      {
        uLoudness = atoi(optarg);
        if (uLoudness>MAX_LOUDNESS_PERCENT)
        {
          fprintf(stderr, "%s: Loudness more than %d%% is not possible!\n", program, MAX_LOUDNESS_PERCENT);
          return NAU8811_RETURN_FAILURE_CODE;
        }
        break;
      }
      case '?':
      {
        fprintf(stderr, "%s: Usage %s <protocol/host/port> -l loudness 0..%d\n", program, program, MAX_LOUDNESS_PERCENT);
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
    fprintf(stderr, "Usage: %s <protocol/host/port> -l loudness 0..100\n", program);
    return NAU8811_RETURN_FAILURE_CODE;
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
  
  /* Write new speaker value to the device */
  /* Open cycle */
  if ((s_EBStatus = eb_cycle_open(s_DeviceName, 0, eb_block, &s_EBCycle)) != EB_OK)
  {
    printf("\n Error: Failed to create cycle: %s\n", eb_status(s_EBStatus));
    return(NAU8811_RETURN_FAILURE_CODE);
  }
  
  /* Setup loudness */
  if (uLoudness==MAX_LOUDNESS_PERCENT) { uTxBuffer = (NAU8811_REG_SPKOUT_VOLUME<<NAU8811_ADDRESS_SHIFT) | MAX_LOUDNESS; }
  else if (uLoudness==0)               { uTxBuffer = (NAU8811_REG_SPKOUT_VOLUME<<NAU8811_ADDRESS_SHIFT) | 0; }
  else                                 { uTxBuffer = (NAU8811_REG_SPKOUT_VOLUME<<NAU8811_ADDRESS_SHIFT) | (MAX_LOUDNESS*uLoudness)/100; }
  eb_cycle_write(s_EBCycle, (eb_address_t)(p_uNAU8811_Area+TX_SPI_DATA_REG), s_EBFormat, uTxBuffer);

  /* Close cycle */
  if ((eb_cycle_close(s_EBCycle) != EB_OK))
  {
    printf("\n Error: Failed to close cycle: %s\n", eb_status(s_EBStatus));
    return(NAU8811_RETURN_FAILURE_CODE);
  }
  
  /* Enable the trigger (heartbeat) mode */
  /* Open cycle */
  if ((s_EBStatus = eb_cycle_open(s_DeviceName, 0, eb_block, &s_EBCycle)) != EB_OK)
  {
    printf("\n Error: Failed to create cycle: %s\n", eb_status(s_EBStatus));
    return(NAU8811_RETURN_FAILURE_CODE);
  }
  
  /* Enable heartbeat mode and clock */
  if (uLoudness>0) { uTxBuffer = NAU8811_REG_CONTROL_HEARTBEAT_MODE | NAU8811_REG_CONTROL_CLOCK_ENABLE; }
  else             { uTxBuffer = 0; } /* Turn off sound */
  eb_cycle_write(s_EBCycle, (eb_address_t)(p_uNAU8811_Area+TX_IIS_CONFIG_REG), s_EBFormat, uTxBuffer);

  /* Close cycle */
  if ((eb_cycle_close(s_EBCycle) != EB_OK))
  {
    printf("\n Error: Failed to close cycle: %s\n", eb_status(s_EBStatus));
    return(NAU8811_RETURN_FAILURE_CODE);
  }
  
  /* Close handler properly */
  if ((status = eb_device_close(s_DeviceName)) != EB_OK) { vHandleEBError("eb_device_close", status); }
  if ((status = eb_socket_close(socket)) != EB_OK) { vHandleEBError("eb_socket_close", status); }
  
  /* Done */
  printf("\n");
  printf("%s: Set loudness to %d%% ...\n", program, uLoudness);
  return NAU8811_RETURN_SUCCESS_CODE;
  
}
