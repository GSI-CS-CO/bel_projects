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
#include "milExample_shared_mmap.h"

/* stuff required for environment */
extern uint32_t*       _startshared[];
unsigned int cpuId, cpuQty;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;

volatile uint32_t *pMILPiggy;    // WB address of MIL piggy on SCU
volatile uint32_t *pShared;      // pointer to begin of shared memory region
uint32_t *pCpuRamExternal;       // external address (seen from host bridge) of this CPU's RAM

#define MILSLOT 0                // SCU bus slot, 0: MILPiggy, 1..N: SIO in slot 1..N

void init()
{
  discoverPeriphery();   // mini-sdb: get info on important Wishbone infrastructure, such as (this) CPU, flash, EB Master, ...
  uart_init_hw();        // init UART, required for printf... . To view print message, you may use 'eb-console' from the host
  cpuId = getCpuIdx();   // get ID of THIS CPU
  isr_table_clr();       // set MSI IRQ handler
  irq_set_mask(0x01);
  irq_disable(); 
} // init


/***********************************************************
 *
 * find shared memory and print start addresses from perspectives
 * of lm32 and host bridge
 *
 ***********************************************************/
void findSharedMem()
{
  uint32_t       i,j;
  uint32_t       idx;
  const uint32_t c_Max_Rams = 10;
  sdb_location   found_sdb[c_Max_Rams];
  sdb_location   found_clu;

  // get pointer to shared memory  from lm32 perspective
  pShared        = (uint32_t *)_startshared;       
  mprintf("internal shared memory: start            @ 0x%08x\n", (uint32_t)pShared);

  // get pointer to shared memory from the external perspective
  idx = 0;
  find_device_multi(&found_clu, &idx, 1, GSI, LM32_CB_CLUSTER);	
  idx = 0;
  find_device_multi_in_subtree(&found_clu, &found_sdb[0], &idx, c_Max_Rams, GSI, LM32_RAM_USER);
  if(idx >= cpuId) {
    pCpuRamExternal = (uint32_t*)(getSdbAdr(&found_sdb[cpuId]) & 0x7FFFFFFF); // CPU sees the 'world' under 0x8..., remove that bit to get host bridge perspective 
    mprintf("external WB address   : start           @ 0x%08x\n", (uint32_t)(pCpuRamExternal));
  } else {
    pCpuRamExternal = (uint32_t*)ERROR_NOT_FOUND;
    mprintf("Could not find external WB address of my own RAM !\n");
  } //if,else
} // findSharedMem


/*************************************************************
* 
* demonstrate how to talk to MIL devicebus and receive MIL events
* HERE: get WB address of Wishbone device GSI_MIL_SCU "MIL Piggy"
*
**************************************************************/
void findMILPiggy(){
  pMILPiggy   = find_device_adr(GSI, SCU_MIL);   // get Wishbone address for MIL piggy 
  mprintf("pMILPiggy: 0x%08x\n",  pMILPiggy);
} // findMILPiggy


/*************************************************************
* 
* demonstrate how to get MIL Events of the MIL piggy FIFO
* and how to handle them
*
**************************************************************/
void MILEvtHandler(uint32_t nOfEvents){
  uint32_t evtData;            // data of one MIL event
  uint32_t evtCode;            //"event number"
  uint32_t virtAcc;            // virtual accelerator
  uint32_t beamStat;           // beam status               
  uint32_t evtTime, evtTime1, evtTime2, evtTime3; // EVT_TIME (time in 10ms since UTC 0000 current day)
  uint32_t secsUTC, msecsUTC, UTC1, UTC2, UTC3, UTC4, UTC5; //TIME_UTC, time since 1 Jan 2008
  uint32_t help1, help2; 
  int i;

  i = 0;
  while (i < 500) {
    if (fifoNotemptyEvtMil(pMILPiggy, MILSLOT)) {
      i++;
      popFifoEvtMil(pMILPiggy, MILSLOT, &evtData);
      evtCode  = evtData & 0x000000ff;
      help1   = (evtData >> 8);
      switch (evtCode) {
      case 0 ... 199:           // 000..199: timing events
        virtAcc  = help1 & 0x0f;
        beamStat = help1 >> 4;
        mprintf("EVENT - code: %03u, virtAcc: %02u, stat: %02u\n", evtCode, virtAcc, beamStat);
        break;
      case 209:                 // 209..211: time in 10ms of the current day
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
        mprintf("------TIME: %09u [10ms ticks since midnight]\n", evtTime);
        break;
      case 224:                 // 224..228: "UTC" time starting 1 Jan 2008
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
      } // switch
    } // if regValue
  } // while
} // MILEvtHandler

/*************************************************************
* 
* demonstrate how to talk to a MIL device
* HERE: write (read) data to (from)  the echo register of 
* a MIL device
*
**************************************************************/
void echoTestMILDevice(uint16_t wData, uint8_t ifbAddr)
{
  int16_t  busStatus;
  uint16_t rData = 0x0;

  busStatus = writeDevMil(pMILPiggy, MILSLOT, ifbAddr, FC_WR_IFC_ECHO, wData);
  if (busStatus != MIL_STAT_OK) mprintf("echo test on MIL: ERROR\n");

  busStatus = readDevMil(pMILPiggy, MILSLOT, ifbAddr, FC_RD_IFC_ECHO, &rData);
  if (busStatus != MIL_STAT_OK) mprintf("echo test on MIL: ERROR\n");

  if (wData != rData)  mprintf("echo test on MIL: ERROR\n");
  else                 mprintf("echo test on MIL: OK\n");

  // please note, that a dedicated funtion is already implemented in scu_mil.h(c)
  // busStatus = echoTestDevMil(pMILPiggy, ifbAddr, wData);
 } // testEchoMILDevice

void main(void) {

  uint8_t  ifbAddr = 0x20;                         // address of MIL interface board
  uint32_t i,j;
  
  init();                                          // initialize 'boot' lm32
 
  for (j = 0; j < (32000000); ++j) { asm("nop"); } // wait 1 second 
  mprintf("Hello World!\n");                       // and print initial message to UART

  findSharedMem();                                 // retrieve address of shared memory
  
  findMILPiggy();                                  // find Wishbone address of MIL piggy on SCU     
  echoTestMILDevice(0xbabe, ifbAddr);              // write/read to echo register of MIL device

  writeCtrlStatRegEvtMil(pMILPiggy, MILSLOT, MIL_CTRL_STAT_ENDECODER_FPGA | MIL_CTRL_STAT_INTR_DEB_ON);   // write initial values to control register

  disableFilterEvtMil(pMILPiggy, MILSLOT);         // disable event filtering

  clearFilterEvtMil(pMILPiggy, MILSLOT);                                                      // clear filter RAM by setting all filters to 0x0
  setFilterEvtMil(pMILPiggy, MILSLOT,  1, 15, MIL_FILTER_EV_TO_FIFO);                         // filter on event code 1 and accelerator 15; FIFO
  setFilterEvtMil(pMILPiggy, MILSLOT, 12, 15, MIL_FILTER_EV_TO_FIFO | MIL_FILTER_EV_PULS2_S); // filter on event code 12 and accelerator 15; FIFO and pulse on LEMO2
  configLemoPulseEvtMil(pMILPiggy, MILSLOT, 2);                                               // configure LEMO2 for pulsing mode
  setFilterEvtMil(pMILPiggy, MILSLOT, 18, 15, MIL_FILTER_EV_TO_FIFO);                         // filter on event code 1 and accelerator 15; FIFO
  setFilterEvtMil(pMILPiggy, MILSLOT, 24, 15, MIL_FILTER_EV_TO_FIFO);                         // filter on event code 1 and accelerator 15; FIFO
  setFilterEvtMil(pMILPiggy, MILSLOT, 25, 15, MIL_FILTER_EV_TO_FIFO);                         // filter on event code 1 and accelerator 15; FIFO
  setFilterEvtMil(pMILPiggy, MILSLOT, 26, 15, MIL_FILTER_EV_TO_FIFO);                         // filter on event code 1 and accelerator 15; FIFO
  setFilterEvtMil(pMILPiggy, MILSLOT, 27, 15, MIL_FILTER_EV_TO_FIFO | MIL_FILTER_EV_PULS1_S); // filter on event code 1 and accelerator 15; FIFO and pulse on LEMO1
  configLemoPulseEvtMil(pMILPiggy, MILSLOT, 1);                                               // configure LEMO1 for pulsing mode

  clearFifoEvtMil(pMILPiggy, MILSLOT);             // clear the FIFO to remove possible junk
  enableFilterEvtMil(pMILPiggy, MILSLOT);          // enable filtering 

  MILEvtHandler(100);
} /* main */
