////////////////////////////////////////////////////////////////////////////////
//
// filename: eb-iflash.c
// desc: flash program for updating the gateware of an ifa8 card with an SCU
// creation date: 24.05.2017
// last modified:
// author: Stefan Rauch <s.rauch@gsi.de>
//
// Copyright (C) 2017 GSI Helmholtz Centre for Heavy Ion Research GmbH 
//
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////// 
//
//standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>
#include <etherbone.h>
#include <sys/time.h>


#define GSI_ID      0x651
#define CERN_ID     0xce42
#define DEV_BUS_ID  0x35aa6b96
#define SCU_BUS_ID  0x9602eb6f

#define DATA_REG    0x0
#define CMD_REG     0x4
#define IFK_ID      0xcc
#define IFK_VERS    0xcd
#define STAT_BUFFER 0xa8
#define STAT_MAX    0xa9
#define STAT_MIN    0xaa
#define STAT_AVG    0xab

#define SDB_DEVICES 1

#define CALC_OFFS(SLOT)   (((SLOT) * (1 << 17))) // from slot 1 to slot 12
// addresses for scu bus *2, for wb_mil *4
#define MIL_SIO3_OFFSET   0x400
#define MIL_SIO3_TX_DATA  0x400
#define MIL_SIO3_TX_CMD   0x401
#define MIL_SIO3_STAT     0x402
#define MIL_SIO3_RESET    0x412
#define MIL_SIO3_RX_TASK1 0xd01
#define MIL_SIO3_TX_TASK1 0xc01
#define MIL_SIO3_RX_TASK2 0xd02
#define MIL_SIO3_TX_TASK2 0xc02
#define MIL_SIO3_D_RCVD   0xe00
#define MIL_SIO3_D_ERR    0xe10
#define MIL_SIO3_TX_REQ   0xe20
#define TASKMIN           1
#define TASKMAX           254
#define TASK              241
#define   OKAY                 1
#define   TRM_NOT_FREE        -1
#define   RCV_ERROR           -2
#define   RCV_TIMEOUT         -3
#define   RCV_TASK_ERR        -4
#define   RCV_PARITY          -5
#define   ERROR               -6
#define   RCV_TASK_BSY        -7
#define   MIL_RST_WAIT        1500000
#define   MIL_RST_PULSE       500

#define IFA_ID            0xfa00
#define RELOAD_FAILSAVE   0x1
#define RELOAD_USER       0x2
#define WR_LW_ADDR        0x4
#define WR_HW_ADDR        0x8
#define ERASE_FIFO        0x40
#define WR_FIFO           0x80
#define FIFO_TO_USER      0x100
#define RDFIFO_EMPTY      0x800
#define RDFIFO_NOT_FULL   0x1000
#define RD_USER_FLASH     0x2000
#define ERASE_USER_FLASH  0x4000
#define RELOAD_USER_L     0x8000

#define FWL_STATUS_WR     0x66
#define FWL_STATUS_RD     0x9d
#define FWL_DATA_WR       0x65
#define FWL_DATA_RD       0x9c
#define MAGIC_WORD        0x654321
#define PAGE_SIZE         256
#define EPCS_SIZE         2048 * PAGE_SIZE



static const char* devName;
static const char* program;
static eb_device_t device;
static eb_socket_t socket;


void itoa(unsigned int n,char s[], int base){
     int i;
 
     i = 0;
     do {                           /* generate digits in reverse order */
         s[i++] = n % base + '0';   /* get next digit */
     } while ((n /= base) > 0);     /* delete it */
     s[i] = '\0';
}

static const unsigned char BitReverseTable256[] = 
{
  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 
  0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 
  0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 
  0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 
  0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 
  0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
  0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 
  0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
  0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
  0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 
  0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
  0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
  0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 
  0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
  0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 
  0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

unsigned char flip(unsigned char byte) {
  return BitReverseTable256[byte];
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
  printf("Usage: eb-fg-statistic [OPTION] <proto/host/port>\n");
  printf("\n");
  printf("-h             show the help for this program\n");
  printf("-i <ifa adr>   address of the ifa from 0x0 to 0x254\n");
  printf("-s <sio slot>  slot number of sio card with dev bus\n");
  printf("-f             scan for ifas on dev bus\n");
  printf("-r             read from statistic buffer\n");
  printf("-t             reset the mil interface\n");
}

/* blocking read usign a task slot */
int devb_read(eb_address_t base, int task, int ifa_addr, unsigned char fc, eb_data_t* read_value) {
  eb_data_t rx_data_avail;
  eb_data_t rx_err;
  unsigned int reg_offset;
  unsigned int bit_offset;
  //eb_data_t rx_req;
  if ((task < TASKMIN) || (task > TASKMAX))
    return RCV_TASK_ERR;

  // fetch avail and err bits
  reg_offset = task / 16;
  bit_offset = task % 16;
  if (!(fc >> 7))  {
    printf("not a read fc!\n");
    exit(1);
  } else {
    // write fc and addr to taskram
    if ((eb_device_write(device, base + (MIL_SIO3_TX_TASK1 + task - 1) * 4, EB_DATA32|EB_BIG_ENDIAN, fc << 8 | ifa_addr, 0, eb_block)) != EB_OK)
      return RCV_TASK_ERR;
    // wait for task to finish, a read over the dev bus needs at least 40us
    if ((eb_device_read(device, base + (MIL_SIO3_D_RCVD + reg_offset) * 4, EB_DATA32|EB_BIG_ENDIAN, &rx_data_avail, 0, eb_block)) != EB_OK)
      return RCV_TASK_ERR;
      
    while(!(rx_data_avail & (1 << bit_offset))) {
      usleep(1);
      if ((eb_device_read(device, base + (MIL_SIO3_D_RCVD + reg_offset) * 4, EB_DATA32|EB_BIG_ENDIAN, &rx_data_avail, 0, eb_block)) != EB_OK)
        return RCV_TASK_ERR;
    }

    // task finished
    if ((eb_device_read(device, base + (MIL_SIO3_D_ERR + reg_offset) * 4, EB_DATA32|EB_BIG_ENDIAN, &rx_err, 0, eb_block)) != EB_OK)
      return RCV_ERROR;

    // no error
    if ((rx_data_avail & (1 << bit_offset)) && !(rx_err & (1 << bit_offset))) {
      if ((eb_device_read(device, base + (MIL_SIO3_RX_TASK1 + task - 1) * 4, EB_DATA32|EB_BIG_ENDIAN, read_value, 0, eb_block)) == EB_OK)
      return OKAY;
    // error bit set
    } else {
      if ((eb_device_read(device, base + (MIL_SIO3_RX_TASK1 + task - 1) * 4, EB_DATA32|EB_BIG_ENDIAN, read_value, 0, eb_block)) == EB_OK) {
        if ((*read_value & 0xffff) == 0xdead)
          return RCV_PARITY;
        else
          return RCV_TIMEOUT;
      }

    }
    return RCV_ERROR;

  }
}
/* blocking read usign a task slot */
int scub_devb_read(eb_address_t base, int task, int slot, int ifa_addr, unsigned char fc, eb_data_t* read_value) {
  eb_data_t rx_data_avail;
  eb_data_t rx_err;
  unsigned int reg_offset;
  unsigned int bit_offset;
  //eb_data_t rx_req;
  if ((task < TASKMIN) || (task > TASKMAX))
    return RCV_TASK_ERR;

  // fetch avail and err bits
  reg_offset = task / 16;
  bit_offset = task % 16;
  if (!(fc >> 7))  {
    printf("not a read fc!\n");
    exit(1);
  } else {
    // write fc and addr to taskram
    if ((eb_device_write(device, base + CALC_OFFS(slot) + (MIL_SIO3_TX_TASK1 + task - 1)  * 2, EB_DATA16|EB_BIG_ENDIAN, fc << 8 | ifa_addr, 0, eb_block)) != EB_OK)
      return RCV_TASK_ERR;
    // wait for task to finish, a read over the dev bus needs at least 40us
    if ((eb_device_read(device, base + CALC_OFFS(slot) +  (MIL_SIO3_D_RCVD + reg_offset) * 2, EB_DATA16|EB_BIG_ENDIAN, &rx_data_avail, 0, eb_block)) != EB_OK)
      return RCV_TASK_ERR;
      
    while(!(rx_data_avail & (1 << bit_offset))) {
      usleep(1);
      if ((eb_device_read(device, base + CALC_OFFS(slot) + (MIL_SIO3_D_RCVD + reg_offset) * 2, EB_DATA16|EB_BIG_ENDIAN, &rx_data_avail, 0, eb_block)) != EB_OK)
        return RCV_TASK_ERR;
    }

    // task finished
    if ((eb_device_read(device, base + CALC_OFFS(slot) + (MIL_SIO3_D_ERR + reg_offset) * 2, EB_DATA16|EB_BIG_ENDIAN, &rx_err, 0, eb_block)) != EB_OK)
      return RCV_ERROR;

    // no error
    if ((rx_data_avail & (1 << bit_offset)) && !(rx_err & (1 << bit_offset))) {
      if ((eb_device_read(device, base + CALC_OFFS(slot) + (MIL_SIO3_RX_TASK1 + task - 1) * 2, EB_DATA16|EB_BIG_ENDIAN, read_value, 0, eb_block)) == EB_OK)
      return OKAY;
    // error bit set
    } else {
      if ((eb_device_read(device, base + CALC_OFFS(slot) + (MIL_SIO3_RX_TASK1 + task - 1) * 2, EB_DATA16|EB_BIG_ENDIAN, read_value, 0, eb_block)) == EB_OK) {
        if ((*read_value & 0xffff) == 0xdead)
          return RCV_PARITY;
        else
          return RCV_TIMEOUT;
      }

    }
    return RCV_ERROR;

  }
}
   
// write the word write_value to the function code fc of the ifa with the addr ifa_addr
// uses the mil extension card of an SCU
int devb_write(eb_address_t base, int ifa_addr, unsigned char fc, eb_data_t write_value) {
  if (fc >> 7)  {
    printf("not a write fc!\n");
    exit(1);
  } else {
    if ((eb_device_write(device, base + MIL_SIO3_TX_DATA * 4, EB_DATA32|EB_BIG_ENDIAN, write_value, 0, eb_block)) != EB_OK) {
      printf("devb write tx data failed!\n");
      exit(1);
    }
    if ((eb_device_write(device, base + MIL_SIO3_TX_CMD * 4, EB_DATA32|EB_BIG_ENDIAN, fc << 8 | ifa_addr, 0, eb_block)) != EB_OK) {
      printf("devb write tx cmd failed!\n");
      exit(1);
    }
  }
  return OKAY;
}

// write the word write_value to the function code fc of the ifa with the addr ifa_addr
// uses the mil extension card of an SCU
int scub_devb_write(eb_address_t base, int slot, int ifa_addr, unsigned char fc, eb_data_t write_value) {
  eb_data_t status;
  if (fc >> 7)  {
    printf("not a write fc!\n");
    exit(1);
  } else {
    status = eb_device_write(device, base + CALC_OFFS(slot) + MIL_SIO3_TX_DATA * 2 , EB_DATA16|EB_BIG_ENDIAN, write_value, 0, eb_block);
    if (status != EB_OK)
      die("scub devb write tx data failed\n", status);

    status = eb_device_write(device, base + CALC_OFFS(slot) + MIL_SIO3_TX_CMD * 2, EB_DATA16|EB_BIG_ENDIAN, fc << 8 | ifa_addr, 0, eb_block);
    if (status != EB_OK)
      die("scub devb write tx cmd failed\n", status);
  }
  return OKAY;
}

// reset mil controller
int reset_mil(eb_address_t base) {
  //reset
  if ((eb_device_write(device, base + MIL_SIO3_RESET * 4, EB_DATA32|EB_BIG_ENDIAN, 0, 0, eb_block)) != EB_OK) {
    printf("resetting mil failed!\n");
    exit(1);
  }
  usleep(500);
  //release reset
  if ((eb_device_write(device, base + MIL_SIO3_RESET * 4, EB_DATA32|EB_BIG_ENDIAN, 0xff, 0, eb_block)) != EB_OK) {
    printf("resetting mil failed!\n");
    exit(1);
  }
  return OKAY;
}

// reset mil controller
int scub_reset_mil(eb_address_t base, int slot) {
  //reset
  if ((eb_device_write(device, base + CALC_OFFS(slot) + MIL_SIO3_RESET * 2, EB_DATA16|EB_BIG_ENDIAN, 0, 0, eb_block)) != EB_OK) {
    printf("resetting mil failed!\n");
    exit(1);
  }
  usleep(500);
  //release reset
  if ((eb_device_write(device, base + CALC_OFFS(slot) + MIL_SIO3_RESET * 2, EB_DATA16|EB_BIG_ENDIAN, 0xff, 0, eb_block)) != EB_OK) {
    printf("resetting mil failed!\n");
    exit(1);
  }
  return OKAY;
}
   



// print information about the found ifa
void check_ifa_addr(eb_address_t devb_base, eb_address_t scub_base, unsigned char slot, int ifa_addr) {
  eb_data_t id;
  eb_data_t version;
  int status;

  if (slot < 1) {
    status = devb_read(devb_base, TASK, ifa_addr, IFK_ID, &id);
    devb_read(devb_base, TASK, ifa_addr, IFK_VERS, &version);
  } else {
    status = scub_devb_read(scub_base, TASK, slot, ifa_addr, IFK_ID, &id);
    scub_devb_read(scub_base, TASK, slot, ifa_addr, IFK_VERS, &version);
  }

  if (status == OKAY)
    printf("Found IFA with addr 0x%x and id 0x%"EB_DATA_FMT" and vers 0x%"EB_DATA_FMT"\n", ifa_addr, id, version);
  else if (status == RCV_ERROR)
    printf("no IFA found, rcv error\n");
  else if (status == RCV_TIMEOUT)
    printf("no IFA found, rcv timeout\n");
  else if (status == RCV_PARITY)
    printf("no IFA found, rcv parity\n");
  else if (status == RCV_TASK_ERR)
    printf("no IFA found, rcv task error\n");
}

// scan dev bus for ifas
void scanDevBus(eb_address_t dev_base, eb_address_t scu_base, unsigned char slot) {
  eb_data_t id;
  eb_data_t version;
  int status;
  int addr;

  if (slot < 1) {
    for (addr = 0; addr < 254; addr++) {
      status = devb_read(dev_base, TASK, addr, IFK_ID, &id);
      devb_read(dev_base, TASK, addr, IFK_VERS, &version);
      if (status == OKAY)
        printf("Found IFA with addr 0x%x and id 0x%"EB_DATA_FMT" and vers 0x%"EB_DATA_FMT"\n", addr, id, version);
    }
  } else if (slot > 0) {
    for (addr = 0; addr < 254; addr++) {
      status = scub_devb_read(scu_base, TASK, slot, addr, IFK_ID, &id);
      scub_devb_read(scu_base, TASK, slot, addr, IFK_VERS, &version);
      if (status == OKAY)
        printf("Found IFA with addr 0x%x and id 0x%"EB_DATA_FMT" and vers 0x%"EB_DATA_FMT"\n", addr, id, version);
    }
  }
}

double time_diff(struct timeval x, struct timeval y) {
  double x_ms, y_ms, diff;

  x_ms = (double)x.tv_sec*1000000 + (double)x.tv_usec;
  y_ms = (double)y.tv_sec*1000000 + (double)y.tv_usec;

  diff = (double)y_ms - (double)x_ms;
  return diff;
}
  


int main(int argc, char * const* argv) {
  eb_status_t status;
  eb_data_t value;
  struct sdb_device sdbDevice[SDB_DEVICES];
  int nDevices;  
  eb_address_t dev_bus;
  eb_address_t scu_bus;

  char *ivalue = NULL;
  char *svalue = NULL;
  int fflag = 0;
  int rflag = 0;
  int tflag = 0;
  int index;
  int c;
 
  unsigned int ifa_addr = 0;
  unsigned char slot = 0;
  char *p;
  errno = 0;
 
  /* Process the command-line arguments */
  opterr = 0;
  while ((c = getopt (argc, argv, "i:w:v:hfuers:t")) != -1)
    switch (c)
      {
        case 'w':
          break;
        case 'v':
          break;
        case 'i':
          ivalue = optarg;
          break;
        case 'r':
          rflag = 1;
          break;
        case 'f':
          fflag = 1;
          break;
        case 'e':
          break;
        case 's':
          svalue = optarg;
          break;
        case 't':
          tflag = 1;
          break;
        case 'h':
          show_help();
          exit(1);
        case '?':
          if (optopt == 'w' || optopt == 'v' || optopt == 'i' || optopt == 's')
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

  // convert input to numbers
  if (ivalue != NULL) {
    long conv = strtol(ivalue, &p, 16);
    if (errno != 0 || *p != '\0' || conv <= 0x0 || conv > 0xff) {
      printf("parameter i is out of range 0x00 - 0xff\n");
      exit(1);
    } else {
      ifa_addr = conv;
    }
  } else {
    if (!fflag) {
      fprintf(stderr, "no ifa address set!\n");
      exit(1);
    }
  }

  if (svalue != NULL) {
    unsigned char conv = strtol(svalue, &p, 10);
    if (errno != 0 || *p != '\0' || conv <= 0x0 || conv > 12) {
      printf("parameter t is out of range 1 - 12\n");
      exit(1);
    } else {
      slot = conv;
    }
  }

  // assign non option arguments
  index = optind;

  if (argc < 3 || argc-optind < 1) {
    printf("program needs at least the device name of the etherbone device and an ifa address in the range 0-254.\n");
    printf("e.g. %s -i0x50 dev/wbm0\n", argv[0]);
    exit(0);
  }

  if (index < argc) {
    devName = argv[index];
    index++;
  }
 
  
  /* Open a socket supporting only 32-bit operations.
   * As we are not exporting any slaves^Mwe don't care what port we get => 0.
   * This function always returns immediately.
   * EB_ABI_CODE helps detect if the application matches the library.
   */
  if ((status = eb_socket_open(EB_ABI_CODE,0, EB_ADDR32|EB_DATA32, &socket)) != EB_OK)
    die("eb_socket_open",status);
  
  /* Open the remote device with 3 attempts to negotiate bus width.
   * This function is blocking and may stall the thread for up to 3 seconds.
   * If you need asynchronous open see eb_device_open_nb.
   * Note: the supported widths can never be more than the socket supports.
   */
  if ((status = eb_device_open(socket,devName, EB_ADDR32|EB_DATA32, 3, &device)) != EB_OK)
    die("eb_device_open",status);

  nDevices = 1;
  if ((status = eb_sdb_find_by_identity(device, GSI_ID, DEV_BUS_ID, &sdbDevice[0], &nDevices)) != EB_OK)
    die("find_by_identiy failed", status);

  if (nDevices == 0)
    die("no DEV bus found", EB_FAIL);
  if (nDevices > 1)
    die("more then one DEV bus", EB_FAIL);

  dev_bus = sdbDevice[0].sdb_component.addr_first;

  // search for scu bus
  nDevices = 1;
  if ((status = eb_sdb_find_by_identity(device, GSI_ID, SCU_BUS_ID, &sdbDevice[0], &nDevices)) != EB_OK)
    die("find_by_identiy failed", status);

  if (nDevices == 0)
    die("no SCU bus found", EB_FAIL);
  if (nDevices > 1)
    die("more then one SCU bus", EB_FAIL);

  scu_bus = sdbDevice[0].sdb_component.addr_first;

    
  // reset mil controller
  if (tflag == 1) {
    if(svalue != NULL)
      scub_reset_mil(scu_bus, slot);
    else
      reset_mil(dev_bus);
  }

  // scan dev bus
  if (fflag == 1) {
    scanDevBus(dev_bus, scu_bus, slot);
    exit(1);
  }

  // read from statistics buffer
  if (rflag == 1) {
    struct timeval start_t, end_t;
    double diff_t;
    while (1) {
      gettimeofday(&start_t, NULL);

      if (slot < 1) 
        devb_read(dev_bus, TASK, ifa_addr, STAT_BUFFER, &value);
      else
        scub_devb_read(scu_bus, TASK, slot, ifa_addr, STAT_BUFFER, &value);
      gettimeofday(&end_t, NULL);
      diff_t = time_diff(start_t, end_t);
      if (diff_t > 3000.0)
        usleep(20000);

      // primitive stream control
      if (value == 0) {
        usleep(20000);
        continue;
      }
      printf("0x%"EB_DATA_FMT", Execution time = %.01f us\n", value, diff_t);
      usleep(10000);
    }
  }

    

  //print information about the found ifa
  //check_ifa_addr(dev_bus, scu_bus, slot, ifa_addr);

  /* close handler cleanly */
  if ((status = eb_device_close(device)) != EB_OK)
    die("eb_device_close",status);
  if ((status = eb_socket_close(socket)) != EB_OK)
    die("eb_socket_close",status);
  
  return 0;
}
