/********************************************************************************************
 *  timerExample.c
 *
 *  created : 2020
 *  author  : Dietrich Beck, some code is pirated from Stefan Rauch, GSI-Darmstadt
 *  version : 26-May-2020
 *
 *  very basic example program for using a timer via IRQ and callback in an lm32 softcore
 *
 *  it also demonstrated how to read the uptime of the user lm32 'ftm cluter'
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
#include "../wb_timer/wb_timer_regs.h"                    // WB timer

// shared memory map for communication via Wishbone 
#include "timerExample_shared_mmap.h"

// stuff required for lm32 environment 
extern uint32_t*       _startshared[];
unsigned int cpuId, cpuQty;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;

// variables for this program
volatile unsigned int* wb_timer_base = 0;                 // Wishbone address of timer HDL
volatile unsigned int* wb_timer_preset;                   // preset register of timer
volatile unsigned int* wb_timer_config;                   // config register of time
volatile unsigned int* wb_timer_counter;
volatile unsigned int* wb_timer_timestampTick;            // period of a timestamp tick
volatile unsigned int* wb_timer_timestampLo;              // low word of a timestamp
volatile unsigned int* wb_timer_timestampHi;              // high word of a timestamp

// generic init used by lm32 programs
void init()
{
  discoverPeriphery();                                    // mini-sdb: get info on important Wishbone infrastructure
  uart_init_hw();                                         // init UART, required for pp_printf...
  cpuId = getCpuIdx();                                    // get ID of THIS CPU 
} // init

// implement simple callback routine for our timer; this demonstrates
// - how to set up a callback function for the timer IRQ
// - how to read and calculate the uptime of the lm32 ftm-cluster
// as a bonus, we calculate the approximate delay penalty when receiving an IRQ via a callback function
void timer_handler() {
  static uint32_t len    = 0x0;
  static uint32_t preset = 0x0;

  uint64_t ts;
  uint32_t irqDelay;

  if (!len)    len    = *wb_timer_timestampTick;          // read tick length [ns] of timestamp upon first run
  if (!preset) preset = *wb_timer_preset;                 // read timer preset [ticks]

  irqDelay = (preset - *wb_timer_counter) * len;          // read actual counter value, calculate delay for IRQ and convert to nanoseconds

  ts = *wb_timer_timestampLo;                             // read timestamp
  ts = ts + ((uint64_t)(*wb_timer_timestampHi) << 32);
  ts = ts * len;                                          // convert timestamp to nanoseconds
  
  pp_printf("timer_handler: ftm uptime %lu seconds, IRQ delay %lu [ns]\n", (uint32_t)(ts / 1000000000UL), irqDelay);
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
  wb_timer_base = (unsigned int*)find_device_adr(WB_TIMER_SDB_VENDOR_ID, WB_TIMER_SDB_DEVICE_ID);
  if((int)wb_timer_base == ERROR_NOT_FOUND) {
    pp_printf("no wb_timer found!\n");
  } else {
    pp_printf("wb_timer_base: 0x%x\n", wb_timer_base);
  } // if wb_timer ...

  // calculate register addresses for timer
  wb_timer_config        = wb_timer_base + (WB_TIMER_CONFIG >> 2);
  wb_timer_preset        = wb_timer_base + (WB_TIMER_PRESET >> 2);
  wb_timer_counter       = wb_timer_base + (WB_TIMER_COUNTER >> 2);
  wb_timer_timestampTick = wb_timer_base + (WB_TIMER_TIMESTAMP_TICK >> 2);
  wb_timer_timestampLo   = wb_timer_base + (WB_TIMER_TIMESTAMP_LO >> 2);
  wb_timer_timestampHi   = wb_timer_base + (WB_TIMER_TIMESTAMP_HI >> 2);

  // set timer to 1 second
  *wb_timer_preset = 1000000000UL / *wb_timer_timestampTick;  

  init_irq_table();                                       // init IRQ
  *wb_timer_config = 0x1;                                 // start timer

  while (1) {
    ;
  } // while
} /* main */
