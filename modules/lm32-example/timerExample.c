/********************************************************************************************
 *  timerExample.c
 *
 *  created : 2020
 *  author  : Dietrich Beck, some code is pirated from Stefan Rauch, GSI-Darmstadt
 *  version : 20-May-2020
 *
 *  very basic example program for using a timer via IRQ and callback in an lm32 softcore
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
#include "pp-printf.h"                                    // print function; please use 'pp_printf' instead of 'printf'
#include "mini_sdb.h"                                     // required for using Wisbhone self-describing bus from lm32

#include "aux.h"                                          // basic helper routines for the lm32 CPU
#include "uart.h"                                         // WR console

// shared memory map for communication via Wishbone 
#include "timerExample_shared_mmap.h"

// stuff required for lm32 environment 
extern uint32_t*       _startshared[];
unsigned int cpuId, cpuQty;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;

// variables for this program
uint64_t tics;                                            // # of timer tics
volatile unsigned int* wb_timer_base = 0;                 // Wishbone address of timer HDL


// generic init used by lm32 programs
void init()
{
  discoverPeriphery();                                    // mini-sdb: get info on important Wishbone infrastructure
  uart_init_hw();                                         // init UART, required for pp_printf...
  cpuId = getCpuIdx();                                    // get ID of THIS CPU 
} // init

// implement simple callback routine for our timer
// important: IRQ and callback take almost 3us, if
// one omits the 'pp_print'
void timer_handler() {
  tics++;
  pp_printf("timer tics: %lu \n", (uint32_t)tics);
} // timer_handler

// init IRQ table; here we just configure the timer
void init_irq_table() {
  isr_table_clr();                                        // clear table
  // isr_ptr_table[0] = &irq_handler;                     // 0: hard-wired MSI; don't use here
  isr_ptr_table[1] = &timer_handler;                      // 1: hard-wired timer
  irq_set_mask(0x02);                                     // only use timer
  irq_enable();                                           // enable IRQs
  pp_printf("IRQ table configured.\n");
} // init_irq_table

// main loop
void main(void) {

  init();                                                 // basic init for a lm32 program

  pp_printf("Hello World!\n");

  // get Wishbone address of timer
  wb_timer_base = (unsigned int*)find_device_adr(GSI, 0xd8baaa13);
  if((int)wb_timer_base == ERROR_NOT_FOUND) {
    pp_printf("no wb_timer found!\n");
  } else {
    pp_printf("wb_timer_base: 0x%x\n", wb_timer_base);
  } // if wb_timer ...

  init_irq_table();                                       // init IRQ
  
  tics             = 0x0;                                 // init tic counter
  wb_timer_base[1] = 125000000UL;                         // set timer to 1 second
  wb_timer_base[0] = 0x1;                                 // start timer
  
  while (1) {
    ;
  } // while
} /* main */
