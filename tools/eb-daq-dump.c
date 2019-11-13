//
// eb-sflash: tool for programming scu slave card remotly with new gateware
// 

//standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>
//Etherbone
#include <etherbone.h>


#define GSI_ID       0x651
#define CERN_ID      0xce42
#define LM32_USER    0x54111351
#define MAGIC_NUMBER 0x524
#define DAQ_BUF      0x65b8

#define SDB_DEVICES     3
#define DAQ_RING_SIZE   2048 
#define MESSAGE_SIZE    5
#define DAQ_RING_HEADER 8

static const char* devName;
static const char* program;
static eb_address_t lm32_ram_base;
static eb_device_t device;
static eb_cycle_t  cycle;
static eb_socket_t socket;

void itoa(unsigned int n,char s[], int base){
     int i;
 
     i = 0;
     do {                           /* generate digits in reverse order */
         s[i++] = n % base + '0';   /* get next digit */
     } while ((n /= base) > 0);     /* delete it */
     s[i] = '\0';
}




void die_eb(const char* where,eb_status_t status) {
  fprintf(stderr,"%s: %s failed: %s\n",
    program,where, eb_status(status));
  exit(1);
}
void die(const char* where,eb_status_t status) {
  fprintf(stderr,"%s: %s failed: %s\n",
    program,where, eb_status(status));
  exit(1);
}


void show_help() {
  printf("Usage: eb-daq-dump [OPTION] <proto/host/port>\n");
  printf("\n");
  printf("-h          show the help for this program\n");
}

void remove_daq_message() {
  eb_status_t status;
  eb_data_t message[MESSAGE_SIZE];
  eb_data_t ring_head, ring_tail;
  int i;

  // get ring_head and ring_tail
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK)
    die("EP eb_cycle_open", status);
  eb_cycle_read(cycle, lm32_ram_base + DAQ_BUF + 0, EB_BIG_ENDIAN|EB_DATA32, &ring_head);   
  eb_cycle_read(cycle, lm32_ram_base + DAQ_BUF + 4, EB_BIG_ENDIAN|EB_DATA32, &ring_tail);   

  if ((status = eb_cycle_close(cycle)) != EB_OK) {
    die("eb_cycle_close failed", status);
  }

  if (ring_head != ring_tail) {
    // get the message from position ring_tail
    if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK)
      die("EP eb_cycle_open", status);
    for (i = 0; i < MESSAGE_SIZE; i++) {
      eb_cycle_read(cycle, lm32_ram_base + DAQ_BUF                      // begin of daq buffer in shared memory
                                         + DAQ_RING_HEADER              // header with head and tail
                                         + 4 * MESSAGE_SIZE * ring_tail // position of message in the buffer
                                         + 4 * i                        // word position in the message
                                          , EB_BIG_ENDIAN|EB_DATA32, &message[i]);   
    }
    if ((status = eb_cycle_close(cycle)) != EB_OK) {
      die("eb_cycle_close failed", status);
    }

    ring_tail = (ring_tail + 1) % DAQ_RING_SIZE;
    if ((status = eb_device_write(device, lm32_ram_base + DAQ_BUF + 4, EB_DATA32, ring_tail, 0, 0)) != EB_OK)
      die("eb_device_write ring_tail", status);

    printf("%ld", message[2] | (message[3] << 32));
    printf(" %d", (signed)message[1]);
    printf(" %d", ((signed)message[0]) >> 16);

    printf(" fg-%u-%u\n", (unsigned)message[4] >> 24, (unsigned)(message[4] & 0x00ff0000) >> 16);
  }

}

int main(int argc, char * const* argv) {
  eb_status_t status;
  struct sdb_device sdbDevice[SDB_DEVICES];
  int nDevices;  
  int c;
  opterr = 0;

  while ((c = getopt (argc, argv, "h")) != -1)
    switch (c)
      {
      case 'h':
        show_help();
        exit(1);
      case '?':
        if (optopt == 'w' || optopt == 'v' || optopt == 's')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }
  
  if (argc < 2 || argc-optind < 1) {
    printf("program needs at least the device name of the etherbone device.\n");
    printf("e.g. %s dev/wbm0\n", argv[0]);
    exit(0);
  }

 devName = argv[1];
 
  /* Open a socket supporting only 32-bit operations.
   * As we are not exporting any slaves^Mwe don't care what port we get => 0.
   * This function always returns immediately.
   * EB_ABI_CODE helps detect if the application matches the library.
   */
  if ((status = eb_socket_open(EB_ABI_CODE,0, EB_ADDR32|EB_DATA32, &socket)) != EB_OK)
    die("eb_socket_open",status);
  
  /* Open the remote device with 3 attemptis to negotiate bus width.
   * This function is blocking and may stall the thread for up to 3 seconds.
   * If you need asynchronous open^Msee eb_device_open_nb.
   * Note: the supported widths can never be more than the socket supports.
   */
  if ((status = eb_device_open(socket,devName, EB_ADDR32|EB_DATA32, 3, &device)) != EB_OK)
    die("eb_device_open",status);

  nDevices = 1;
  if ((status = eb_sdb_find_by_identity(device, GSI_ID, LM32_USER, sdbDevice, &nDevices)) != EB_OK)
    die("find_by_identiy failed", status);

  if (nDevices == 0)
    die("no LM32_USER_RAM found", EB_FAIL);
  
  /* Record the address of the device */
  lm32_ram_base = sdbDevice[0].sdb_component.addr_first;
  
/*
  if ((status = eb_cycle_open(device,0, eb_block, &cycle)) != EB_OK)
    die("EP eb_cycle_open", status);
     
  eb_data_t value[7];
  int offset = 0;
  for (i = 0; i < 7; i++) {
    eb_cycle_read(cycle, lm32_ram_base + DAQ_BUF + offset, EB_BIG_ENDIAN|EB_DATA32, &value[i]);   
    offset+=4;
  }
  if ((status = eb_cycle_close(cycle)) != EB_OK) {
    printf("eb_cycle_close failed\n");
    exit(1);
  }
  for (i = 0; i < 7; i++) {
    printf("daq: 0x%"EB_DATA_FMT"\n", value[i]);
  }
*/
  while (1) {
    remove_daq_message();
    usleep(1);
  }

  /* close handler cleanly */
  if ((status = eb_device_close(device)) != EB_OK)
    die("eb_device_close",status);
  if ((status = eb_socket_close(socket)) != EB_OK)
    die("eb_socket_close",status);
  
  return 0;
}
