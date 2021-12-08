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

/* Function print_help */
/* ==================================================================================================== */
void print_help(void)
{
  printf("%s is a simple tool to control each WB I2C master.\n", program);
}

/* Function main */
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
  program = argv[0];
  if (argc < 2)
  {
    printf("Error: Missing arguments (got only %i)!\n", argc);
    return 1;
  }
  device_name = argv[1];

  /* Process the command-line arguments */
  while ((opt = getopt(argc, argv, "hv")) != -1)
  {
    switch (opt)
    {
      case 'h':
        print_help();
        return 0;
      case 'v':
        verbose = 1;
        break;
      default:
        printf("Error: Ambiguous arguments!\n");
        error = 1;
    }
  }

  /* Exit on error(s) */
  if (error) { return 1; }

  /* Find device */
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_DATAX|EB_ADDRX, &socket)) != EB_OK)
  {
    printf("Error: Failed to open a socket!\n");
    return 1;
  }

  if ((status = eb_device_open(socket, device_name, EB_ADDRX|EB_DATAX, 3, &device)) != EB_OK)
  {
    printf("Error: Failed to open device (%s)!\n", device_name);
    return 1;
  }

  if ((status = eb_sdb_find_by_identity(device, I2C_WRAP_GSI_ID, I2C_WRAP_ID, &sdb, &devices)) != EB_OK)
  {
    printf("Error: Failed find I2C master!\n");
    return 1;
  }
  base = (uint32_t) sdb.sdb_component.addr_first;

  if (verbose)
  {
    printf("Found I2C master at 0x%x-0x%x\n", (uint32_t) sdb.sdb_component.addr_first,
                                              (uint32_t) sdb.sdb_component.addr_last);
  }

  /* Enable core */
  if (verbose)
  {
    current_base = base+(OC_I2C_CTR<<2);
    eb_device_read(device, (eb_address_t)(current_base), EB_DATA32, &data, 0, NULL);
    printf("Core status: 0x%x (0x%x)\n", (uint32_t) data, current_base);
  }

  /* Done */
  return 0;
}
