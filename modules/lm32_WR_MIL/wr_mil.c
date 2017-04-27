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
#include "wr_mil_tai.h"

#define  MY_ECA_TAG      0x4 //just define a tag for ECA actions we want to receive

/* register maps for some selected Wishbone devices  */
#include "../../tools/wb_slaves.h" /* this is a hack */
#include "../../top/gsi_scu/scu_mil.h"

/* shared memory map for communication via Wishbone  */
#include "example_smmap.h"

/* stuff required for environment */
extern uint32_t*       _startshared[];
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


uint32_t delta1, delta2, delta3;

uint64_t evtDeadl; // global for now...
TAI_t   evtDeadl2; // a union of a 64bit value and 2 32bit values
// send EVT_START_CYCLE ...
//    ... wait for some us and ...
// ... send EVT_END_CYCLE.
int16_t writeEvtMilCycle(volatile uint32_t *base)
{
    //int j;
    //for (j = 0; j < (waitingtime); ++j) { asm("nop"); }
  //if (trm_free(base) == OKAY) {
//    base[MIL_WR_CMD] = 32;
//  } else {
//    return TRM_NOT_FREE;
//  }
    base[MIL_WR_CMD] = 55;

    volatile int64_t now  ;
    volatile int64_t then ;
    volatile int64_t delta;
    TAI_t ecaTime;
    evtDeadl2.value += 30000;
    //do {
      //getECATAI(&ecaTime);
    GET_ECA_TAI(ecaTime);
    delta   = evtDeadl2.value - ecaTime.value; // delay in ns
    delta >>= 4;                             // delay in tics (divide by 16)
    if (delta > 0)  delay_96plus32n_ns(delta);

    //} while(ecaTime.value < evtDeadl2.value);

    base[MIL_WR_CMD] = 32;
    delta1 = delta;

    for (int i = 0; i < 10; ++i)
    {
      evtDeadl2.value += 30000;
    //  do {
        //getECATAI(&ecaTime);
        GET_ECA_TAI(ecaTime);
        delta   = evtDeadl2.value - ecaTime.value; // delay in ns
        delta >>= 4;                             // delay in tics (divide by 16)
        if (delta > 0) delay_96plus32n_ns(delta);
  //    } while(ecaTime.value < evtDeadl2.value);
      base[MIL_WR_CMD] = 0x22;
    }

    evtDeadl2.value += 30000;
    do {
      //getECATAI(&ecaTime);
      GET_ECA_TAI(ecaTime);
      //  delta   = evtDeadl2.value - ecaTime.value; // delay in ns
      //  delta >>= 4;                             // delay in tics (divide by 16)
      //  delay_96plus32n_ns(delta);
    } while(ecaTime.value < evtDeadl2.value);

    base[MIL_WR_CMD] = 55;
    delta3 = delta;
//  } else {
//    return TRM_NOT_FREE;
//  }

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
  //uint64_t evtDeadl;
  uint32_t actTag;              // tag of action
  uint32_t evtNo;               // EVTNO as extracted from EventID field
  uint32_t evtCode;             // event code for MIL
  uint32_t virtAcc;             // virtual accelerator number for MIL
  uint32_t ecaTimeHi, ecaTimeLo;// time from eca registers
  TAI_t ecaTime, ecaTime_old; 

  // read flag and check if there was an action 
  flag         = *(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2));
  if (flag & (0x0001 << ECA_VALID)) { 
    // read data 
    evtIdHigh         = *(pECAQ + (ECA_QUEUE_EVENT_ID_HI_GET >> 2));
    evtIdLow          = *(pECAQ + (ECA_QUEUE_EVENT_ID_LO_GET >> 2));
    evtDeadl2.part.hi = *(pECAQ + (ECA_QUEUE_DEADLINE_HI_GET >> 2));
    evtDeadl2.part.lo = *(pECAQ + (ECA_QUEUE_DEADLINE_LO_GET >> 2));
    actTag            = *(pECAQ + (ECA_QUEUE_TAG_GET >> 2));

    // get EventNo and SID from EventId
    evtNo        = (evtIdHigh>>4)&0x00000fff;
    evtCode      = evtNo         &0x000000ff;
    virtAcc      = (evtIdLow>>24)&0x0000000f;
    
    //mprintf("pop from eca queue\n");
    // pop action from channel
    *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;

    // here: do s.th. according to action
    switch (actTag) {
    case MY_ECA_TAG:
      //mprintf("EvtID: 0x%08x%08x; deadline: 0x%08x%08x; flag: 0x%08x\n", evtIdHigh, evtIdLow, evtDeadlHigh, evtDeadlLow, flag);

      // evtDeadl   = evtDeadlHigh;
      // evtDeadl <<= 32;
      // evtDeadl  |= evtDeadlLow;
      // evtDeadl2.part.hi = evtDeadlHi;
      // evtDeadl2.part.lo = evtDeadlLow;

      //mprintf("hi %08x lo %08x    my hi %08x lo %08x    my hi %08x lo %08x\n", evtDeadlHigh, evtDeadlLow, evtDeadl2.pos[0], evtDeadl2.pos[1], evtDeadl2.part.hi, evtDeadl2.part.lo );

      //evtDeadl2.value += 10000; // add 10 us

      //if ((evtNo&0x00000f00) == 0) // if evtNo in MIL-range [0..255]
      {
        //mprintf("timing event is for MIL event bus evnt_code=0x%08x virt.Acc=0x%08x\n", evtCode, virtAcc);
        //int32_t attempts = 0;
        //int32_t loops = 0;
        //do {
          //++loops;
          //asm("nop");
          //getECATAI(&ecaTime.part.hi, &ecaTime.part.lo, &attempts);

          // {                                                                             
          //   volatile uint32_t *pECATimeHi = (pECACtrl + (ECA_TIME_HI_GET >> 2));        
          //   volatile uint32_t *pECATimeLo = (pECACtrl + (ECA_TIME_LO_GET >> 2));        
          //   uint32_t timeHi2;                                                           
          //   do {                                                                        
          //     ++(attempts);                                                             
          //     ecaTime.part.hi  = *pECATimeHi;                                                    
          //     ecaTime.part.lo  = *pECATimeLo;                                                    
          //     timeHi2          = *pECATimeHi;      // read high word again to check for overflow 
          //   } while (ecaTime.part.hi != timeHi2); // repeat until high time is consistent        
          // }                                                                             

          //mprintf("eca time is (after %d attempts): 0x%08x%08x   dt=%d\n", attempts, ecaTimeHi, ecaTimeLo, dt);
        //} while(ecaTime.value < evtDeadl2.value); // quit the loop at least 7us before the deadline
        //int64_t t_remaining = evtDeadl-ecaTime; // remaining time
        //for (int32_t q = t_remaining; q > 0; --q) asm("nop");
        //mprintf("t_rem=%d\n",t_remaining);
        //mprintf("deliver NOW\n");

        //int64_t now   = ecaTime.value;
        //int64_t then  = evtDeadl2.value;
        //if (now >= then) break;
        //int32_t delta = then-now;
        //precise_delay(delta);
    //TAI_t ecaTime;
    pMILPiggy[MIL_WR_CMD] = 32;

    //GET_ECA_TAI(evtDeadl2);

    for (int i = 0; i < 50; ++i)
    {
      //evtDeadl2.value += 200000;
      //GET_ECA_TAI(ecaTime);
      //int32_t delta = evtDeadl2.value - ecaTime.value;
      //delta >>= 4; // delay in tics (divide by 16)
      //if (delta > 0)  
      //  delay_96plus32n_ns(delta);
      //DELAY100us;
      delay_96plus32n_ns(3122-9);
      pMILPiggy[MIL_WR_CMD] = 0x22;
    }
    DELAY50us;
    pMILPiggy[MIL_WR_CMD] = 55;
    DELAY1000us;


        //writeEvtMilCycle(pMILPiggy);


        //mprintf("deltas %d %d %d\n", delta1, delta2, delta3);
        // getECATAI(&ecaTime.part.hi, &ecaTime.part.lo);
        // delta1 = ecaTime.value - ecaTime_old.value;
        // ecaTime_old = ecaTime;
        // precise_delay(30000);
        // getECATAI(&ecaTime.part.hi, &ecaTime.part.lo);
        // ecaTime_old = ecaTime;
        // getECATAI(&ecaTime.part.hi, &ecaTime.part.lo);
        // delta1 = ecaTime.value - ecaTime_old.value;
        // ecaTime_old = ecaTime;
        // getECATAI(&ecaTime.part.hi, &ecaTime.part.lo);
        // delta2 = ecaTime.value - ecaTime_old.value;
        // ecaTime_old = ecaTime;
        // getECATAI(&ecaTime.part.hi, &ecaTime.part.lo);
        // delta3 = ecaTime.value - ecaTime_old.value;
        // ecaTime_old = ecaTime;

        mprintf("deltas %d %d %d\n", delta1, delta2, delta3);



      }

      break;
    default:
      mprintf("ecaHandler: unknown tag\n");
    } // switch

  } // if data is valid
} // ecaHandler




void main(void) {
  
  uint32_t i,j;
  uint64_t k = 100000ll;
  
  init();   // initialize 'boot' lm32


  for (j = 0; j < (31000000); ++j) { asm("nop"); }     // wait 1 second
  mprintf("Hello World!\n");                           // print message to UART


  getWishboneTAI();  // get TAI via WB and print to UART
  findECAQ();        // find ECA channel for LM32
  findMILPiggy();
  findEcaControl();

  while(1) {
    uint32_t flag = *(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2));
    if (flag & (0x0001 << ECA_VALID)) {
      //mprintf("pop\n");
      *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;
    }
    else break;
  }

  pMILPiggy[MIL_REG_WR_RF_LEMO_CONF>>2] |= (1<<MIL_LEMO_OUT_EN1) | (1<<MIL_LEMO_OUT_EN2);

  i=0;
  while (1) {


    // do the things that have to be done
    //ecaHandler();
    pMILPiggy[MIL_REG_WR_RD_LEMO_DAT>>2] |= (1<<MIL_LEMO_DAT1) | (1<<MIL_LEMO_DAT2);
    DELAY100us;
    pMILPiggy[MIL_REG_WR_RD_LEMO_DAT>>2] &= ~((1<<MIL_LEMO_DAT1) | (1<<MIL_LEMO_DAT2));
    DELAY100us;


    // increment and update iteration counter
    //i++;
    //*pSharedCounter = i;

    // wait for 100  microseconds 
    if (k) --k;
    else  {
      mprintf("program done!");
      while(1) asm("nop"); // do nothing forerver
    } 
  } // while
} /* main */
