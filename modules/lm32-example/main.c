/********************************************************************************************
 *  main.c
 *
 *  created : 2017
 *  author  : Mathias Kreider,  Dietrich Beck, GSI-Darmstadt
 *  version : 23-Feb-2017
 *
 *  example program for lm32 softcore on GSI timing receivers
 * 
 *  a few things are demonstrated
 *  - exchange of data via shared RAM. Shared RAM is accessible from the lm32 (this program) 
 *    and via Wishbone from outside the LM32.
 *  - communication with other Wishbone devices
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
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"

/* register maps for some selected Wishbone devices  */
#include "../../tools/wb_slaves.h" /* this is a hack */

/* shared memory map for communication via Wishbone  */
#include "example_smmap.h"

/* stuff required for environment */
extern uint32_t*       _startshared[];
unsigned int cpuId, cpuQty;
//#define SHARED __attribute__((section(".shared")))
//uint64_t SHARED dummy = 0;

void show_msi()
{
  mprintf(" Msg:\t%08x\nAdr:\t%08x\nSel:\t%01x\n", global_msi.msg, global_msi.adr, global_msi.sel);
}

void isr0()
{
  mprintf("ISR0\n");   
  show_msi();
} 

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

void irq_handler()
{
  show_msi();
  mprintf("huhu\n");
} /* irq_handler */

void init()
{
  /* mini-sdb: get info on important Wishbone infrastructure, such as (this) CPU, flash, EB Master, ... */
  discoverPeriphery();
  /* init UART, required for printf... . To view print message, you may use 'eb-console' from the host  */
  uart_init_hw();
  /* get ID of THIS CPU */ 
  cpuId = getCpuIdx();
  /* ebmInit(); 
  /* set MSI IRQ handler */
  isr_table_clr();
  isr_ptr_table[0] = &isr0;
  irq_set_mask(0x01);
  irq_disable(); 
} /* init */


void main(void) {
  
  int i,j;

  
  /* initialize 'boot' lm32 */
  init();
  
  /* print initial message to UART */
  for (j = 0; j < (125000000/4); ++j) { asm("nop"); }
  mprintf("Hello World!\n");

  /***********************************************************
   * 
   * demonstrate how to talk to a SoC Wishbone device
   * here: get White Rabbit time from WR PPS GEN
   *
   ***********************************************************/
  uint32_t *pPPSGen;            /* WB address of PPS_GEN     */
  uint32_t taiSecs;             /* TAI full seconds          */
  uint32_t taiNsecs;            /* TAI nanoseconds part      */

  /* get Wishbone address of WR PPS GEN                      */
  pPPSGen   = find_device_adr(WR_PPS_GEN_VENDOR, WR_PPS_GEN_PRODUCT);

  /* get data from WR PPS GEN and print to UART              */
  taiSecs  = *(pPPSGen + (WR_PPS_GEN_CNTR_UTCLO >> 2));
  taiNsecs = *(pPPSGen + (WR_PPS_GEN_CNTR_NSEC >> 2));

  /* print TAI to UART */
  mprintf("TAI: %08u.%09u\n", taiSecs, taiNsecs);
  

  /***********************************************************
   *
   * demonstrate exchange if data to Wishbone via shared RAM 
   *
   ***********************************************************/
  uint32_t *pShared;            /* pointer to begin of shared memory region                                   */
  uint32_t *pSharedCounter;     /* pointer to a "user defined" u32 register; here: publish counter            */
  uint32_t *pSharedInput;       /* pointer to a "user defined" u32 register; here: get input from host system */

  /* get pointer to shared memory       */                    
  pShared        = (uint32_t*)_startshared;
  pSharedCounter = (uint32_t*)(pShared + (EXAMPLE_SHARED_COUNTER >> 2));
  pSharedInput   = (uint32_t*)(pShared + (EXAMPLE_SHARED_INPUT >> 2));

  /* print pointers to UART             */
  mprintf("shared memory: start            @ 0x%08x\n", (uint32_t)pShared);
  mprintf("shared memory: counter register @ 0x%08x\n", (uint32_t)pSharedCounter);
  mprintf("shared memory: input register   @ 0x%08x\n", (uint32_t)pSharedInput);

  /* initialize values of shared memory */
  *pSharedCounter = 0x0;
  *pSharedInput   = 0x0;

  /* write counter value to shared memory. Moreover, read and print value from shared memory               */
  /* initialize counter */
  i=0;
  while (i<10) {
    for (j = 0; j < (125000000/4); ++j) { asm("nop"); } /* wait for 1 second                               */
    i++;
    /* write value of counter 'i' to shared memory. Use eb-read to obtain the value from the host system.  */
    *pSharedCounter = i;
    /* read and print value from shared memory                                                             */
    /* via Wishbone - from outside the lm32, change the value of the input register (eb-write form host).  */
    /* Use 'eb-console' to see printed value                                                               */
    mprintf("value of input register: 0d%08d\n", *pSharedInput);
  } /* while 1 */


 /***********************************************************
   * 
   * demonstrate how to poll actions ("events") from ECA
   *
   ***********************************************************/
#define ECAQMAX           4
#define ECACHANNELFORLM32 2  
  sdb_location ECAQ_base[ECAQMAX]; /*needed to find ECA queue*/
  uint32_t ECAQidx = 0;         /* needed to find ECA queue  */
  uint32_t *tmp;                /* needed to find ECA queue  */
  uint32_t *pECAQ  ;            /* WB address of ECA queue   */
  uint32_t evtIdHigh;           /* high 32bit of eventID     */
  uint32_t evtIdLow;            /* low 32bit of eventID      */
  uint32_t evtDeadlHigh;        /* high 32bit of deadline    */
  uint32_t evtDeadlLow;         /* low 32bit of deadline     */
  uint32_t flag;                /* flag for the next action  */

  /* get Wishbone address of ECA queue                       */
  /* get list of ECA queues                                  */
  find_device_multi(ECAQ_base, &ECAQidx, ECAQMAX, ECA_QUEUE_SDB_VENDOR_ID, ECA_QUEUE_SDB_DEVICE_ID);
  pECAQ = 0x0;
  /* find ECA queue connected to ECA chanel for LM32         */
  for (i=0; i < ECAQidx; i++) {
    tmp = (getSdbAdr(&ECAQ_base[i]));  
    if ( *(tmp + (ECA_QUEUE_QUEUE_ID_GET >> 2)) == ECACHANNELFORLM32) pECAQ = tmp;
  }
  if (!pECAQ) { mprintf("ERROR: can't find ECA queue for lm32, good bye! \n"); while(1) asm("nop"); }
  mprintf("ECA queue found at: %08x. Waiting for actions ...\n", pECAQ);
   

  /* poll ECA queue for actions                              */
  while (1) {
    /* read flag and check if there was an action            */
    flag         = *(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2));
    if (flag & 0x10) { /* data is valid?                     */

      /* read data */
      evtIdHigh    = *(pECAQ + (ECA_QUEUE_EVENT_ID_HI_GET >> 2));
      evtIdLow     = *(pECAQ + (ECA_QUEUE_EVENT_ID_LO_GET >> 2));
      evtDeadlHigh = *(pECAQ + (ECA_QUEUE_DEADLINE_HI_GET >> 2));
      evtDeadlLow  = *(pECAQ + (ECA_QUEUE_DEADLINE_LO_GET >> 2));

      /* pop action from channel */
      *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;

      mprintf("EvtID: 0x%08x%08x; deadline: %08x%08x; flag: %08x\n", evtIdHigh, evtIdLow, evtDeadlHigh, evtDeadlLow, flag);
    } /* if data is valid */
    else {
      for (j = 0; j < (125000000/4000000); ++j) { asm("nop"); } /* wait for 1 microsecond */
    } /* else: data not valid */
  } /* while */
    
} /* main */
