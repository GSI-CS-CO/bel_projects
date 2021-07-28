/********************************************************************************************
 *  Example program to present an issue with pp_printf()
 *
 *  Compile it for the SCU3 timing receiver
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
 * Last update: 10-September-2019
 ********************************************************************************************/

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "pp-printf.h"
#include "mini_sdb.h"
#include "stack.h"

/* includes specific for bel_projects */
#include "aux.h"
#include "dbg.h"

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

uint64_t a[4];

void main(void) {
  int j;

  init();

  uint32_t u32_a, u32_b;
  uint8_t  u8_a, u8_b;
  u32_a = u32_b = 0x11223344;
  u8_a = u8_b = 0xab;

  // wait 1 second and print initial message to UART
  // pro tip: try 'eb-console' to view printed messages
  for (j = 0; j < (31000000); ++j) { asm("nop"); } // 31.25 x 'asm("nop")' operations take 1 us.
  pp_printf("Hello World!\n");

  for (int i = 0; i < 4; ++i)
    a[i] = 0xdeadbeefdeadbeef;

  // an issue with pp_printf()
  // expected output: 11223344, 11223344, ab, ab, deadbeefdeadbeef, deadbeefdeadbeef
  // but got:         11223344, 11223344, ab, ab, deadbeefdeadbeef, ce42deadbeef
  pp_printf(" %x, %x, %x, %x, %Lx, %Lx\n", u32_a, u32_b, u8_a, u8_b, a[0], a[0]);
  check_stack();

  // but after changing the argument order output is correct
  // output:          11223344, 11223344, ab, deadbeefdeadbeef, deadbeefdeadbeef, ab
  pp_printf(" %x, %x, %x, %Lx, %Lx, %x\n", u32_a, u32_b, u8_a, a[0], a[0], u8_b);
  check_stack();

  while (1) {
    check_stack();
    for (j = 0; j < (310000000); ++j) { asm("nop"); } // takes around 10 seconds
    pp_printf("boring...\n");
  } // while
} /* main */
