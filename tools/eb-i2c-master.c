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
#include "../modules/i2c_wrapper/src/c/oc_i2c_master.h"

/* Defines */
/* ==================================================================================================== */
#define I2C_WRAP_GSI_ID 0x651
#define I2C_WRAP_ID     0x12575a95

/* Globals Variables */
/* ==================================================================================================== */
const char *program;
const char *device_name;

/* Main */
/* ==================================================================================================== */
int main (int argc, char** argv)
{
  /* Helpers */
  int opt;
  struct sdb_device sdb;
  eb_status_t status;
  eb_socket_t socket;
  eb_device_t device;
  eb_data_t data;
  int verbose = 0;
  int error = 0;
  int devices = 1;
  int base = 0;
  int current_base = 0;

  /* Check argument counter */
  if (argc < 2)
  {
    printf("%s: Missing arguments (got only %i)\n", program, argc);
    return 1;
  }
  program = argv[0];
  device_name = argv[1];

  /* Process the command-line arguments */
  while ((opt = getopt(argc, argv, "hv")) != -1)
  {
    switch (opt)
    {
      case 'h':
        printf("%s: Simple tool to control each I2C master\n", program);
        return 0;
      case 'v':
        verbose = 1;
        break;
      default:
        printf("%s: Bad getopt result\n", program);
        error = 1;
    }
  }

  /* Exit on error(s) */
  if (error) { return 1; }

  /* Find device */
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_DATAX|EB_ADDRX, &socket)) != EB_OK)
  {
    printf("%s: Failed to open a socket!\n", program);
    return 1;
  }

  if ((status = eb_device_open(socket, device_name, EB_ADDRX|EB_DATAX, 3, &device)) != EB_OK)
  {
    printf("%s: Failed to open device %s\n", program, device_name);
    return 1;
  }

  if ((status = eb_sdb_find_by_identity(device, I2C_WRAP_GSI_ID, I2C_WRAP_ID, &sdb, &devices)) != EB_OK)
  {
    printf("%s: Failed find I2C master\n", program);
    return 1;
  }
  base = (uint32_t) sdb.sdb_component.addr_first;

  if (verbose)
  {
    printf("%s: Found I2C master at 0x%x (start)\n", program, (uint32_t) sdb.sdb_component.addr_first);
    printf("%s: Found I2C master at 0x%x (end)\n", program, (uint32_t) sdb.sdb_component.addr_last);
  }

  /* Enable core */
  if (verbose)
  {
    current_base = base+(OC_I2C_CTR<<2);
    eb_device_read(device, (eb_address_t)(current_base), EB_DATA32, &data, 0, NULL);
    printf("%s: Core status: 0x%x (0x%x)\n", program, (uint32_t) data, current_base);
  }

  /* Done */
  return 0;
}
