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

#define RAM_ID	0x54111351
#define FWID_LEN 0x400
#define BOOTL_LEN 0x100

const char *program;

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <proto/host/port>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -w             show found firmware IDs\n");
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
  struct sdb_device fwram[25];
  /* Default arguments */
  program = argv[0];
  error = 0;
  
  
  /* Process the command-line arguments */
  error = 0;
  while ((opt = getopt(argc, argv, "hw")) != -1) {
    switch (opt) {
    case 'w':
      detectFW = 1;
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
    fprintf(stderr, "%s: expecting three non-optional arguments: <proto/host/port>\n", program);
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
        //checcnt for magic word
        for(i = 0; i< 8; i++) printf("%c %c\n", cBuff[i], cFwMagic[i]); 
        if(strncmp(cBuff, cFwMagic, 8)) {inc = 0;} 
          
      } else {inc = 0;}
      if(inc){fwram[cnt] = sdb[idx]; cnt++;} 
    }             
    printf("\nFound %u RAMs, %u holding a Firmware ID\n\n", c, cnt); 
    
    for( idx = 0; idx < cnt; idx++) {
      printf("\nRAM @ 0x%08x: \n\n", (unsigned int)fwram[idx].sdb_component.addr_first);
      pCur = &fwdata[idx * len];
      for (j = 0; j < len; ++j) {
        for (i = 0; i < 4; i++) {
          printf("%c", (char)(pCur[j] >> (8*(3-i)) & 0xff) );
        }  
      }
    }
  }
  
  
  
  
  return 0;
}
