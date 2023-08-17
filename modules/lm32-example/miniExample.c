/********************************************************************************************
 *  miniExample.c
 *
 *  created : 2017
 *  author  : Mathias Kreider,  Dietrich Beck, GSI-Darmstadt
 *  version : 05-Jun-2020
 *
 *  very basic example program for lm32 softcore on GSI timing receivers
 *
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2017  Dietrich Beck
 * GSI Helmholtzzentrum für Schwerionenforschung GmbH
 * Planckstraße 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: d.beck@gsi.de
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
 * For all questions and ideas contact: d.beck@gsi.de
 * Last update: 25-April-2015
 ********************************************************************************************/

// standard includes
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

// includes specific for bel_projects
#include "pp-printf.h"
#include "mini_sdb.h"
#include "aux.h"
#include "dbg.h"
#include "uart.h"

// shared memory map for communication via Wishbone
#include "miniExample_shared_mmap.h"

// stuff required for environment
extern uint32_t*       _startshared[];
unsigned int cpuId, cpuQty;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;

void init(){
  discoverPeriphery();   // mini-sdb: get info on important Wishbone infrastructure
  uart_init_hw();        // init UART, required for printf...
  cpuId = getCpuIdx();   // get ID of THIS CPU
} // init

int main(void) {
  int j;

  init();

  // wait 1 second and print initial message to UART
  // pro tip: try 'eb-console' to view printed messages
  for (j = 0; j < (31000000); ++j) { asm("nop"); } // 31.25 x 'asm("nop")' operations take 1 us; handmade sleep

  pp_printf("Hello World!\n");

  while (1) {
    pp_printf("boring...\n");
    uwait(1000000);                                // alternative to handmade sleep using the wb_timer for lm32
  } // while

  return 0;
} // main
