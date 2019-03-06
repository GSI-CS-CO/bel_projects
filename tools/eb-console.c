/** @file eb-info.c
 *  @brief Report the contents of an FPGA using Etherbone.
 *
 *  Copyright (C) 2013 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
 *  A complete skeleton of an application using the Etherbone library.
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

#define _XOPEN_SOURCE 500 /* strtoull, usleep, tcgetattr */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <etherbone.h>
#include <unistd.h> /* getopt */
#include <termios.h>
#include <fcntl.h>

#define CERN_ID	0xce42
#define UART_ID	0xe2d13d04
#define VUART_TX 0x10
#define VUART_RX 0x14
#define STDIN_FD 0
#define BATCH_SIZE 200

const char *program;

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <proto/host/port>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -i <index>     select between multiple uarts\n");
  fprintf(stderr, "  -h             display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report bugs to <w.terpstra@gsi.de>\n");
}

static void die(const char* msg, eb_status_t status) {
  fprintf(stderr, "%s: %s: %s\n", program, msg, eb_status(status));
  exit(1);
}

int main(int argc, char** argv) {
  int opt, error, c, i, flags, busy;
  char *tail;
  struct sdb_device sdb[10];
  struct termios tc, old;
  eb_status_t status;
  eb_socket_t socket;
  eb_device_t device;
  eb_address_t tx, rx;
  eb_data_t rx_data[BATCH_SIZE], tx_data, done;
  eb_cycle_t cycle;
  char byte;
  
  /* Default arguments */
  program = argv[0];
  error = 0;
  i = -1;
  
  /* Process the command-line arguments */
  error = 0;
  while ((opt = getopt(argc, argv, "i:h")) != -1) {
    switch (opt) {
    case 'h':
      help();
      return 0;
    case 'i':
      i = strtol(optarg, &tail, 0);
      if (*tail != 0) {
        fprintf(stderr, "%s: specify a proper number, not '%s'!\n", program, optarg);
        error = 1;
      }
      break;
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
    die("eb_socket_open", status);
  
  if ((status = eb_device_open(socket, argv[optind], EB_DATAX|EB_ADDRX, 3, &device)) != EB_OK)
    die(argv[optind], status);
  
  c = sizeof(sdb)/sizeof(struct sdb_device);
  if ((status = eb_sdb_find_by_identity(device, CERN_ID, UART_ID, &sdb[0], &c)) != EB_OK)
    die("eb_sdb_find_by_identity", status);
  
  if (i == -1) {
    if (c != 1) {
      fprintf(stderr, "%s: found %d UARTs on that device; pick one with -i #\n", program, c);
      exit(1);
    } else {
      i = 0;
    }
  }
  if (i >= c) {
    fprintf(stderr, "%s: could not find UART #%d on that device (%d total)\n", program, i, c);
    exit(1);
  }
  
  printf("Connected to uart at address %"PRIx64"\n", sdb[i].sdb_component.addr_first);
  tx = sdb[i].sdb_component.addr_first + VUART_TX;
  rx = sdb[i].sdb_component.addr_first + VUART_RX;
  
  /* disable input buffering and echo */
  tcgetattr(STDIN_FD, &old);
  tcgetattr(STDIN_FD, &tc);
  tc.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
  tc.c_iflag = IGNPAR;
  tc.c_oflag = 0;
  tc.c_lflag = 0;
  tc.c_cc[VMIN]=1;
  tc.c_cc[VTIME]=0;
  tcflush(STDIN_FD, TCIFLUSH);
  tcsetattr(STDIN_FD, TCSANOW, &tc);
  
  flags = fcntl(STDIN_FD, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(STDIN_FD, F_SETFL, flags);
  
  /* be lazy and just poll for now */
  busy = 0;
  while (1) {
    int nread;

    if (!busy) usleep(10000); /* 10ms */
    
    /* Poll for status */
    eb_cycle_open(device, 0, eb_block, &cycle);
    eb_cycle_read(cycle, rx, EB_BIG_ENDIAN|EB_DATA32, &rx_data[0]);
    eb_cycle_read(cycle, tx, EB_BIG_ENDIAN|EB_DATA32, &done);
    eb_cycle_close(cycle);
    
    /* Bulk read anything extra */
    if ((rx_data[0] & 0x100) != 0) {
      eb_cycle_open(device, 0, eb_block, &cycle);
      for (i = 1; i < BATCH_SIZE; ++i)
        eb_cycle_read(cycle, rx, EB_BIG_ENDIAN|EB_DATA32, &rx_data[i]);
      eb_cycle_close(cycle);
    
      for (i = 0; i < BATCH_SIZE; ++i) {
        if ((rx_data[i] & 0x100) == 0) continue;
        byte = rx_data[i] & 0xFF;
        fputc(byte, stdout);
      }
      fflush(stdout);
    }
    
    busy = busy && (done & 0x100) == 0;
    
    if (!busy) {
      nread = read(STDIN_FD, &byte, 1);
      if (nread == 0)
        exit(0); /* EOF, user stopped, all right */
      if (nread == 1 && byte == 3) { /* control-C */
        tcsetattr(STDIN_FD, TCSANOW, &old);
        exit(0);
      }
      if (nread == 1) {
        tx_data = byte;
        eb_device_write(device, tx, EB_BIG_ENDIAN|EB_DATA32, tx_data, eb_block, 0);
        busy = 1;
      }
    }
  }
  
  return 0;
}
