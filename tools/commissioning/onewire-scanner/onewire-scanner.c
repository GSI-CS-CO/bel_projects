#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <etherbone.h>
#include <w1.h>

#define W1_VENDOR              0xce42     /* CERN */
#define W1_DEVICE              0x779c5443 /* WR-Periph-1Wire */
#define ONEWIRE_TEMPERATURE_ID 0x28
#define ONEWIRE_EEPROM_ID      0x43

char *prgname;
int total_slaves = 0;
eb_address_t BASE_ONEWIRE;
eb_device_t device;
extern struct w1_bus wrpc_w1_bus;

static void die(const char *reason, eb_status_t status)
{
  fprintf(stderr, "%s: %s: %s\n", prgname, reason, eb_status(status));
  exit(1);
}

int scan(int slave_id)
{
  /* Helper */
  struct w1_dev *d;
  int i;

  /* Initialize and scan bus */
  wrpc_w1_init();
  w1_scan_bus(&wrpc_w1_bus);
    
  for (i = 0; i < W1_MAX_DEVICES; i++)
  {
    d = wrpc_w1_bus.devs + i;
    if (d->rom)
    {
      fprintf(stderr, "--  ---                 %02d    0x%08x%08x  ", i, (int)(d->rom >> 32), (int)d->rom);
      total_slaves++;
      if ((int)d->rom&ONEWIRE_TEMPERATURE_ID)
      {
        printf("DS18B20 - Digital Thermometer\n");
      }
      else if ((int)d->rom&ONEWIRE_EEPROM_ID)
      {
        printf("DS28EC20 - 20Kb EEPROM\n");
      }
      else
      {
        printf("Unknown\n");
      }
    }
  }
  
  /* Done */
  return 0;
}

int main(int argc, char **argv)
{
  /* Helper */
  int c;
  int onewire_iterator = 0;
  eb_status_t status;
  eb_socket_t socket;
  struct sdb_device sdb[10];;
  prgname = argv[0];
  
  /* Open socket */
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_DATAX|EB_ADDRX, &socket)) != EB_OK) { die("eb_socket_open", status); }
  
  /* Open device */
  if ((status = eb_device_open(socket, argv[optind], EB_DATAX|EB_ADDRX, 3, &device)) != EB_OK) { die(argv[optind], status); }
  
  /* Find the onewire slave */
  c = sizeof(sdb) / sizeof(struct sdb_device);
  if ((status = eb_sdb_find_by_identity(device, W1_VENDOR, W1_DEVICE, &sdb[0], &c)) != EB_OK) { die("eb_sdb_find_by_identity", status); }
  
  /* Check if we found at least one controller */
  printf("Scanning for OneWire controller(s) on %s now...\n\n", argv[optind]);
  if (c >= 1)
  {
    /* Show controllers(s) */
    printf("ID  Wishbone Address    OWID  Serial Code         Type\n");
    printf("-------------------------------------------------------------------------------\n");
    /* Print controller(s) and check for slaves */
    for (onewire_iterator = 0; onewire_iterator < c; onewire_iterator++)
    {
      printf("%02d  0x%016lx  --    ---                 ---\n", onewire_iterator, sdb[onewire_iterator].sdb_component.addr_first);
      BASE_ONEWIRE = sdb[onewire_iterator].sdb_component.addr_first;
      scan(onewire_iterator);
    }
    printf("\nFound %d OneWire controller(s) on %s.", c, argv[optind]);
    printf("\nFound %d OneWire device(s)/slave(s) on %s.\n", total_slaves, argv[optind]);
  }
  else
  {
    printf("Could not find any OneWire controller(s) on %s!\n", argv[optind]);
  }
  
  /* Done */
  return 0;
  
}
