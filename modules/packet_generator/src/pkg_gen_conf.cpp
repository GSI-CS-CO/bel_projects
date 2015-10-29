#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
//PRIX32: these macros are defined for C program. They are defined for C++ only when  __STDC_FORMAT_MACROS is defined before <inttypes.h> is included.
#define __STDC_CONSTANT_MACROS
//UINT32_C:  these macros are defined for C program. They are defined for C++ only when  __STDC_CONSTRANT_MACROS is defined before <stdint.h> is included.
#include <cassert>
#include <cstdio>
#include <etherbone.h>
#include <string.h>
#include <cstdlib>
#include <unistd.h> // sleep
#include <vector>
#include <iostream>
#include <math.h>

#define GSI_ID          0x651
#define PKG_GEN_ID      0x53bee0e2

#define s_rate_max 0x14
#define s_load_max 0x4

unsigned long int bandwidth;
eb_data_t payload_length;
eb_data_t rate;
eb_data_t r_rate_max;
int pkg_length;
char * value_end;
eb_address_t base;
static eb_address_t rate_addr, length_addr;

int main(int argc, const char** argv) {
  eb_socket_t socket;
  eb_device_t device;
  eb_status_t status;
  char port[16];
  int c;
  struct sdb_device sdb;

  /* Get the port from the command line */
  strcpy(port, argv[1]);

  if (argc != 4) {
    fprintf(stderr, "%s: expecting argument <device>\n", argv[0]);
    return 1;
  }
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_DATAX|EB_ADDRX, &socket)) != EB_OK) {
    fprintf(stderr,"socket open failed.\n" );
    return 1;
  }


  if ((status = eb_device_open(socket, port, EB_ADDRX|EB_DATAX, 3, &device)) != EB_OK) {
    fprintf(stderr,"device open failed.\n" );
    return 1;
  }

  c = 1;
  if ((status = eb_sdb_find_by_identity(device, GSI_ID, PKG_GEN_ID, &sdb, &c)) != EB_OK) {
    fprintf(stderr,"sdb find failed.\n" );
    return 1;
  }
  base = sdb.sdb_component.addr_first;

  /*Find the FEC packet generator*/
  rate_addr = base + s_rate_max;
  length_addr = base + s_load_max;

  bandwidth = strtoull(argv[2], &value_end, 10);
  payload_length = strtoull (argv[3], &value_end, 10);

  // printf("~~~~~~~~~~~~%x------------------", (payload_length/2)<<16);
  printf("----------------------------\n bandwidth = %lu bps\n payload_length = %lx Bytes\n payload_length = 0x%lx Bytes\n",
          bandwidth, payload_length, payload_length);

  //set the payload length Range 46-1500 byte
  if ((status = eb_device_write(device, length_addr, EB_DATA32, (payload_length/2)<<16, 0, 0)) != EB_OK) {
    printf("Payload length error.\n");
  }


  // payload + header length (bit)
  pkg_length = (payload_length + 14)*8;
  // Number of the packet
  rate = bandwidth/pkg_length;
  // clk ticks of one packet
  r_rate_max = 62500000/rate;

  //set the rate for the packet generator
  if ((status = eb_device_write(device, rate_addr, EB_DATA32, r_rate_max, 0, 0)) != EB_OK) {
    printf("%lx-------------%lx.\n",rate_addr,length_addr);
    printf("Rate setting error.\n");
  }

  printf(" Packet length = %d Bytes\n Packet number per second = %d\n ", pkg_length/8, (int)rate);
  return 0;
}
