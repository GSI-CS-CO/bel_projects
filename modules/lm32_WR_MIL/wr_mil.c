/********************************************************************************************
 *  example.c
 *
 *  created : 2017
 *  author  : Dietrich Beck, Mathias Kreider GSI-Darmstadt
 *  version : 23-Mar-2017
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
#include "aux.h"
#include "dbg.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA controle
#include "../../ip_cores/saftlib/drivers/eca_flags.h"
#define  MY_ECA_TAG      0x4 //just define a tag for ECA actions we want to receive

/* register maps for some selected Wishbone devices  */
#include "../../tools/wb_slaves.h" /* this is a hack */
#include "../../top/gsi_scu/scu_mil.h"

/* shared memory map for communication via Wishbone  */
#include "example_smmap.h"


#define BUILDID_OFFS 0x100 // location in linker script (ram.ld) (do I need this?)
#define SHARED_OFFS  0x500 // location in linker script (ram.ld) (do I need this?)

/* stuff required for environment */
extern uint32_t*       _startshared[];
unsigned int cpuId, cpuQty;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;


volatile uint32_t *pECAQ;           // WB address of ECA queue
volatile uint32_t *pECACtrl;        // WB address of ECA control
volatile uint32_t *pShared;         // pointer to begin of shared memory region
volatile uint32_t *pSharedCounter;  // pointer to a "user defined" u32 register; here: publish counter
volatile uint32_t *pSharedInput;    // pointer to a "user defined" u32 register; here: get input from host system 
volatile uint32_t *pSharedCmd;      // pointer to a "user defined" u32 register; here: get commnand from host system
volatile uint32_t *pCpuRamExternal; // external address (seen from host bridge) of this CPU's RAM
volatile uint32_t *pMILPiggy;       // WB address of MIL piggy on SCU

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

void init()
{
  discoverPeriphery();    // mini-sdb: get info on important Wishbone infrastructure, such as (this) CPU, flash, ...
  uart_init_hw();         // init UART, required for printf... . To view print message, you may use 'eb-console' from the host
  cpuId = getCpuIdx();    // get ID of THIS CPU
  isr_table_clr();        // set MSI IRQ handler
  irq_set_mask(0x01);     // ...
  irq_disable();          // ...
} // init


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


/***********************************************************
 *  
 * demonstrate how to talk to a SoC Wishbone device
 * here: get White Rabbit time from WR PPS GEN
 *
 ***********************************************************/
void getWishboneTAI()
{
  uint32_t *pPPSGen;   // WB address of PPS_GEN
  uint32_t taiSecs;    // TAI full seconds     
  uint32_t taiNsecs;   // TAI nanoseconds part 

  // find Wishbone address of WR PPS GEN                    
  pPPSGen   = find_device_adr(WR_PPS_GEN_VENDOR, WR_PPS_GEN_PRODUCT);

  // get data from WR PPS GEN and print to UART
  taiSecs  = *(pPPSGen + (WR_PPS_GEN_CNTR_UTCLO >> 2));
  taiNsecs = *(pPPSGen + (WR_PPS_GEN_CNTR_NSEC >> 2));

  //print TAI to UART
  mprintf("TAI: %08u.%09u\n", taiSecs, taiNsecs);

} // getWishboneTAI

/*************************************************************
* 
* demonstrate how to poll actions ("events") from ECA
* HERE: get WB address of relevant ECA queue
*
**************************************************************/
void findECAQ()
{
#define ECAQMAX           4         //  max number of ECA queues
#define ECACHANNELFORLM32 2         //  this is a hack! suggest to implement proper sdb-records with info for queues

  // stuff below needed to get WB address of ECA queue 
  sdb_location ECAQ_base[ECAQMAX]; // base addresses of ECA queues
  uint32_t ECAQidx = 0;            // max number of ECA queues in the SoC
  uint32_t *tmp;                
  uint32_t i;

  pECAQ = 0x0; //initialize Wishbone address for LM32 ECA queue

  // get Wishbone addresses of all ECA Queues
  find_device_multi(ECAQ_base, &ECAQidx, ECAQMAX, ECA_QUEUE_SDB_VENDOR_ID, ECA_QUEUE_SDB_DEVICE_ID);

  // walk through all ECA Queues and find the one for the LM32
  for (i=0; i < ECAQidx; i++) {
    tmp = (uint32_t *)(getSdbAdr(&ECAQ_base[i]));  
    if ( *(tmp + (ECA_QUEUE_QUEUE_ID_GET >> 2)) == ECACHANNELFORLM32) pECAQ = tmp;
  }
  
  mprintf("\n");
  if (!pECAQ) { mprintf("FATAL: can't find ECA queue for lm32, good bye! \n"); while(1) asm("nop"); }
  mprintf("ECA queue found at: 0x%08x. Waiting for actions with flag 0x%08x ...\n", pECAQ, MY_ECA_TAG);
  mprintf("\n");

} // findECAQ

uint64_t getECATAI(uint32_t *timeHi, uint32_t *timeLo) // get TAI from local ECA
{
  volatile uint32_t *pECATimeHi = (pECACtrl + (ECA_TIME_HI_GET >> 2));
  volatile uint32_t *pECATimeLo = (pECACtrl + (ECA_TIME_LO_GET >> 2));
  uint32_t timeHi2;
  uint64_t time;

  do {
    *timeHi = *pECATimeHi;
    *timeLo = *pECATimeLo;
    timeHi2 = *pECATimeHi;      // read high word again to check for overflow
  } while (*timeHi != timeHi2); // repeat until high time is consistent

  // 64-bit return value for convenience
  time   = *timeHi;
  time <<= 32;
  time  |= *timeLo;
  return time;
} 


// write an event to the device bus. The SCU dev bus output (sub-D) will be connected to the "yellow box"
// that forwards the signal through a Lemo event bus cable to a TIF module
int16_t writeEvtMil(volatile uint32_t *base, uint8_t  funcPz, uint8_t virtAcc, uint16_t evtNr)
{
  uint32_t telegram = 0; // 32 bit write to mil macro, upper bits [31:16] don't care
  telegram |= funcPz  << 12;
  telegram |= virtAcc <<  8;
  telegram |= evtNr;

  if (trm_free(base) == OKAY) {
    base[MIL_WR_CMD] = telegram;
  } else {
    return TRM_NOT_FREE;
  }
  return OKAY;
}

// void usleep(int us)
// {

//   unsigned i;
//   unsigned long long delay = us;
//   /* prevent arithmetic overflow */
//   delay *= CPU_CLOCK;
//   delay /= 1000000;
//   delay /= 4; // instructions per loop
//   for (i = delay; i > 0; i--) asm("# noop");
// }  

// send EVT_START_CYCLE ...
//    ... wait for some us and ...
// ... send EVT_END_CYCLE.
int16_t writeEvtMilCycle(volatile uint32_t *base)
{

  if (trm_free(base) == OKAY) {
    base[MIL_WR_CMD] = 0x32;
  } else {
    return TRM_NOT_FREE;
  }

  usleep(10);

  if (trm_free(base) == OKAY) {
    base[MIL_WR_CMD] = 0x55;
  } else {
    return TRM_NOT_FREE;
  }


  return OKAY;
}

/*************************************************************
* 
* demonstrate how to poll actions ("events") from ECA
* HERE: poll ECA, get data of action and do something
*
* This example assumes that
* - action for this lm32 are configured by using saft-ecpu-ctl
*   from the host system
* - a TAG with value 0x4 has been configure (see saft-ecpu-ctl -h
*   for help
*
**************************************************************/
void ecaHandler()
{
  uint32_t flag;                // flag for the next action
  uint32_t evtIdHigh;           // high 32bit of eventID
  uint32_t evtIdLow;            // low 32bit of eventID
  uint32_t evtDeadlHigh;        // high 32bit of deadline
  uint32_t evtDeadlLow;         // low 32bit of deadline
  uint64_t evtDeadl;
  uint32_t actTag;              // tag of action
  uint32_t evtNo;               // EVTNO as extracted from EventID field
  uint32_t evtCode;             // event code for MIL
  uint32_t virtAcc;             // virtual accelerator number for MIL
  uint32_t ecaTimeHi, ecaTimeLo;// time from eca registers
  uint64_t ecaTime; 
  uint32_t milTelegram    = 0;  // translated information for MIL event bus
  uint16_t milHighCurrent = 1;  // flags that go into the most significant 4 bits of the mil Telegram
  uint16_t milNoBeam      = 0;
  uint16_t milRigidBeam   = 1;
  uint16_t milUnused      = 0; 

  // read flag and check if there was an action 
  flag         = *(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2));
  if (flag & (0x0001 << ECA_VALID)) { 
    // read data 
    evtIdHigh    = *(pECAQ + (ECA_QUEUE_EVENT_ID_HI_GET >> 2));
    evtIdLow     = *(pECAQ + (ECA_QUEUE_EVENT_ID_LO_GET >> 2));
    evtDeadlHigh = *(pECAQ + (ECA_QUEUE_DEADLINE_HI_GET >> 2));
    evtDeadlLow  = *(pECAQ + (ECA_QUEUE_DEADLINE_LO_GET >> 2));
    actTag       = *(pECAQ + (ECA_QUEUE_TAG_GET >> 2));

    // get EventNo and SID from EventId
    evtNo        = (evtIdHigh>>4)&0x00000fff;
    evtCode      = evtNo         &0x000000ff;
    virtAcc      = (evtIdLow>>24)&0x0000000f;
    
    // pop action from channel
    *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;

    // here: do s.th. according to action
    switch (actTag) {
    case MY_ECA_TAG:
      mprintf("EvtID: 0x%08x%08x; deadline: 0x%08x%08x; flag: 0x%08x\n", evtIdHigh, evtIdLow, evtDeadlHigh, evtDeadlLow, flag);
      evtDeadl   = evtDeadlHigh;
      evtDeadl <<= 32;
      evtDeadl  |= evtDeadlLow;

      mprintf("EvtNo: 0x%08x\n", evtNo);
      if ((evtNo&0x00000f00) == 0) // if evtNo in MIL-range [0..255]
      {
        //mprintf("timing event is for MIL event bus evnt_code=0x%08x virt.Acc=0x%08x\n", evtCode, virtAcc);
        do {
          ecaTime = getECATAI(&ecaTimeHi,&ecaTimeLo);
        } while(ecaTime < evtDeadl);
        mprintf("deliver NOW\n");

      //   milTelegram  = virtAcc<<8;
        milTelegram |= evtCode;
      //   milTelegram |= milHighCurrent << 15;
      //   milTelegram |= milNoBeam      << 15;
      //   milTelegram |= milRigidBeam   << 15;
      //   milTelegram |= milUnused      << 15;



      //   mprintf("MIL event telegram: 0x%08x\n", milTelegram);
      //   while (ecaTime < evtDeadl); // wait ...
        mprintf("eca time is : 0x%08x%08x\n", ecaTimeHi, ecaTimeLo);
        //writeEvtMil(pMILPiggy, 0, 1, 32);
        writeEvtMilCycle(pMILPiggy);

      }

      break;
    default:
      mprintf("ecaHandler: unknown tag\n");
    } // switch

  } // if data is valid
} // ecaHandler

void findEcaControl() // find WB address of ECA Control
{
  pECACtrl  = find_device_adr(ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID);
}

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

  busStatus = writeDevMil(pMILPiggy, ifbAddr, FC_WR_IFC_ECHO, wData);
  if (busStatus != MIL_STAT_OK) mprintf("echo test on MIL: ERROR\n");

  busStatus = readDevMil(pMILPiggy, ifbAddr, FC_RD_IFC_ECHO, &rData);
  if (busStatus != MIL_STAT_OK) mprintf("echo test on MIL: ERROR\n");

  if (wData != rData)  mprintf("echo test on MIL: ERROR\n");
  else                 mprintf("echo test on MIL: OK\n");

  // please note, that a dedicated funtion is already implemented in scu_mil.h(c)
  // busStatus = echoTestDevMil(pMILPiggy, ifbAddr, wData);
} // testEchoMILDevice

void main(void) {
  uint8_t  ifbAddr = 0x20;  // address of MIL interface board  
  uint32_t i,j;
  int16_t busStatus;
  
  init();   // initialize 'boot' lm32
  
  for (j = 0; j < (31000000); ++j) { asm("nop"); }     // wait 1 second
  mprintf("Hello World!\n");                           // print message to UART

  getWishboneTAI();  // get TAI via WB and print to UART
  findECAQ();        // find ECA channel for LM32
  findMILPiggy();
  findEcaControl();
  // mprintf("write to MIL\n");                           // print message to UART
  // busStatus = writeDevMil(pMILPiggy, ifbAddr, FC_WR_IFC_ECHO, 0xaffe);
  // mprintf("MIL busStatus: %d (OKAY = %d)\n", busStatus, OKAY);

  i=0;
  while (1) {
    // do the things that have to be done
    ecaHandler();

    // increment and update iteration counter
    i++;
    *pSharedCounter = i;

    // wait for 100  microseconds 
    for (j = 0; j < (3200); ++j) { asm("nop"); }
  } // while
} /* main */
