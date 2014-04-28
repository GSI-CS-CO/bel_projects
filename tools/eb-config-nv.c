#include <stdio.h>
#include <stdlib.h>
#include <etherbone.h>

#define GSI_ID   0x651
#define FLASH_ID 0x5cf12a1c

void die(const char *where, eb_status_t status) {
  fprintf(stderr, "%s: %s\n", where, eb_status(status));
  exit(1);
}

int main(int argc, const char **argv) {
  eb_socket_t socket;
  eb_device_t device;
  eb_cycle_t cycle;
  eb_address_t magic;
  eb_format_t format;
  eb_status_t status;
  struct sdb_device dev;
  int cycles, bytes, count;
  
  if (argc != 4) {
    fprintf(stderr, "syntax: <device> <dummy-cycles> <address-bytes>\n");
    return 1;
  }

  cycles = atol(argv[2]);
  bytes = atol(argv[3]);
  
  if (cycles < 1 || cycles > 14) {
    fprintf(stderr, "be reasonable. cycles should probably by 4, 10, or 12. check the data-sheet.\n");
    return 1;
  }
  
  if (bytes < 3 || bytes > 4) {
    fprintf(stderr, "too much crack? spi only supports 3 or 4 byte addresses\n");
    return 1;
  }
  
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_DATAX|EB_ADDRX, &socket)) != EB_OK)
    die("eb_socket_open", status);
    
  if ((status = eb_device_open(socket, argv[1], EB_ADDRX|EB_DATAX, 3, &device)) != EB_OK)
    die(argv[1], status);
  
  count = 1;
  if ((status = eb_sdb_find_by_identity(device, GSI_ID, FLASH_ID, &dev, &count)) != EB_OK)
    die("eb_sdb_find_by_identity", status);
  
  if (count != 1)
    die("no flash chip", EB_FAIL);
  
  magic = dev.sdb_component.addr_last-3;
  
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK)
    die("eb_cycle_open", status);
  
  format = EB_DATA32 | EB_BIG_ENDIAN;
  eb_cycle_write(cycle, magic, format, 0x6);        /* write enable */
  eb_cycle_write(cycle, magic, format, 0x80000000); /* execute */
  eb_cycle_write(cycle, magic, format, 0xb1);       /* write non-volatile configuration */
  eb_cycle_write(cycle, magic, format, 0xfe | (bytes==3));
  eb_cycle_write(cycle, magic, format, (cycles << 4) | 0xf);
  eb_cycle_write(cycle, magic, format, 0x80000000); /* execute */
  
  if ((status = eb_cycle_close(cycle)) != EB_OK)
    die("eb_cycle_close", status);

  if ((status = eb_device_close(device)) != EB_OK)
    die("eb_device_close", status);

  if ((status = eb_socket_close(socket)) != EB_OK)
    die("eb_socket_close", status);

  return 0;
}
