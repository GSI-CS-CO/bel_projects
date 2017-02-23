/******************************************************************************
 *  main.c
 *
 *  created : 2017
 *  author  : Mathias Kreider,  Dietrich Beck, GSI-Darmstadt
 *  version : 23-Feb-2017
 *
 *  example program for lm32 softcore on GSI timing receivers
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

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "mprintf.h"
#include "mini_sdb.h"

/* includes specific for bel_projects */
#include "irq.h"
#include "ebm.h"
#include "aux.h"
#include "dbg.h"

/* register map for communication via Wishbone */
#include "example_regs.h"

/* stuff required for environment */
extern uint32_t*       _startshared[];
unsigned int cpuId, cpuQty;
//#define SHARED __attribute__((section(".shared")))
//uint64_t SHARED dummy = 0;

/* 
void show_msi()
{
  mprintf(" Msg:\t%08x\nAdr:\t%08x\nSel:\t%01x\n", global_msi.msg, global_msi.adr, global_msi.sel);

}

void isr0()
{
   mprintf("ISR0\n");   
   show_msi();
} 

*/

/*
void ebmInit()
{
   int j;
   
   while (*(pEbCfg + (EBC_SRC_IP>>2)) == EBC_DEFAULT_IP) {
     for (j = 0; j < (125000000/2); ++j) { asm("nop"); }
     mprintf("#%02u: DM cores Waiting for IP from WRC...\n", cpuId);  
   } 

   ebm_init();
   ebm_config_meta(1500, 42, 0x00000000 );
   ebm_config_if(DESTINATION, 0xffffffffffff, 0xffffffff,                0xebd0); //Dst: EB broadcast - CAREFUL HERE!
   ebm_config_if(SOURCE,      0xd15ea5edbeef, *(pEbCfg + (EBC_SRC_IP>>2)), 0xebd0); //Src: bogus mac (will be replaced by WR), WR IP

}
*/

void init()
{
  /* mini-sdb: get info on important Wishbone infrastructure, such as (this) CPU, flash, EB Master, ... */
  discoverPeriphery();
  /* init UART, required for printf... . To view print message, you may use 'eb-console'                */
  uart_init_hw();
  /* get ID of THIS CPU */ 
  cpuId = getCpuIdx();
  /* 
  ebmInit();
  */
  /* set MSI IRQ handler */
  isr_table_clr();
  irq_set_mask(0x01);
  irq_disable(); 
}


void main(void) {
   
  int i,j;
  uint32_t *pShared;            /* pointer to begin of shared memory region                                   */
  uint32_t *pSharedCounter;     /* pointer to a "user defined" u32 register; here: publish counter            */
  uint32_t *pSharedInput;       /* pointer to a "user defined" u32 register; here: get input from host system */

  /* initialize 'boot' lm32 */
  init();

  /* get pointer values to shared memory */
  pShared        = (uint32_t*)_startshared;
  pSharedCounter = (uint32_t*)((uint8_t*)pShared + EXAMPLE_SHARED_COUNTER);
  pSharedInput   = (uint32_t*)((uint8_t*)pShared + EXAMPLE_SHARED_INPUT);

  mprintf("shared memory: start   @ 0x%08x\n", (uint32_t)pShared);
  mprintf("shared memory: counter @ 0x%08x\n", (uint32_t)pSharedCounter);
  mprintf("shared memory: input   @ 0x%08x\n", (uint32_t)pSharedInput);
  mprintf("sharsdfsdfed memory: input   @ 0x%08x\n", (uint32_t)EXAMPLE_SHARED_INPUT);

  /* initialize values of I/O registers */
  *pSharedCounter = 0x0;
  *pSharedInput   = 0x0;

  /* print initial message to UART */
  for (j = 0; j < (125000000/4); ++j) { asm("nop"); }
  mprintf("Hello World!\n");







  

  /* demonstrate exchange if data to Wishbone via shared RAM                                               */
  i=0;
  while (1) {
    for (j = 0; j < (125000000/4); ++j) { asm("nop"); } /* wait for 1 second                               */
    i++;
    /* write value of counter 'i' to shared memory'. Use eb-read to obtain the value from the host system. */
    *pSharedCounter = i;
    /* read and print value of input register.  */
    /* Use eb-write to change the value from the host system. Use 'eb-console' to see printed value        */
    mprintf("input register: 0d%08d\n", *pSharedInput);
  } /* while 1 */
} /* main */
