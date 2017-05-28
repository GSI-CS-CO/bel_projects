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
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>
//Etherbone
#include <etherbone.h>


#define GSI_ID      0x651
#define CERN_ID     0xce42
#define DEV_BUS_ID  0x35aa6b96

#define DATA_REG    0x0
#define CMD_REG     0x4
#define IFK_ID      0xcc

#define PAGE_SIZE   256
#define SDB_DEVICES 1

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

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}


void show_help() {
  printf("Usage: eb-iflash [OPTION] <proto/host/port>\n");
  printf("\n");
  printf("-h             show the help for this program\n");
  printf("-i <ifa adr>   address of the ifa from 0x0 to 0x254\n");
  printf("-f             trigger failsave reconfiguration\n");
  printf("-u             trigger user reconfiguration\n");
  printf("-w <file>      write programming file into flash\n");
  printf("-v <file>      verify flash against programming file\n");
}

// read from a function code fc of the ifa with the addr ifa_addr
// if the read is successful, the word read is in read_value
// uses the mil extension card of an SCU
void devb_read(eb_address_t base, int ifa_addr, unsigned char fc, eb_data_t* read_value) {
  if (!(fc >> 7))  {
    printf("not a read fc!\n");
    exit(1);
  } else {
    if ((eb_device_write(device, base + CMD_REG, EB_DATA32|EB_BIG_ENDIAN, fc << 8 | ifa_addr, 0, eb_block)) != EB_OK) {
      printf("eb write failed!\n");
      exit(1);
    }
    if ((eb_device_read(device, base + DATA_REG, EB_DATA32|EB_BIG_ENDIAN, read_value, 0, eb_block)) != EB_OK) {
      printf("no IFA card found with this addr!\n");
      exit(1);
    }
  }
}
   
// write the word write_value to the function code fc of the ifa with the addr ifa_addr
// uses the mil extension card of an SCU
void devb_write(eb_address_t base, int ifa_addr, unsigned char fc, eb_data_t write_value) {
  if (fc >> 7)  {
    printf("not a write fc!\n");
    exit(1);
  } else {
    if ((eb_device_write(device, base + DATA_REG, EB_DATA32|EB_BIG_ENDIAN, write_value, 0, eb_block)) != EB_OK) {
      printf("eb write failed!\n");
      exit(1);
    }
    if ((eb_device_write(device, base + CMD_REG, EB_DATA32|EB_BIG_ENDIAN, fc << 8 | ifa_addr, 0, eb_block)) != EB_OK) {
      printf("eb write failed!\n");
      exit(1);
    }
  }
}
   
// sets the addr registers in the firmware loader. flash_addr is written to
// low word and high word register
void set_flash_addr(eb_address_t base, int ifa_addr, unsigned int flash_addr) {
  devb_write(base, ifa_addr, FWL_STATUS_WR, WR_LW_ADDR); 
  devb_write(base, ifa_addr, FWL_DATA_WR, flash_addr & 0xffff);
  devb_write(base, ifa_addr, FWL_STATUS_WR, WR_HW_ADDR); 
  devb_write(base, ifa_addr, FWL_DATA_WR, flash_addr >> 16);
}

void clear_flash(eb_address_t base, int ifa_addr) {
  eb_data_t status;
  set_flash_addr(base, ifa_addr, MAGIC_WORD);
  devb_write(base, ifa_addr, FWL_STATUS_WR, ERASE_USER_FLASH);
  devb_read(base, ifa_addr, FWL_STATUS_RD, &status);
  // wait for operation to finish
  while(status & ERASE_USER_FLASH)
    devb_read(base, ifa_addr, FWL_STATUS_RD, &status);
}

void check_ifa_addr(eb_address_t base, int ifa_addr) {
  eb_data_t read_val;
  devb_read(base, ifa_addr, IFK_ID, &read_val);
  printf("Found IFA with addr 0x%x and id 0x%"EB_DATA_FMT"\n", ifa_addr, read_val);
}



int main(int argc, char * const* argv) {
  eb_status_t status;
  struct sdb_device sdbDevice[SDB_DEVICES];
  int nDevices;  
  eb_address_t dev_bus;

  char *wvalue = NULL;
  int rflag = 0;
  char *vvalue = NULL; 
  char *ivalue = NULL;
  int fflag = 0;
  int uflag = 0;
  int index;
  int c;
 
  unsigned int ifa_id = 0;
  char *p;
  errno = 0;
 
  /* Process the command-line arguments */
  opterr = 0;
  while ((c = getopt (argc, argv, "i:w:rv:hfu")) != -1)
    switch (c)
      {
      case 'w':
        wvalue = optarg;
        break;
      case 'r':
        rflag = 1;
        break;
      case 'v':
        vvalue = optarg;
        break;
      case 'i':
        ivalue = optarg;
        break;
      case 'u':
        uflag = 1;
        break;
      case 'f':
        fflag = 1;
        break;
      case 'h':
        show_help();
        exit(1);
      case '?':
        if (optopt == 'w' || optopt == 'v' || optopt == 'i')
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

  if (ivalue != NULL) {
    long conv = strtol(ivalue, &p, 16);
    if (errno != 0 || *p != '\0' || conv <= 0x0 || conv > 0xff) {
      printf("parameter i is out of range 0x00 - 0xff\n");
      exit(1);
    } else {
      ifa_id = conv;
    }
  } else {
    fprintf(stderr, "no ifa address set!\n");
    exit(1);
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
  
  /* Open the remote device with 3 attemptis to negotiate bus width.
   * This function is blocking and may stall the thread for up to 3 seconds.
   * If you need asynchronous open^Msee eb_device_open_nb.
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

  //print information about the found ifa
  check_ifa_addr(dev_bus, ifa_id);
  //clear_flash(dev_bus, ifa_id);
  
  // load failsave config
  if (fflag == 1) {
    devb_write(dev_bus, ifa_id, FWL_STATUS_WR, RELOAD_FAILSAVE);
    printf("reload fpga with failsave config.\n");
    exit(1);
  }
  // load user config
  if (uflag == 1) {
    devb_write(dev_bus, ifa_id, FWL_STATUS_WR, RELOAD_USER);
    printf("reload fpga with user config.\n");
    exit(1);
  }

  FILE *fp;

  //write file
  if (wvalue != NULL) {
    if ((fp = fopen(wvalue, "r")) == NULL) {
      printf("open of programming file not successful.\n");
      exit(1);
    }
    struct stat buf;
    stat(wvalue, &buf);
    int size = buf.st_size;
    if (size % PAGE_SIZE) {
      printf("size of programming file is not a multiple of %d\n", PAGE_SIZE);
      exit(1);
    }
    printf("filesize: %d bytes\n", size);

    //read in data from stdin
      //reverse bits before writing

      //copy reversed bytes to page_buffer in 16Bit chunks


      //check written data
        
    fclose(fp);
    printf("New image written to epcs.\n");
  }

  //read page
  if (rflag == 1) {

  }

  //verify flash against programming file
  if (vvalue != NULL) { 
   
    printf("Starting Verify...\n"); 
    if ((fp = fopen(vvalue, "r")) == NULL) {
      printf("open of programming file not successful.\n");
      exit(1);
    }
    struct stat buf;
    stat(vvalue, &buf);
    int size = buf.st_size;
    if (size % PAGE_SIZE) {
      printf("size of programming file is not a multiple of %d\n", PAGE_SIZE);
      exit(1);
    }
    printf("filesize: %d bytes\n", size);

    //while( fread(&file_page, 1,  PAGE_SIZE, fp) == PAGE_SIZE) {}

    fclose(fp);
    printf("Verify successful!\n");
  }

  /* close handler cleanly */
  if ((status = eb_device_close(device)) != EB_OK)
    die("eb_device_close",status);
  if ((status = eb_socket_close(socket)) != EB_OK)
    die("eb_socket_close",status);
  
  return 0;
}
