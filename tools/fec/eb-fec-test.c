/******************************************************************************
 *  eb-fec.c
 *
 *  created : 2018
 *  author  : Cesar Prados, GSI-Darmstadt
 *  version : Jan 2018
 *
 * Command-line interface for FEC via Etherbone.
 *
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2013 Cesar Prados
 * GSI Helmholtzzentrum für Schwerionenforschung GmbH
 * Planckstraße 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: c.prados@gsi.de - bradomyn@mailfence.com
 *
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
 *
 * For all questions and ideas contact: c.prados@gsi.de
 ********************************************************************************************/
#define EBMON_VERSION "1.3.0"

/* standard includes */
#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

/* Etherbone */
#include <etherbone.h>

///* Wishbone api */
#include <wb_api.h>
#include <wb_slaves.h>


/*** WB_DROPPER ****/
#define WB_DROPPER_VENDOR            WB_GSI              /* vendor ID */
#define WB_DROPPER_PRODUCT           0x73c0b112          /* product ID */
#define WB_DROPPER_VMAJOR            1                   /* major revision */
#define WB_DROPPER_VMINOR            1                   /* minor revision */

/**** WB_FEC Register ****/
#define WB_DROPPER_CONF                     0x0

/*** WB_CNT ****/
#define WB_CNT_VENDOR            WB_GSI              /* vendor ID */
#define WB_CNT_PRODUCT           0x73aaa112          /* product ID */
#define WB_CNT_VMAJOR            1                   /* major revision */
#define WB_CNT_VMINOR            1                   /* minor revision */

/**** WB_FEC Register ****/
#define WB_CNT_CNT                     0x0
#define WB_CNT_MIS                     0x4


const char* program;
static int verbose=0;
eb_device_t       device;
eb_address_t wb_dropper_addr    = EB_NULL;
eb_address_t wb_cnt_addr        = EB_NULL;
eb_device_t  known              = EB_NULL;   /* etherbone device */

struct error_stat {
  unsigned long one_error;
  unsigned long two_error;
  unsigned long three_error;
  unsigned long x01;
  unsigned long x02;
  unsigned long x03;
  unsigned long x05;
  unsigned long x06;
  unsigned long x07;

};

static struct error_stat stat;

static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
}

static unsigned int hamming_dist(int next_drop) {

  unsigned int count = 0;

  while (next_drop)
  {
    count += next_drop & 1;
    next_drop >>= 1;
  }

  return count;
}

static void drop_cal(int next_drop, struct error_stat *stat) {

  unsigned int count = 0;

  if (next_drop == 1) {
    stat->x01++;
  } else if (next_drop == 2) {
    stat->x02++;
  } else if (next_drop == 3) {
    stat->x03++;
  } else if (next_drop == 5) {
    stat->x05++;
  } else if (next_drop == 6) {
    stat->x06++;
  } else if (next_drop == 7) {
    stat->x07++;
  }

  count = hamming_dist(next_drop);

  if (count == 1) {
    printf("Dropping 1 FEC packet\n");
    stat->one_error++;
  } else if (count == 2) {
    printf("Dropping 2 FEC packets\n");
    stat->two_error++;
  } else {
    printf("Dropping 3 FEC packets\n");
    stat->three_error++;
  }
}

static eb_status_t wb_check_device(eb_device_t device, uint64_t vendor_id, uint32_t product_id, uint8_t ver_major, uint8_t ver_minor, int devIndex, eb_address_t *addr)
{
  eb_address_t tmp;
  eb_status_t  status;

  if ((known == EB_NULL) || (known != device) ||  (*addr == EB_NULL)) {
    known = EB_NULL;
    *addr = EB_NULL;
  }

  if ((status = wb_get_device_address(device, vendor_id, product_id, ver_major, ver_minor, devIndex, &tmp)) != EB_OK) return status;

  known  = device;
  *addr  = tmp;

  return status;
} /* wb_check device */


eb_status_t wb_dropper(eb_device_t device, int devIndex, int drop, int *current_drop)
{
  eb_address_t  address;
  eb_status_t   status;
  eb_data_t     tmp;
  eb_data_t     data;

  if ((status = wb_check_device(device, WB_DROPPER_VENDOR, WB_DROPPER_PRODUCT, WB_DROPPER_VMAJOR, WB_DROPPER_VMINOR, devIndex, &wb_dropper_addr)) != EB_OK) return status;

  address = wb_dropper_addr;

  if ((status = eb_device_read(device, address, EB_BIG_ENDIAN|EB_DATA32, &tmp, 0, eb_block)) != EB_OK) return status;
  *current_drop = (uint32_t)tmp;

  data = (eb_data_t)drop;
  if ((status = eb_device_write(device, address, EB_BIG_ENDIAN|EB_DATA32, data, 0, eb_block)) != EB_OK) return status;

  return EB_OK;
}


eb_status_t wb_cnt(eb_device_t device, int devIndex, unsigned long *cnt, unsigned long *cnt_miss)
{
  eb_address_t  address;
  eb_status_t   status;
  eb_data_t     tmp;

  if ((status = wb_check_device(device, WB_CNT_VENDOR, WB_CNT_PRODUCT, WB_CNT_VMAJOR, WB_CNT_VMINOR, devIndex, &wb_cnt_addr)) != EB_OK) return status;

  address = wb_cnt_addr;

  if ((status = eb_device_read(device, address, EB_BIG_ENDIAN|EB_DATA32, &tmp, 0, eb_block)) != EB_OK) return status;
  *cnt = (uint32_t)tmp;

  address = wb_cnt_addr + WB_CNT_MIS;

  if ((status = eb_device_read(device, address, EB_BIG_ENDIAN|EB_DATA32, &tmp, 0, eb_block)) != EB_OK) return status;
  *cnt_miss = (uint32_t)tmp;

  return EB_OK;
}


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -t<polling interval> \n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <c.pradosk@gsi.de> <bradomyn@mailfence.com>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", EBMON_VERSION);
}


int main(int argc, char** argv) {
  eb_status_t       status;
  eb_socket_t       socket;
  struct fec_info   info;
  int               devIndex=-1;  /* 0,1,2... - there may be more than 1 device on the WB bus */

  const char* devName;
  char name[7]={0};

  int         getEBVersion=0;
  int         exitCode=0;
  int         next_refresh;
  int         next_drop=0;
  int         current_drop=0;
  unsigned long cnt = 0;
  unsigned long cnt_miss = 0;


  program = argv[0];

  if (optind >= argc) {
    fprintf(stderr, "%s: expecting one non-optional argument: <etherbone-device>\n", program);
    fprintf(stderr, "\n");
    help();
    return 1;
  }

  devName = argv[optind];

  printf("\nLog file name: ");
  scanf("%s", name);


  if (devIndex < 0) devIndex = 0; /* default: grab first device of the requested type on the wishbone bus */

  if (getEBVersion) {
    if (verbose) fprintf(stdout, "EB version / EB source: ");
    fprintf(stdout, "%s / %s\n", eb_source_version(), eb_build_info());
  }

  /* open Etherbone device and socket */
  if ((status = wb_open(devName, &device, &socket)) != EB_OK) {
    fprintf(stderr, "can't open connection to device %s \n", devName);
    return (1);
  }

  srand(time(NULL));

  while(1) {
    printf("\e[1;1H\e[2J");

   FILE *f;
   f = fopen(name, "wb+");

   if (f == NULL)
   {
     printf("Error opening file!\n");
   }

    if ((status = wb_fec_info(device, devIndex, &info)) != EB_OK) die("------FEC Info ----- \n", status);

    printf("------- FEC Info %s ------\n", devName);
    fprintf(f,"------- FEC Info %s ------\n", devName);

    printf("# FEC Configuration ");
    fprintf(f,"# FEC Configuration ");

    if (info.fec_type == 0) {
      printf("Fixed Rate Code\n");
      fprintf(f,"Fixed Rate Code\n");
    } else if (info.fec_type == 1) {
      printf("Fixed Rate Code + Golay\n");
      fprintf(f,"Fixed Rate Code + Golay\n");
    } else if (info.fec_type == 2) {
      printf("LDPC\n");
      fprintf(f,"Fixed Rate Code + Golay\n");
    } else if (info.fec_type == 3) {
      printf("LDPC + Golay\n");
      fprintf(f,"LDPC + Golay\n");
    }

    printf("# FEC Stats \n");
    fprintf(f,"# FEC Stats \n");

    if (info.config == 0) {
      printf("Encoder: OFF -- Decoder: OFF\n");
      fprintf(f,"Encoder: OFF -- Decoder: OFF\n");
    } else if (info.config == 1) {
      printf("Encoder: ON -- Decoder: OFF\n");
      fprintf(f,"Encoder: OFF -- Decoder: OFF\n");
    } else if (info.config == 2) {
      printf("Encoder: OFF -- Decoder: ON\n");
      fprintf(f,"Encoder: OFF -- Decoder: OFF\n");
    } else if (info.config == 3) {
      printf("Encoder: ON -- Decoder: ON\n");
      fprintf(f,"Encoder: ON -- Decoder: ON\n");
    }

    printf("FEC Ethertype: 0x%x \n", info.fec_ethtype);
    printf("Etherbone Ethertype: 0x%x \n", info.eth_ethtype);
    printf("Encoder Packets: %d\n", info.enc_cnt);
    printf("Decoder Packets: %d\n", info.dec_cnt);
    printf("Error Decoder: %d\n", info.err_dec);
    printf("Error Encoder: %d\n", info.err_enc);
    printf("Jumbo Packet Rx: %d\n", info.jumbo_rx);

    fprintf(f,"FEC Ethertype: 0x%x \n", info.fec_ethtype);
    fprintf(f,"Etherbone Ethertype: 0x%x \n", info.eth_ethtype);
    fprintf(f,"Encoder Packets: %d\n", info.enc_cnt);
    fprintf(f,"Decoder Packets: %d\n", info.dec_cnt);
    fprintf(f,"Error Decoder: %d\n", info.err_dec);
    fprintf(f,"Error Encoder: %d\n", info.err_enc);
    fprintf(f,"Jumbo Packet Rx: %d\n", info.jumbo_rx);



    printf("\n---Drop Stats---%d\n", info.jumbo_rx);
    printf("One Error: %ld\n", stat.one_error);
    printf("Two Error: %ld\n", stat.two_error);
    printf("Three Error: %ld\n", stat.three_error);

    printf("x01: %ld\n", stat.x01);
    printf("x02: %ld\n", stat.x02);
    printf("x03: %ld\n", stat.x03);
    printf("x05: %ld\n", stat.x05);
    printf("x06  %ld\n", stat.x06);
    printf("x07: %ld\n", stat.x07);

    fprintf(f,"\n---Drop Stats---%d\n", info.jumbo_rx);
    fprintf(f,"One Error: %ld\n", stat.one_error);
    fprintf(f,"Two Error: %ld\n", stat.two_error);
    fprintf(f,"Three Error: %ld\n", stat.three_error);

    fprintf(f,"x01: %ld\n", stat.x01);
    fprintf(f,"x02: %ld\n", stat.x02);
    fprintf(f,"x03: %ld\n", stat.x03);
    fprintf(f,"x05: %ld\n", stat.x05);
    fprintf(f,"x06  %ld\n", stat.x06);
    fprintf(f,"x07: %ld\n", stat.x07);

    wb_cnt(device, devIndex, &cnt, &cnt_miss);


    printf("\n---Packet Cnt Stats---%d\n", info.jumbo_rx);
    printf("Cnt: %ld\n", cnt);
    printf("Cnt Missing: %ld\n", cnt_miss);
    fprintf(f,"\n---Packet Cnt Stats---%d\n", info.jumbo_rx);
    fprintf(f,"Cnt: %ld\n", cnt);
    fprintf(f,"Cnt Missing: %ld\n", cnt_miss);

    next_refresh = rand()%500;
    next_drop = rand()%14;

    drop_cal(next_drop, &stat);

    wb_dropper(device, devIndex, next_drop, &current_drop);

    printf("------- Current Num of Pkt Drop %d \n", hamming_dist(current_drop));
    printf("------- Next Num of Pkt Drop in %d \n", hamming_dist(next_drop));
    printf("------- Next Burst of Pkt Drop in %d sec \n", next_refresh);

    fprintf(f,"------- Current Num of Pkt Drop %d \n", hamming_dist(current_drop));
    fprintf(f,"------- Next Num of Pkt Drop in %d \n", hamming_dist(next_drop));
    fprintf(f,"------- Next Burst of Pkt Drop in %d sec \n", next_refresh);


    fclose(f);
    sleep(next_refresh);
  }

  wb_close(device, socket);

  return exitCode;
}
