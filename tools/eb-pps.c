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

/* Defines */
/* ==================================================================================================== */
#define WB_IO_CONTROL_PRODUCT_ID               0x10c05791
#define WB_IO_CONTROL_VENDOR_ID                0x00000651
#define WB_IO_CONTROL_GPIO_Oe_Set_low          0x00000200
#define WB_IO_CONTROL_GPIO_Oe_Set_high         0x00000204
#define WB_IO_CONTROL_GPIO_Oe_Reset_low        0x00000208
#define WB_IO_CONTROL_GPIO_Oe_Reset_high       0x0000020c
#define WB_IO_CONTROL_LVDS_Oe_Set_low          0x00000300
#define WB_IO_CONTROL_LVDS_Oe_Set_high         0x00000304
#define WB_IO_CONTROL_LVDS_Oe_Reset_low        0x00000308
#define WB_IO_CONTROL_LVDS_Oe_Reset_high       0x0000030c
#define WB_IO_CONTROL_GPIO_Term_Set_low        0x00000400
#define WB_IO_CONTROL_GPIO_Term_Set_high       0x00000404
#define WB_IO_CONTROL_GPIO_Term_Reset_low      0x00000408
#define WB_IO_CONTROL_GPIO_Term_Reset_high     0x0000040c
#define WB_IO_CONTROL_LVDS_Term_Set_low        0x00000500
#define WB_IO_CONTROL_LVDS_Term_Set_high       0x00000504
#define WB_IO_CONTROL_LVDS_Term_Reset_low      0x00000508
#define WB_IO_CONTROL_LVDS_Term_Reset_high     0x0000050c
#define WB_IO_CONTROL_GPIO_PPS_Mux_Set_low     0x00000e00
#define WB_IO_CONTROL_GPIO_PPS_Mux_Set_high    0x00000e04
#define WB_IO_CONTROL_GPIO_PPS_Mux_Reset_low   0x00000e08
#define WB_IO_CONTROL_GPIO_PPS_Mux_Reset_high  0x00000e0c
#define WB_IO_CONTROL_LVDS_PPS_Mux_Set_low     0x00000f00
#define WB_IO_CONTROL_LVDS_PPS_Mux_Set_high    0x00000f04
#define WB_IO_CONTROL_LVDS_PPS_Mux_Reset_low   0x00000f08
#define WB_IO_CONTROL_LVDS_PPS_Mux_Reset_high  0x00000f0c
#define WB_IO_CONTROL_MASK_ALL                 0xffffffff
#define WB_IO_CONTROL_MAX_DEVICES              1

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
  int opt = 0;
  int dev_count = 0;
  bool setup_io = false;
  bool verbose = false;
  bool show_help = false;
  bool turn_off = false;
  
  /* Etherbone */
  static eb_device_t device;
  eb_socket_t socket;
  eb_status_t status;
  struct sdb_device devices[WB_IO_CONTROL_MAX_DEVICES];
  uint32_t wb_addr_base;
  
  /* Get the application name */
  program = argv[0]; 
  
  /* Parse for options */
  while ((opt = getopt(argc, argv, ":svho")) != -1)
  {
    switch (opt)
    {
      case 's': { setup_io = true; break; }
      case 'v': { verbose = true; break; }
      case 'h': { show_help = true; break; }
      case 'o': { turn_off = true; break; }
      default:  { printf("Unknown argument...\n"); show_help = true; break; }
    }
  }
  
  /* Get basic arguments, we need at least the device name */
  if (optind + 1 == argc)
  { 
    devName = argv[optind];
  }
  else 
  { 
    show_help = true;
    printf("Incorrect non-optional arguments...\n");
  }
  
  /* Show help? */
  if (show_help) { return -1; }
  
  /* Open socket and device */
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_DATAX|EB_ADDRX, &socket)) != EB_OK) { vHandleEBError("eb_socket_open", status); }
  if ((status = eb_device_open(socket, devName, EB_DATAX|EB_ADDRX, 3, &device)) != EB_OK) { vHandleEBError(devName, status); }
  
  /* Try to find the IO CONTROL module */
  dev_count = WB_IO_CONTROL_MAX_DEVICES;
  if ((status = eb_sdb_find_by_identity(device, WB_IO_CONTROL_VENDOR_ID, WB_IO_CONTROL_PRODUCT_ID, &devices[0], &dev_count)) != EB_OK)
  {
    vHandleEBError("eb_sdb_find_by_identity", status); 
  }
  
  /* Did we find at least one? */
  if (dev_count != WB_IO_CONTROL_MAX_DEVICES) 
  {
    printf("IO CONTROL unit missing!\n");
    return -1;
  }
  else
  {
    wb_addr_base = (eb_address_t)(devices[0].sdb_component.addr_first);
    if (verbose) { printf("Found IO CONTROL unit at 0x%08x (%d)\n", wb_addr_base, dev_count); }
  }
  
  /* Configure unit */
  if (!turn_off)
  {
    eb_device_write(device, wb_addr_base + WB_IO_CONTROL_GPIO_PPS_Mux_Set_low, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
    eb_device_write(device, wb_addr_base + WB_IO_CONTROL_GPIO_PPS_Mux_Set_high, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
    eb_device_write(device, wb_addr_base + WB_IO_CONTROL_LVDS_PPS_Mux_Set_low, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
    eb_device_write(device, wb_addr_base + WB_IO_CONTROL_LVDS_PPS_Mux_Set_high, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
    if (setup_io)
    {
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_GPIO_Oe_Set_low, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_GPIO_Oe_Set_high, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_LVDS_Oe_Set_low, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_LVDS_Oe_Set_high, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_GPIO_Term_Reset_low, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_GPIO_Term_Reset_high, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_LVDS_Term_Reset_low, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_LVDS_Term_Reset_high, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
    }
  }
  else
  {
    eb_device_write(device, wb_addr_base + WB_IO_CONTROL_GPIO_PPS_Mux_Reset_low, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
    eb_device_write(device, wb_addr_base + WB_IO_CONTROL_GPIO_PPS_Mux_Reset_high, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
    eb_device_write(device, wb_addr_base + WB_IO_CONTROL_LVDS_PPS_Mux_Reset_low, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
    eb_device_write(device, wb_addr_base + WB_IO_CONTROL_LVDS_PPS_Mux_Reset_high, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
    if (setup_io)
    {
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_GPIO_Oe_Reset_low, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_GPIO_Oe_Reset_high, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_LVDS_Oe_Reset_low, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_LVDS_Oe_Reset_high, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_GPIO_Term_Set_low, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_GPIO_Term_Set_high, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_LVDS_Term_Set_low, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
      eb_device_write(device, wb_addr_base + WB_IO_CONTROL_LVDS_Term_Set_high, EB_DATA32, WB_IO_CONTROL_MASK_ALL, 0, NULL);
    }
  }
  
  /* Done */
  return 0;
}
