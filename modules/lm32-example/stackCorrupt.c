/********************************************************************************************
 *  stackCorrupt.c
 *
 *  created : 2019
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 04-Jun-2019
 *
 *  this lm32 program will corrupt its own stack. As a result, the content of the 
 *  firmware ID will change (try 'eb-info -w <proto/host/port>')
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
#include <uart.h>
#include <stack.h>
#include "pp-printf.h"
#include "mini_sdb.h"
#include "aux.h"
#include "dbg.h"

// shared memory map for communication via Wishbone 
#include "stackCorrupt_shared_mmap.h"

/* stuff required for environment */
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
  
  uint32_t *buildID;

  init();

  buildID           = (uint32_t *)(INT_BASE_ADR + BUILDID_OFFS);
  
  // wait 1 second and print initial message to UART
  // pro tip: try 'eb-console' to view printed messages
  for (j = 0; j < (31000000); ++j) { asm("nop"); } // 31.25 x 'asm("nop")' operations take 1 us.
  pp_printf("Hello World!\n");

  while (1) {
    check_stack_fwid(buildID);
    pp_printf("boring... (doing bad things)\n");

    // ohps!!!! (this overwrites the magic word protecting the stack)
    _endram = 0xcafebabe;
    
    for (j = 0; j < (31000000); ++j) { asm("nop"); }
  } // while
  return (1); // this should never happen
} // main
