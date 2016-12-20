/** @file eb-info.c
 *  @brief Report the contents of an FPGA using Etherbone.
 *
 *  Copyright (C) 2013 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
 *  A complete scnteleton of an application using the Etherbone library.
 *
 *  @author Wesley W. Terpstra <w.terpstra@gsi.de>
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */

#define _POSIX_C_SOURCE 200112L /* strtoull */

#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <etherbone.h>

#define GSI_ID	0x651
#define ROM_ID	0x2d39fa8b
#define CERN_ID 0xce42

#define RAM_ID	      0x54111351
#define FWID_LEN      0x400
#define BOOTL_LEN     0x100
#define SLV_INFO_ROM  0x1c0
#define ECHO_REG      0x20
#define SCU_BUS_ID    0x9602eb6f
#define DEV_BUS_ID    0x35aa6b96

#define SLV_INFO_ROM_SIZE 256
#define IFK_INFO_ROM_SIZE 1024

#define DATA_REG          0x0
#define CMD_REG           0x4
#define IFK_ID            0xcc
#define IFK_BUILD_ID_RD   0xce 
#define IFK_BUILD_ID_RST  0xcf

const char *program;

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <proto/host/port>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -w             show found firmware IDs\n");
  fprintf(stderr, "  -s 1..12       show build info of slave card\n");
  fprintf(stderr, "  -i 0x00..0xff  show build info of ifa8 card\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h             display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report bugs to <w.terpstra@gsi.de>\n");
}

static void die(const char* msg, eb_status_t status) {
  fprintf(stderr, "%s: %s: %s\n", program, msg, eb_status(status));
  exit(1);
}

int main(int argc, char** argv) {
  int opt, error, c, i, j, cnt, idx, inc, len;
  struct sdb_device sdb[25];
  eb_status_t status;
  eb_socket_t socket;
  eb_device_t device;
  eb_cycle_t cycle;
  eb_data_t *data;
  eb_data_t *fwdata;
  eb_data_t *pCur;
  const char cFwMagic[] = "UserLM32";
  unsigned char detectFW = 0;
  char *svalue = NULL;
  char *ivalue = NULL;
  struct sdb_device fwram[25];
  eb_address_t scu_bus;
  eb_address_t dev_bus;
  /* Default arguments */
  program = argv[0];
  error = 0;
  
  
  /* Process the command-line arguments */
  error = 0;
  while ((opt = getopt(argc, argv, "hws:i:")) != -1) {
    switch (opt) {
    case 'w':
      detectFW = 1;
      break;
    case 's':
      svalue = optarg;
      break;
    case 'i':
      ivalue = optarg;
      break;
    case 'h':
      help();
      return 0;
    case ':':
    case '?':
      error = 1;
      break;
    default:
      fprintf(stderr, "%s: bad getopt result\n", program);
      error = 1;
    }
  }
  
  if (error) return 1;
  
  if (optind + 1 != argc) {
    fprintf(stderr, "%s: expecting non-optional argument: <proto/host/port>\n", program);
    return 1;
  }
  
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_DATAX|EB_ADDRX, &socket)) != EB_OK)
    die("eb_soccntet_open", status);
  
  if ((status = eb_device_open(socket, argv[optind], EB_DATAX|EB_ADDRX, 3, &device)) != EB_OK)
    die(argv[optind], status);
  
  c = 1;
  if ((status = eb_sdb_find_by_identity(device, GSI_ID, ROM_ID, &sdb[0], &c)) != EB_OK)
    die("eb_sdb_find_by_identity", status);
  if (c != 1) {
    fprintf(stderr, "Found %d ROM identifiers on that device\n", c);
    exit(1);
  }
  
  if ((status = eb_cycle_open(device, 0, 0, &cycle)) != EB_OK)
    die("eb_cycle_open", status);
  
  len = ((sdb[0].sdb_component.addr_last - sdb[0].sdb_component.addr_first) + 1) / 4;
  if ((data = malloc(len * sizeof(eb_data_t))) == 0)
    die("malloc", EB_OOM);
  
  for (i = 0; i < len; ++i)
    eb_cycle_read(cycle, sdb[0].sdb_component.addr_first + i*4, EB_DATA32|EB_BIG_ENDIAN, &data[i]);
  
  if ((status = eb_cycle_close(cycle)) != EB_OK)
    die("eb_cycle_close", status);
    
  for (i = 0; i < len; ++i) {
    printf("%c%c%c%c", 
      (char)(data[i] >> 24) & 0xff, 
      (char)(data[i] >> 16) & 0xff, 
      (char)(data[i] >>  8) & 0xff,
      (char)(data[i]      ) & 0xff);
  }


  if(ivalue != NULL) {
    int nDevices;
    unsigned int ifa_id = 0;
    char *p;
    errno = 0;
    long conv = strtol(ivalue, &p, 16);
    eb_data_t ifk_version;
   
    if (errno != 0 || *p != '\0' || conv <= 0x0 || conv > 0xff) {
      printf("parameter i is out of range 0x00 - 0xff\n");
      exit(1);
    } else {
      ifa_id = conv;
    }

    printf("\nInfo ROM of IFA 0x%x\n", ifa_id);
    nDevices = 1;
    if ((status = eb_sdb_find_by_identity(device, GSI_ID, DEV_BUS_ID, &sdb[0], &nDevices)) != EB_OK)
      die("find_by_identiy failed", status);

    if (nDevices == 0)
      die("no DEV bus found", EB_FAIL);
    if (nDevices > 1)
      die("more then one DEV bus", EB_FAIL);
  
    dev_bus = sdb[0].sdb_component.addr_first;
    if ((eb_device_write(device, dev_bus + CMD_REG, EB_DATA32|EB_BIG_ENDIAN, IFK_ID << 8 | ifa_id, 0, eb_block)) != EB_OK) {
      exit(1);
    }
    if ((eb_device_read(device, dev_bus + DATA_REG, EB_DATA32|EB_BIG_ENDIAN, &ifk_version, 0, eb_block)) != EB_OK) {
      printf("no ifa card found with this id!\n");
      exit(1);
    }

    eb_device_write(device, dev_bus + CMD_REG, EB_DATA32|EB_BIG_ENDIAN, IFK_BUILD_ID_RST << 8 | ifa_id, 0, eb_block);      
    len = IFK_INFO_ROM_SIZE;
    
    if ((data = malloc(len * sizeof(eb_data_t))) == 0)
      die("malloc", EB_OOM);
  
    for (i = 0; i < len; ++i) {
      eb_device_write(device, dev_bus + CMD_REG, EB_DATA32|EB_BIG_ENDIAN, IFK_BUILD_ID_RD << 8 | ifa_id, 0, eb_block);      
      eb_device_read(device, dev_bus + DATA_REG, EB_DATA32|EB_BIG_ENDIAN, &data[i], 0, eb_block);
    } 


    for (i = 0; i < len; ++i) {
      printf("%c%c", 
        (char)(data[i] >>  8) & 0xff,
        (char)(data[i]      ) & 0xff);
    }
  
  }

  if(svalue != NULL) {

    int nDevices;
    unsigned int slave_id = 0;
    char *p;
    errno = 0;
    long conv = strtol(svalue, &p, 10);
    eb_data_t echo;

    if (errno != 0 || *p != '\0' || conv <= 0 || conv > 12) {
      printf("parameter s is out of range 1-12\n");
      exit(1);
    } else {
      slave_id = conv;
    }

    printf("\nInfo ROM of Slave %d\n", slave_id);
    
    nDevices = 1;
    if ((status = eb_sdb_find_by_identity(device, GSI_ID, SCU_BUS_ID, &sdb[0], &nDevices)) != EB_OK)
      die("find_by_identiy failed", status);

    if (nDevices == 0)
      die("no SCU bus found", EB_FAIL);
    if (nDevices > 1)
      die("more then one SCU bus", EB_FAIL);
  
    scu_bus = sdb[0].sdb_component.addr_first;
 
    if ((eb_device_read(device, scu_bus + slave_id * (1<<17) + ECHO_REG, EB_DATA16|EB_BIG_ENDIAN, &echo, 0, eb_block)) != EB_OK) {
      printf("no slave card found in this slot!\n");
      exit(1);
    }
 
    if ((status = eb_cycle_open(device,0, eb_block, &cycle)) != EB_OK)
      die("EP eb_cycle_open", status);
     
    len = SLV_INFO_ROM_SIZE;
    
    if ((data = malloc(len * sizeof(eb_data_t))) == 0)
      die("malloc", EB_OOM);
  
    for (i = 0; i < len; ++i)
      eb_cycle_read(cycle, scu_bus + slave_id * (1<<17) + SLV_INFO_ROM + i*2, EB_DATA16|EB_BIG_ENDIAN, &data[i]);
  
    if ((status = eb_cycle_close(cycle)) != EB_OK) {
      printf ("no INFO_ROM found on this slave!\n");
      exit(1);
    }

    for (i = 0; i < len; ++i) {
      printf("%c%c%c%c", 
        (char)(data[i] >> 24) & 0xff, 
        (char)(data[i] >> 16) & 0xff, 
        (char)(data[i] >>  8) & 0xff,
        (char)(data[i]      ) & 0xff);
    }

  }
  
  if(detectFW) {
    printf("\nDetecting Firmwares ...\n");
    c = 25;
    if ((status = eb_sdb_find_by_identity(device, GSI_ID, RAM_ID, &sdb[0], &c)) != EB_OK)
      die("eb_sdb_find_by_identity", status);
    if (c > 25) {
      fprintf(stderr, "Found %d RAM identifiers on that device\n", c);
      exit(1);
    }
    
    len = FWID_LEN/4; 
    if ((fwdata = malloc(c * len * (sizeof(eb_data_t)))) == 0)
      die("malloc", EB_OOM);
    char cBuff[FWID_LEN];    
    
    cnt = 0;
    for(idx=0;idx<c;idx++) {
      //RAM Big enough to actually contain a FW ID?
      pCur = &fwdata[cnt * len]; 
      inc = 1;
      if ((sdb[idx].sdb_component.addr_last - sdb[idx].sdb_component.addr_first + 1) >= (FWID_LEN + BOOTL_LEN)) {

        
        if ((status = eb_cycle_open(device, 0, 0, &cycle)) != EB_OK)
          die("eb_cycle_open", status);
      
        for (j = 0; j < len; ++j)
          eb_cycle_read(cycle, sdb[idx].sdb_component.addr_first + BOOTL_LEN + j*4, EB_DATA32|EB_BIG_ENDIAN, &pCur[j]);
      
        if ((status = eb_cycle_close(cycle)) != EB_OK)
          die("eb_cycle_close", status);
        
        //reorder to a normal string
        for (j = 0; j < len; ++j) {
          for (i = 0; i < 4; i++) {
            cBuff[j*4+i] = (char)(pCur[j] >> (8*(3-i)) & 0xff);
          }  
        }
        //check for magic word
//        for(i = 0; i< 8; i++) printf("%c %c\n", cBuff[i], cFwMagic[i]); 
        if(strncmp(cBuff, cFwMagic, 8)) {inc = 0;} 
          
      } else {inc = 0;}
      if(inc){fwram[cnt] = sdb[idx]; cnt++;} 
    }             
    printf("\nFound %u RAMs, %u holding a Firmware ID\n\n", c, cnt); 
    
    for( idx = 0; idx < cnt; idx++) {

      printf("\n********************\n");
      printf("* RAM @ 0x%08x *\n", (unsigned int)fwram[idx].sdb_component.addr_first);
      printf("********************\n");

      pCur = &fwdata[idx * len];
      for (j = 0; j < len; ++j) {
        for (i = 0; i < 4; i++) {
          char tmp = (char)(pCur[j] >> (8*(3-i)) & 0xff); 
	  if(tmp) printf("%c", tmp );
          else goto zeroTerm;
        }  
      }
      zeroTerm:
      printf("*****\n\n");		
    }
  }
  
  
  
  
  return 0;
}
