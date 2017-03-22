/********************************************************************************************
 *  milExample.c
 *
 *  created : 2017
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 21-Mar-2017
 *
 *  example program for lm32 softcore on GSI timing receivers
 * 
 *  this demonstrates the use of
 *  - MIL device bus as master
 *  - Mil event bus as slave
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
#include "aux.h"
#include "dbg.h"
#include "../../top/gsi_scu/scu_mil.h"


/* shared memory map for communication via Wishbone  */
#include "example_shared_mmap.h"
#include "example_smmap.h"

/* stuff required for environment */
extern uint32_t*       _startshared[];
unsigned int cpuId, cpuQty;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;

unsigned int *pMILPiggy;         /* WB address of MIL piggy on SCU                                             */
uint32_t *pShared;               /* pointer to begin of shared memory region                                   */
uint32_t *pCpuRamExternal;       /* external address (seen from host bridge) of this CPU's RAM                 */

void init()
{
  /* mini-sdb: get info on important Wishbone infrastructure, such as (this) CPU, flash, EB Master, ... */
  discoverPeriphery();
  /* init UART, required for printf... . To view print message, you may use 'eb-console' from the host  */
  uart_init_hw();
  /* get ID of THIS CPU */ 
  cpuId = getCpuIdx();
  /* set MSI IRQ handler */
  isr_table_clr();
  irq_set_mask(0x01);
  irq_disable(); 
} /* init */


/***********************************************************
 *
 * demonstrate exchange of data to Wishbone via shared RAM 
 * - the data can be accessed via Etherbone->Wishbone
 * - try eb-read/eb-write from the host system
 *
 ***********************************************************/
void useSharedMem()
{
  int      i,j;
  uint32_t idx;
  const uint32_t c_Max_Rams = 10;
  sdb_location found_sdb[c_Max_Rams];
  sdb_location found_clu;

  /* get pointer to shared memory       */                    
  pShared        = (uint32_t *)_startshared;

  /* print local pointer info to UART   */
  mprintf("internal shared memory: start            @ 0x%08x\n", (uint32_t)pShared);

  idx = 0;
  find_device_multi(&found_clu, &idx, 1, GSI, LM32_CB_CLUSTER);	
  idx = 0;
  find_device_multi_in_subtree(&found_clu, &found_sdb[0], &idx, c_Max_Rams, GSI, LM32_RAM_USER);
  if(idx >= cpuId) {
    pCpuRamExternal = (uint32_t*)(getSdbAdr(&found_sdb[cpuId]) & 0x7FFFFFFF); /* CPU sees the 'world' under 0x8..., remove that bit to get host bridge perspective */
    /* print external WB info to UART     */
    mprintf("external WB address   : start           @ 0x%08x\n", (uint32_t)(pCpuRamExternal));
  } else {
    pCpuRamExternal = (uint32_t*)ERROR_NOT_FOUND;
    mprintf("Could not find external WB address of my own RAM !\n");
  } 
} /* useSharedMem */


/*************************************************************
* 
* demonstrate how to talk to MIL devicebus and receive MIL events
* HERE: get WB address of Wishbone device GSI_MIL_SCU "MIL Piggy"
*
**************************************************************/
void initMILPiggy(){
  /* get Wishbone address for MIL piggy */
  pMILPiggy   = (unsigned int *)find_device_adr(GSI, SCU_MIL);
  mprintf("pMILPiggy: 0x%08x\n",  pMILPiggy);
} /* init MILDevicebus */


/*************************************************************
* 
* demonstrate how to get MIL Events of the MIL piggy FIFO
* and how to handle them
*
**************************************************************/
void MILEvtHandler(){
  uint32_t evtData;            /* data of one MIL event                           */
  uint32_t evtCode;            /* "event number"                                  */
  uint32_t virtAcc;            /* virtual accelerator                             */
  uint32_t beamStat;           /* beam status                                     */
  uint32_t evtTime, evtTime1, evtTime2, evtTime3; /* EVT_TIME (time in a.u.)      */
  uint32_t secsUTC, msecsUTC, UTC1, UTC2, UTC3, UTC4, UTC5; /* EVT_UTC (> 2008)   */
  uint32_t help1, help2; 
  int i;

  i = 0;
  while (i < 500) {
    if (fifo_notempty_evt_mil(pMILPiggy)) {
      i++;
      pop_fifo_evt_mil(pMILPiggy, (unsigned int*)(&evtData));
      evtCode  = evtData & 0x000000ff;
      help1   = (evtData >> 8);
      switch (evtCode) {
      case 0 ... 199:           /* 000..199: timing events */
        virtAcc  = help1 & 0x0f;
        beamStat = help1 >> 4;
        mprintf("EVENT - code: %03u, virtAcc: %02u, stat: %02u\n", evtCode, virtAcc, beamStat);
        break;
      case 209:                 /* 209..211: time in 10ms of the current day */
        evtTime1 = help1;
        break;
      case 210:
        evtTime2 = help1;
        break;
      case 211:
        evtTime3 = help1;
        evtTime2 = (evtTime2 << 8);
        evtTime1 = (evtTime1 << 16);
        evtTime = evtTime1 | evtTime2 | evtTime3;
        mprintf("------TIME: %09u [a.u.]\n", evtTime);
        break;
      case 224:                 /* 224..228: "UTC" time starting 1 Jan 2008 */
          UTC1 = help1;
          break;
      case 225:
        UTC2 = help1;
        break;
      case 226:
        UTC3 = help1;
        break;
      case 227:
        UTC4 = help1;
        break;
      case 228:
        UTC5 = help1;
        
        UTC1 = UTC1 << 2;
        help2 = (UTC2 >> 2);
        msecsUTC = help2 | UTC1;
        
        UTC2 = UTC2 & 0x3f;
        UTC2 = UTC2 << 24;
        UTC3 = UTC3 << 16;
        UTC4 = UTC4 << 8;
        secsUTC = UTC2 | UTC3 | UTC4 | UTC5;
        mprintf("---UTC:  %09u.%03u [seconds since 1. Jan 2008)\n", secsUTC, msecsUTC);
        break;
      default:
        mprintf("---------OTHER - code: %03u, data: 0x%04x\n", evtCode, help1);
      } /* switch */
    } /* if regValue */
  } /* while */
} /* MILEvtHandler */

/*************************************************************
* 
* demonstrate how to talk to a MIL device
* HERE: write (read) data to (from)  the echo register of 
* a MIL device
*
**************************************************************/
void testEchoMILDevice(short wData)
{
  short ifkAddr  = 0x20;   /* address of interface card               */
  short fcEchoW  = 0x13;   /* function code: write to echo register   */
  short fcEchoR  = 0x89;   /* function code: read from echo register  */
  short rData    = 0x0;    /* data to read                            */
  unsigned short fc_ifk;   /* function code and interface card addr   */
  int busStatus = 0;       /* status of bus operation                 */

  mprintf("writeData: 0x%x to IFK with address 0x%x\n", wData, ifkAddr);

  /* write to echo register */
  fc_ifk = ifkAddr | (fcEchoW << 8);
  busStatus = write_mil(pMILPiggy, wData, fc_ifk);
  mprintf("fc_ifk: 0x%x - bus status: %d\n", fc_ifk, busStatus);

  /* read from echo register */
  fc_ifk = ifkAddr | (fcEchoR << 8);
  busStatus = read_mil(pMILPiggy, &rData, fc_ifk);
  mprintf("fc_ifk: 0x%x - bus status: %d\n", fc_ifk, busStatus);
  
  mprintf("readData: 0x%x from IFK with address 0x%x\n", rData, ifkAddr);

} /* testEchoMILDevice */

void main(void) {
  
  int i,j;
  
  /* initialize 'boot' lm32 */
  init();
  
  /* wait 1 second and print initial message to UART         */
  for (j = 0; j < (31000000); ++j) { asm("nop"); }
  mprintf("Hello World!\n");

  useSharedMem();        /* read/write to shared memory      */
  initMILPiggy();        /* init MIL piggy on SCU            */
  testEchoMILDevice(0xbabe);  /* write/read to a MIL device  */

  /* write initial values to status register                 */
  write_statusreg_evt_mil(pMILPiggy, MIL_ENDECODER_FPGA | MIL_INTR_DEB_ON);
  /* disable and clear filter                                */
  disable_filter_evt_mil(pMILPiggy);
  clear_filter_evt_mil(pMILPiggy);
  /* set filter to receive events 1,12,18,24,25,26,27 via fifo
   * and virtual accelerator 15                                  
   *
   * set event filter to trigger lemo 2 at event 12
   * 
   * set event filter to gate lemo  at events 26,27          */
  set_filter_evt_mil(pMILPiggy,  1, 15, 0x1);
  set_filter_evt_mil(pMILPiggy, 12, 15, 0x11);
  set_filter_evt_mil(pMILPiggy, 18, 15, 0x1);
  set_filter_evt_mil(pMILPiggy, 24, 15, 0x1);
  set_filter_evt_mil(pMILPiggy, 25, 15, 0x1);
  set_filter_evt_mil(pMILPiggy, 26, 15, 0x5);
  set_filter_evt_mil(pMILPiggy, 27, 15, 0x9);
  /* configure lemo 2 to generate pulses                     */
  enable_lemo_pulse_evt_mil(pMILPiggy, 2);
  /* configure lemo 1 to generate gates                      */
  enable_lemo_gate_evt_mil(pMILPiggy, 1);
  /* clear event fifo and enable filters                     */
  clear_fifo_evt_mil(pMILPiggy);
  enable_filter_evt_mil(pMILPiggy);


  /* software listener and handler                           */
  MILEvtHandler();

} /* main */
