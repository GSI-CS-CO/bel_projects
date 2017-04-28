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

/* local includes for wr_mil firmware*/
#include "wr_mil_value64bit.h"
#include "wr_mil_piggy.h"
#include "wr_mil_eca_queue.h"

#define  MY_ECA_TAG      0x4 //just define a tag for ECA actions we want to receive

/* register maps for some selected Wishbone devices  */
#include "../../tools/wb_slaves.h" /* this is a hack */
#include "../../top/gsi_scu/scu_mil.h"

/* shared memory map for communication via Wishbone  */
#include "example_smmap.h"

/* stuff required for environment */
//extern uint32_t*       _startshared[];
extern void* _startshared; // provided in linker script "ram.ld"
unsigned int cpuId, cpuQty;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;


volatile uint32_t *pECAQ;           // WB address of ECA queue
volatile uint32_t *pECACtrl;        // WB address of ECA control
volatile uint32_t *pECATimeHi;
volatile uint32_t *pECATimeLo;
volatile uint32_t *pShared;         // pointer to begin of shared memory region
volatile uint32_t *pSharedCounter;  // pointer to a "user defined" u32 register; here: publish counter
volatile uint32_t *pSharedInput;    // pointer to a "user defined" u32 register; here: get input from host system 
volatile uint32_t *pSharedCmd;      // pointer to a "user defined" u32 register; here: get commnand from host system
volatile uint32_t *pCpuRamExternal; // external address (seen from host bridge) of this CPU's RAM
volatile uint32_t *pMILPiggy;       // WB address of MIL piggy on SCU

uint32_t waitingtime = 2000;



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

/*************************************************************
* 
* demonstrate how to poll actions ("events") from ECA
* HERE: get WB address of relevant ECA queue
*
**************************************************************/
uint32_t *new_findECAQ()
{
#define ECAQMAX           4         //  max number of ECA queues
#define ECACHANNELFORLM32 2         //  this is a hack! suggest to implement proper sdb-records with info for queues

  // stuff below needed to get WB address of ECA queue 
  sdb_location ECAQ_base[ECAQMAX]; // base addresses of ECA queues
  uint32_t ECAQidx = 0;            // max number of ECA queues in the SoC
  uint32_t *tmp;                
  uint32_t i;

  uint32_t *pECAQ = 0x0; //initialize Wishbone address for LM32 ECA queue

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
  return pECAQ;
} // findECAQ


void findEcaControl() // find WB address of ECA Control
{
  pECACtrl  = find_device_adr(ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID);
  pECATimeHi = (pECACtrl + (ECA_TIME_HI_GET >> 2));
  pECATimeLo = (pECACtrl + (ECA_TIME_LO_GET >> 2));

  mprintf("pECACtrl: 0x%08x\n",  pECACtrl);
}

// void getECATAI(uint32_t *timeHi, uint32_t *timeLo) // get TAI from local ECA
// {
//   volatile uint32_t *pECATimeHi = (pECACtrl + (ECA_TIME_HI_GET >> 2));
//   volatile uint32_t *pECATimeLo = (pECACtrl + (ECA_TIME_LO_GET >> 2));
//   uint32_t timeHi2;
//   uint64_t time;

//   do {
//     *timeHi = *pECATimeHi;
//     *timeLo = *pECATimeLo;
//     timeHi2 = *pECATimeHi;      // read high word again to check for overflow
//   } while (*timeHi != timeHi2); // repeat until high time is consistent
// } 
void getECATAI(TAI_t *time) // get TAI from local ECA
{
  do {
    time->part.hi = *pECATimeHi;
    time->part.lo = *pECATimeLo;
  } while (time->part.hi != *pECATimeHi); // repeat until high time is consistent
} 

#define GET_ECA_TAI(time) {\
do{\
  time.part.hi=*pECATimeHi;\
  time.part.lo=*pECATimeLo;\
}while(time.part.hi!=*pECATimeHi);\
}

void delay_96plus32n_ns(uint32_t n)
{
  while(n--) asm("nop"); // if "nop" is missing, compiler will optimize the loop away
}
#define DELAY5us    delay_96plus32n_ns(153)
#define DELAY10us   delay_96plus32n_ns(310)
#define DELAY20us   delay_96plus32n_ns(622)
#define DELAY40us   delay_96plus32n_ns(1247)
#define DELAY50us   delay_96plus32n_ns(1560)
#define DELAY100us  delay_96plus32n_ns(3122)
#define DELAY1000us delay_96plus32n_ns(31220)


void newECAHandler(volatile ECAQueueRegs *eca_queue, volatile MilPiggyRegs *mil_piggy)
{
  if (ECAQueue_getFlags(eca_queue) & (1<<ECA_VALID))
  {
    EvtId_t evtId = { 
      .part.hi = eca_queue->event_id_hi_get,
      .part.lo = eca_queue->event_id_lo_get
    };
    TAI_t evtDeadl = { 
      .part.hi = eca_queue->deadline_hi_get,
      .part.lo = eca_queue->deadline_lo_get
    };
    switch(eca_queue->tag_get)
    {
      case MY_ECA_TAG:
        mil_piggy->wr_cmd = 32;
        for (int i = 0; i < 50; ++i)
        {
          DELAY50us;
          mil_piggy->wr_cmd = 0x22;
        }
        DELAY50us;
        mil_piggy->wr_cmd = 55;
        DELAY1000us;      
      break;
      default:
        mprintf("ecaHandler: unknown tag\n");
      break;
    }
    uint32_t actTag = ECAQueue_getActTag(eca_queue);
  }
}


void main(void) {
  
  uint32_t i,j;
  uint64_t k = 10000ll;
  
  init();   // initialize 'boot' lm32

  for (j = 0; j < (31000000); ++j) { asm("nop"); }     // wait 1 second
  mprintf("Hello World!\n");                           // print message to UART


  getWishboneTAI();  // get TAI via WB and print to UART

  // MilPiggy setup
  volatile MilPiggyRegs *mil_piggy = MilPiggy_init(find_device_adr(GSI, SCU_MIL));
  MilPiggy_lemoOut1Enable(mil_piggy);
  MilPiggy_lemoOut2Enable(mil_piggy);
  mprintf("mil_piggy.pMILPiggy = 0x%08x\n", mil_piggy);

  // ECAQueue setup
  ECAQueueRegs *eca_queue = ECAQueue_init(new_findECAQ());
  uint32_t n_events = ECAQueue_clear(eca_queue);
  mprintf("found %d events in eca queue\n", n_events);

  i=0;
  while (1) {
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
    newECAHandler(eca_queue, mil_piggy);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);
    DELAY100us;
    // wait a bit before end all operations
    if (k) --k;
    else  {
      mprintf("program done!");
      while(1) asm("nop"); // do nothing forerver
    } 
  } 
} 
