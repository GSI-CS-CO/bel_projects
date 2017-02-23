/********************************************************************************************
 *  dm-unipz.c
 *
 *  created : 2017
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 02-Mar-2017
 *
 *  lm32 program for gateway between UNILAC Pulszentrale and FAIR-style Data Master
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
#include "../../top/gsi_scu/scu_mil.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h" // register layout ECA queue
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA controle
#include "../../ip_cores/saftlib/drivers/eca_flags.h"              // defitions for ECA queueu
#include "dm-unipz.h"

/* register maps for some selected Wishbone devices  */
// >>>>>>>>>>>>>>>>>>>>> argh ... how about just including their header files in your hack header file ?? <<<<<<<<<<<<<<<<<<<<<<<<
#include "../../tools/wb_slaves.h" /* this is a hack */

/* shared memory map for communication via Wishbone  */
#include "dm-unipz_smmap.h"

/* stuff required for environment */
extern uint32_t*       _startshared[];
unsigned int cpuId, cpuQty;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;

volatile uint32_t *pECAQ;              // WB address of ECA queue
volatile uint32_t *pECACtrl;           // WB address of ECA control
volatile uint32_t *pMILPiggy;          // WB address of MIL device bus (MIL piggy)                              
volatile uint32_t *pShared;            // pointer to begin of shared memory region                              
uint32_t *pSharedStatus;               // pointer to a "user defined" u32 register; here: publish status
uint32_t *pSharedNIterMain;            // pointer to a "user defined" u32 register; here: publish # of iterations of main loop
uint32_t *pSharedNRecMILEvt;           // pointer to a "user defined" u32 register; here: publish # of received  MIL events
volatile uint32_t *pSharedCmd;         // pointer to a "user defined" u32 register; here: get commnand from host
volatile uint32_t *pSharedData4EB ;    // pointer to a n x u32 register; here: memory region for receiving EB return values
uint32_t *pCpuRamExternal;             // external address (seen from host bridge) of this CPU's RAM            
uint32_t *pCpuRamExternalStatus;       // external address (seen from host bridge) of this CPU's RAM: status
uint32_t *pCpuRamExternalCmd;          // external address (seen from host bridge) of this CPU's RAM: command
uint32_t *pCpuRamExternalNIterMain;    // external address (seen from host bridge) of this CPU's RAM: # of iterations of main loop
uint32_t *pCpuRamExternalNRecMILEvt;   // external address (seen from host bridge) of this CPU's RAM: # of received MIL events
uint32_t *pCpuRamExternalData4EB;      // external address (seen from host bridge) of this CPU's RAM: field for EB return values


uint32_t *pRemotePPSGen;               // WB address of remote PPS_GEN 
uint32_t *pRemotePPSSecs;              // TAI full seconds             
uint32_t *pRemotePPSNsecs;             // TAI nanoseconds part         

volatile uint32_t *pLocalPPSGen;       // WB address of PPS_GEN
volatile uint32_t *pLocalPPSSecs;      // TAI full seconds     
volatile uint32_t *pLocalPPSNsecs;     // TAI nanoseconds part

uint32_t timeRecMILEvtHigh;            // TAI of received MIL event 32 high bits
uint32_t timeRecMILEvtLow;             // TAI of received MIL event 32 low bits

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


void ebmInit() // intialize Etherbone master
{
  int j;
  
  while (*(pEbCfg + (EBC_SRC_IP>>2)) == EBC_DEFAULT_IP) {
    for (j = 0; j < (125000000/2); ++j) { asm("nop"); }
    mprintf("#%02u: DM cores Waiting for IP from WRC...\n", cpuId);  
  } /* pEbCfg */
  
  ebm_init();
  ebm_config_meta(1500, 42, 0x00000000 );
  
  ebm_config_if(DESTINATION, 0x00267b000386, 0xc0a8a019,      0xebd0);           //Dst:
  //ebm_config_if(DESTINATION, 0xffffffffffff, 0xffffffff ,      0xebd0);            //Dst: broadcast DANGER!!!
  ebm_config_if(SOURCE,      0x00267b000321, *(pEbCfg + (EBC_SRC_IP>>2)), 0xebd0); //Src: MAC is a hack!, WR IP

  mprintf("my IP:  0x%08x\n",  *(pEbCfg + (EBC_SRC_IP>>2)));
  mprintf("pEbCfg: 0x%08x\n",  pEbCfg);
  ebm_clr();
} // ebminit


void ebmClearSharedMem() // clear shared memory used for EB return values
{
  uint32_t i;

  for (i=0; i< (DMUNIPZ_SHARED_DATA_4EB_SIZE >> 2); i++) pSharedData4EB[i] = 0x0;
} //ebmClearSharedMem


void init() // typical init for lm32
{
  discoverPeriphery();           // mini-sdb ...
  uart_init_hw();                
  cpuId = getCpuIdx();

  // set MSI IRQ handler
  isr_table_clr();
  irq_set_mask(0x01);
  irq_disable(); 
} // init


void findRemotePPSGen() // find WB address of remote PPSGen
{
  uint32_t      i;

  // get Wishbone address of remote  WR PPS GEN
  // could be replaced with remote detection code. Combination of modules/lm32-include/minisdb.c and  ip_cores/etherbone-core/api/tools/eb-find.c
  pRemotePPSGen  = (uint32_t *)0x8060300;   /* this is a hack    */

  // get Wishbone addresses of (nano)seconds counters
  pRemotePPSSecs  = (uint32_t *)(pRemotePPSGen + (WR_PPS_GEN_CNTR_UTCLO >> 2));
  pRemotePPSNsecs = (uint32_t *)(pRemotePPSGen + (WR_PPS_GEN_CNTR_NSEC >> 2));
} // findRemotePPSGen


void findLocalPPSGen() // find WB address of (local) PPSGen
{
  // get Wishbone address of local WR PPS GEN
  pLocalPPSGen    = find_device_adr(WR_PPS_GEN_VENDOR, WR_PPS_GEN_PRODUCT);

  // get Wishbone addresses of (nano)seconds counters
  pLocalPPSSecs   = (uint32_t *)(pLocalPPSGen + (WR_PPS_GEN_CNTR_UTCLO >> 2));
  pLocalPPSNsecs  = (uint32_t *)(pLocalPPSGen + (WR_PPS_GEN_CNTR_NSEC >> 2));
} // findLocalPPSGen


void findMILPiggy() //find WB address of MIL Piggy
{
  // get Wishbone address for MIL Piggy
  pMILPiggy   = find_device_adr(GSI, SCU_MIL);
  mprintf("pMILPiggy: 0x%08x\n",  pMILPiggy);
} // initMILPiggy

void getECATAI(uint32_t *timeHi, uint32_t *timeLo) // get TAI from local ECA
{
  uint32_t *pECATimeHi, *pECATimeLo;

  pECATimeHi = (uint32_t *)(pECACtrl + (ECA_TIME_HI_GET >> 2));
  pECATimeLo = (uint32_t *)(pECACtrl + (ECA_TIME_LO_GET >> 2));

  *timeHi = *pECATimeHi;
  *timeLo = *pECATimeLo;
} //getECATAI

void getWishboneTAI(uint32_t *secs, uint32_t *nsecs) // get TAI via Wishbone from local SoC
{ 
  *secs  = *pLocalPPSSecs;
  *nsecs = *pLocalPPSNsecs;
} // getWishboneTAI

// get TAI of remote SoC via EB
void getEtherboneTAI(uint32_t *secs, uint32_t *nsecs) // get TAI via Etherbone from remote SoC
{
  uint32_t  i;
  
  *secs  = 0x0;
  *nsecs = 0x0;

  // clear shared data for EB return values
  ebmClearSharedMem();

  // setup and commit EB cycle to remote device
  ebm_hi((uint32_t)pRemotePPSSecs);
  ebm_op((uint32_t)pRemotePPSSecs, (uint32_t)(&(pCpuRamExternalData4EB[0])), EBM_READ);
  ebm_op((uint32_t)pRemotePPSNsecs,(uint32_t)(&(pCpuRamExternalData4EB[1])), EBM_READ);
  ebm_flush();

  // wait until timeout values are received via shared mem or timeout
  i=0;
  while (!pSharedData4EB[0]) {
    i++;
    if (i > 250 * 31000) break; // timeout: 250ms
    asm("nop");
  } // while 

  if (pSharedData4EB[0] > 0) {
    i = 0;
    *secs  = pSharedData4EB[0];
    *nsecs = pSharedData4EB[1];
  } // if
} // getEtherboneTAI


void checkSyncViaEB(uint32_t *inSync, uint32_t *dT) // compare local and remote TAI and guess if we are in synch
{
  uint32_t rSecs, lSecs1, lSecs2; 
  uint32_t rNsecs, lNsecs1, lNsecs2;

  uint32_t meanLNsecs;
  uint32_t j;

  getWishboneTAI(&lSecs1, &lNsecs1);

  // wait until the next full second
  lSecs2 = lSecs1;
  while (lSecs2 == lSecs1) {
    getWishboneTAI(&lSecs2, &lNsecs2);
    asm("nop");
  } // while

  // wait for 1ms to be on the safe side
  for (j = 0; j < 31000; ++j) { asm("nop"); }
  
  getWishboneTAI(&lSecs1, &lNsecs1);
  getEtherboneTAI(&rSecs, &rNsecs);
  getWishboneTAI(&lSecs2, &lNsecs2);

  // if same number of seconds: assume we are in synch
  *inSync = ((lSecs1 == lSecs2) && (lSecs1 == rSecs));
  if (*inSync) {
    // get mean time to correct for execution time
    meanLNsecs = ((lNsecs1 + lNsecs2) >> 1);

    if (meanLNsecs > rNsecs) *dT = meanLNsecs - rNsecs;
    else                     *dT = rNsecs - meanLNsecs;
  } // if in sync
  else *dT = 0;
  mprintf("\n\n\nelapsed time : %09u ns\n", (lNsecs2 - lNsecs1));
  
  mprintf("local  WB TAI: %08u.%09u\n", lSecs1, lNsecs1);
  mprintf("remote EB TAI: %08u.%09u\n", rSecs, rNsecs);
  mprintf("local  WB TAI: %08u.%09u\n", lSecs2, lNsecs2);

  mprintf("in sync: %u; dT = %09u ns\n", *inSync, *dT);
} // checkSynchViaEB


void initSharedMem() // determine address and clear shared mem
{
  int      i,j;
  uint32_t idx;
  const uint32_t c_Max_Rams = 10;
  sdb_location found_sdb[c_Max_Rams];
  sdb_location found_clu;

  // get pointer to shared memory
  pShared           = (uint32_t *)_startshared;
  pSharedStatus     = (uint32_t *)(pShared + (DMUNIPZ_SHARED_STATUS >> 2));
  pSharedCmd        = (uint32_t *)(pShared + (DMUNIPZ_SHARED_CMD >> 2));
  pSharedNIterMain  = (uint32_t *)(pShared + (DMUNIPZ_SHARED_NITERMAIN >> 2));
  pSharedNRecMILEvt = (uint32_t *)(pShared + (DMUNIPZ_SHARED_NRECMILEVT >> 2));
  pSharedData4EB    = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DATA_4EB_START >> 2));

  // print local pointer info to UART
  mprintf("internal shared memory: start                   @ 0x%08x\n", (uint32_t)pShared);
  mprintf("internal shared memory: status address          @ 0x%08x\n", (uint32_t)pSharedStatus);
  mprintf("internal shared memory: command address         @ 0x%08x\n", (uint32_t)pSharedCmd);
  mprintf("internal shared memory: # of iterations address @ 0x%08x\n", (uint32_t)pSharedNIterMain);
  mprintf("internal shared memory: # of rec. MIL events    @ 0x%08x\n", (uint32_t)pSharedNRecMILEvt);
  mprintf("internal shared memory: EB return value address @ 0x%08x to 0x%08x\n", (uint32_t)pSharedData4EB, (uint32_t)(&(pSharedData4EB[DMUNIPZ_SHARED_DATA_4EB_SIZE])));

  // find address of CPU from external perspective
  idx = 0;
  find_device_multi(&found_clu, &idx, 1, GSI, LM32_CB_CLUSTER);	
  idx = 0;
  find_device_multi_in_subtree(&found_clu, &found_sdb[0], &idx, c_Max_Rams, GSI, LM32_RAM_USER);
  if(idx >= cpuId) {
    pCpuRamExternal           = (uint32_t *)(getSdbAdr(&found_sdb[cpuId]) & 0x7FFFFFFF); // CPU sees the 'world' under 0x8..., remove that bit to get host bridge perspective
    pCpuRamExternalStatus     = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_STATUS + SHARED_OFFS) >> 2));
    pCpuRamExternalCmd        = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_CMD + SHARED_OFFS) >> 2));
    pCpuRamExternalNIterMain  = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_NITERMAIN + SHARED_OFFS) >> 2));
    pCpuRamExternalNRecMILEvt = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_NRECMILEVT + SHARED_OFFS) >> 2));
    pCpuRamExternalData4EB    = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_DATA_4EB_START + SHARED_OFFS) >> 2));
    
    // print external WB info to UART
    mprintf("external WB address   : start            @ 0x%08x\n", (uint32_t)(pCpuRamExternal));
    mprintf("external WB address   : status           @ 0x%08x\n", (uint32_t)(pCpuRamExternalStatus));
    mprintf("external WB address   : command          @ 0x%08x\n", (uint32_t)(pCpuRamExternalCmd));
    mprintf("external WB address   : # of iterations  @ 0x%08x\n", (uint32_t)(pCpuRamExternalNIterMain));
    mprintf("external WB address   : # of rec. events @ 0x%08x\n", (uint32_t)(pCpuRamExternalNRecMILEvt));
    mprintf("external WB address   : EB return values @ 0x%08x to 0x%08x\n", (uint32_t)pCpuRamExternalData4EB, (uint32_t)(&(pCpuRamExternalData4EB[DMUNIPZ_SHARED_DATA_4EB_SIZE])));
  }
  else {
    pCpuRamExternal = (uint32_t *)ERROR_NOT_FOUND;
    mprintf("Could not find external WB address of my own RAM !\n");
  }
  
  // initialize values of shared memory
  *pSharedStatus     = DMUNIPZ_STAT_UNKNOWN;
  *pSharedCmd        = 0x0;
  *pSharedNIterMain  = 0x0;
  *pSharedNRecMILEvt = 0x0;
  ebmClearSharedMem();
} // initSharedMem 


void findEcaQueue() // find WB address of ECA channel for LM32
{
#define ECAQMAX           4     // max number of ECA channels in the system
#define ECACHANNELFORLM32 2     // this is a hack! suggest implementing finding via sdb-record and info

  // stuff below needed to get WB address of ECA queue
  sdb_location ECAQ_base[ECAQMAX];
  uint32_t ECAQidx = 0;         
  uint32_t *tmp;                
  int i;

  // get Wishbone address of ECA queue 
  // get list of ECA queues
  find_device_multi(ECAQ_base, &ECAQidx, ECAQMAX, ECA_QUEUE_SDB_VENDOR_ID, ECA_QUEUE_SDB_DEVICE_ID);
  pECAQ = 0x0;
  // find ECA queue connected to ECA chanel for LM32
  for (i=0; i < ECAQidx; i++) {
    tmp = (uint32_t *)(getSdbAdr(&ECAQ_base[i]));  
    if ( *(tmp + (ECA_QUEUE_QUEUE_ID_GET >> 2)) == ECACHANNELFORLM32) pECAQ = tmp;
  }
  mprintf("\n");
  if (!pECAQ) { mprintf("FATAL: can't find ECA queue for lm32, good bye! \n"); while(1) asm("nop"); }
  mprintf("ECA queue found at: %08x. Waiting for actions ...\n", pECAQ);
  mprintf("\n");
} // initeca

void findEcaControl() // find WB address of ECA Control
{
  pECACtrl         = find_device_adr(ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID);
}


void ecaHandler() // 1. query ECA for new action, 2. do action
{
  uint32_t flag;                // flag for the next action
  uint32_t evtIdHigh;           // high 32bit of eventID   
  uint32_t evtIdLow;            // low 32bit of eventID    
  uint32_t evtDeadlHigh;        // high 32bit of deadline  
  uint32_t evtDeadlLow;         // low 32bit of deadline   
  uint32_t actTag;              // tag of action           

  flag         = *(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2));   // read flag and check if there was an action

  if (flag & (0x0001 << ECA_VALID)) { // data is valid?
    
    // read data
    evtIdHigh    = *(pECAQ + (ECA_QUEUE_EVENT_ID_HI_GET >> 2));
    evtIdLow     = *(pECAQ + (ECA_QUEUE_EVENT_ID_LO_GET >> 2));
    evtDeadlHigh = *(pECAQ + (ECA_QUEUE_DEADLINE_HI_GET >> 2));
    evtDeadlLow  = *(pECAQ + (ECA_QUEUE_DEADLINE_LO_GET >> 2));
    actTag       = *(pECAQ + (ECA_QUEUE_TAG_GET >> 2));
    
    // pop action from channel
    *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;

    // here: do s.th. according to action
    switch (actTag) {
    case 0x4:
      mprintf("EvtID: 0x%08x%08x; deadline: %08x%08x; flag: %08x\n", evtIdHigh, evtIdLow, evtDeadlHigh, evtDeadlLow, flag);
      break;
    default:
      mprintf("ecaHandler: unknown tag\n");
    } // switch

  } // if data is valid
} // ecaHandler


void initCmds() // init stuff for handling commands, trivial for now, will be extended
{
  //  initalize command value: 0x0 means 'no command'
  *pSharedCmd     = 0x0;

  mprintf("Waiting for commands...\n");
} // initCmds


void cmdHandler() // check for command request and handle command
{
  // check, if a command has been issued (no cmd: 0x0
  if (*pSharedCmd) {
    switch (*pSharedCmd) {
    case 0x1:
      mprintf("received cmd 0x1\n");
      break;
    case 0x2:
      mprintf("received cmd 0x2\n");
      break;
    case 0x3:
      mprintf("received cmd 0x3\n");
      break;
    default:
      mprintf("cmdHandler: unknown command '0x%08x'\n",*pSharedCmd);
    } // switch 
    *pSharedCmd = 0x0; // reset cmd value in shared memory 
  } // if command 
} // ecaHandler


int writeToPZU(uint16_t ifbAddr, uint16_t modAddr, uint16_t data) // write bit field to module bus output (linked to UNI PZ)
{
  uint16_t wData     = 0x0;     // data to write
  uint16_t rData     = 0x0;     // data to read
  int16_t  busStatus = 0;       // status of bus operation
  
  // select module
  wData     = (modAddr << 8) | C_IO32_KANAL_0;
  if ((busStatus = writeDevMil(pMILPiggy, ifbAddr, IFB_ADR_BUS_W, wData)) != MIL_STAT_OK) return busStatus;

  // write data word
  wData     = data;
  busStatus = writeDevMil(pMILPiggy, ifbAddr, IFB_DATA_BUS_W, wData);

  return (busStatus);
} // writeToPZU


int readFromPZU(uint16_t ifbAddr, uint16_t modAddr, uint16_t *data) // read bit field from module bus input (linked to UNI PZ)
{
  uint16_t wData      = 0x0;    // data to write
  uint16_t rData      = 0x0;    // data to read
  int16_t  busStatus  = 0;      // status of bus operation

  // select module
  wData     = (modAddr << 8) | C_IO32_KANAL_0;
  if ((busStatus = writeDevMil(pMILPiggy, ifbAddr, IFB_ADR_BUS_W, wData))  != MIL_STAT_OK) return busStatus;

  // read data
  if ((busStatus = readDevMil(pMILPiggy, ifbAddr, IFB_DATA_BUS_R, &rData)) == MIL_STAT_OK) *data = rData;

  return(busStatus);
} // readFromPZU 
 

void configMILEvent(uint8_t evtCode, uint8_t virtAcc) // configure SoC to receive events via MIL bus
{
  int16_t busStatus;

  // initialize status and command register with intial values; disable event filtering; clear filter RAM
  writeCtrlStatRegEvtMil(pMILPiggy, MIL_CTRL_STAT_ENDECODER_FPGA | MIL_CTRL_STAT_INTR_DEB_ON);
  disableFilterEvtMil(pMILPiggy);
  clearFilterEvtMil(pMILPiggy);
  
  // set filter (FIFO and LEMO1 pulsing), configure LEMO for pulse generation
  setFilterEvtMil(pMILPiggy,  evtCode, virtAcc, MIL_FILTER_EV_TO_FIFO | MIL_FILTER_EV_PULS1_S);
  configLemoPulseEvtMil(pMILPiggy, 1);        

  // clear event FIFO and enable event filtering
  clearFifoEvtMil(pMILPiggy);
  enableFilterEvtMil(pMILPiggy);

  mprintf("dm-unipz: MIL Event Handdler waiting for Mon evtCode %d and virtAcc %d\n", evtCode, virtAcc);
} // configMILEvent


uint8_t wait4MILEvt(uint8_t evtCode, uint8_t virtAcc, uint32_t msTimeout) // 1. check if FIFO not empty; 2. pop element of FIFO 3; if relevant: do action (if not: discard)
{
  uint32_t evtDataRec;         // data of one MIL event
  uint32_t evtCodeRec;         // "event number"
  uint32_t virtAccRec;         // virtual accelerator
  uint32_t i = 0;

  for (i=0; i < msTimeout * DMUNIPZ_MS_ASMNOP; i++) {
    while (fifoNotemptyEvtMil(pMILPiggy)) {     // drain fifo until empty, check!!
      popFifoEvtMil(pMILPiggy, &evtDataRec);    // pop element
      evtCodeRec  = evtDataRec & 0x000000ff;    // extract event code
      virtAccRec  = (evtDataRec >> 8) & 0x0f;   // extract virtual accelerator (assuming event message)
      if ((evtCodeRec == evtCode) && (virtAccRec == virtAcc)) {
        getECATAI(&timeRecMILEvtHigh, &timeRecMILEvtLow); 
        (*pSharedNRecMILEvt)++;
        return DMUNIPZ_STAT_OK;
      } // received event we were waiting for
    } // while fifo not empty
    asm("nop"); // wait for 4 CPU ticks
  } // for i...

  return DMUNIPZ_STAT_TIMEDOUT;
} //wait4MILEvent

void triggerSIS18Cycle() //triggers the start of the SIS18 cycle
{
  // at present this routine is bogus and just writes to the local ECA

  uint32_t *pWBTarget; // Wishbone address we need to address with Etherbone

  // clear shared data for EB return values
  ebmClearSharedMem();

  /* for the first test, we send an event to the ECA of another node */
  pWBTarget = (uint32_t *)0x7ffffff0; 
  mprintf("pWBTarget: 0x%08x, timeHigh: 0x%08x\n", (uint32_t)pWBTarget, timeRecMILEvtHigh);

  /********* work in progress ***********/
  
  // setup and commit EB cycle to remote device
  // here: timing message
  ebm_hi((uint32_t)pWBTarget);
  ebm_op((uint32_t)pWBTarget, 0xaaaaaaaa, EBM_WRITE);                  // evtID high 32 bits
  ebm_op((uint32_t)pWBTarget, 0xbbbbbbbb, EBM_WRITE);                  // evtID low 32 bits
  ebm_op((uint32_t)pWBTarget, 0x00000000, EBM_WRITE);                  // parameter field high 32 bits
  ebm_op((uint32_t)pWBTarget, 0x00000000, EBM_WRITE);                  // parameter field low 32 bits
  ebm_op((uint32_t)pWBTarget, 0x00000000, EBM_WRITE);                  // TEF field 32 bits
  ebm_op((uint32_t)pWBTarget, 0x00000000, EBM_WRITE);                  // reserve field 32 bits

  ebm_op((uint32_t)pWBTarget, timeRecMILEvtLow, EBM_WRITE);            // timestamp low 32 bits
  ebm_op((uint32_t)pWBTarget, timeRecMILEvtHigh, EBM_WRITE);           // timestamp high 32 bits
  
  ebm_flush();
} // triggerSIS18Cycle


void main(void) {
  
  uint32_t i,j;

  uint32_t inSync;
  uint32_t dT;
  uint16_t test;
  int      status;


  WriteToPZU_Type  writePZUData; // Modulbus SIS, I/O-Modul 1, Bits 0..15
  ReadFromPZU_Type readPZUData;  // Modulbus SIS, I/O-Modul 3, Bits 0..15
  uint8_t uniReady2SisCode;      // event code for UNI_READY_TO_SIS
  uint8_t uniReady2SisAcc;       // virtual acc for UNI_READY_TO_SIS

  /************ unfinished - work in progress **************/

  
  init();                        // initialize stuff for lm32           
  //what is all this commented stuff? remove?  
  //initEca();                  /* init for actions from ECA        */
  //initCmds();                 /* init for cmds from shared mem    */
  ebmInit();                    // init EB master
  ebmClearSharedMem();          // clear shared memory used for EB return values
  initSharedMem();              // read/write to shared memory

  //what is all this commented stuff? remove?  
  //initRemotePPSGen();
  //initLocalPPSGen();
  //checkSync(&inSync, &dT);

  uniReady2SisCode =  1;  // hack: change later
  uniReady2SisAcc  = 15;  // hack: change later

  findEcaControl();
  findMILPiggy();
  configMILEvent(uniReady2SisCode, uniReady2SisAcc);
  
  status = echoTestDevMil(pMILPiggy, IFB_ADDRESS_SIS, 0xbabe);
  if (status != OKAY) mprintf("FATAL: Modulbus SIS IFK not available!\n");
  else                mprintf("OK: Modulbus SIS IFK found\n");

  i=0;
  while (1) {

    // clear FIFO to remove junk that has accumulated since the last call of 'wait4MILEvt'
    //disableFilterEvtMil(pMILPiggy);
    //clearFilterEvtMil(pMILPiggy);
    //enableFilterEvtMil(pMILPiggy);
    
    if (wait4MILEvt(uniReady2SisCode, uniReady2SisAcc, 1000) == DMUNIPZ_STAT_OK) {
      triggerSIS18Cycle();
      mprintf("triggered SIS 18 cycle at ECA time: high 0x%08x, low 0x%08x\n", timeRecMILEvtHigh, timeRecMILEvtLow);
    }

    /* do the things that have to be done                    */
    //ecaHandler();
    //cmdHandler();

    //status = writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, i);
    //if (status != OKAY) mprintf("ERROR: Can't write to PZU @ IFK 0x%x!\n", IFB_ADDRESS_SIS);
    //else                mprintf("OK: 0x%x written to PZU @ IFK 0x%x\n", i, IFB_ADDRESS_SIS);

    //status = readFromPZU(IFB_ADDRESS_UNI, IO_MODULE_1, &test);
    //if (status != OKAY) mprintf("ERROR: Can't read from PZU @ IFK 0x%x!\n", IFB_ADDRESS_UNI);
    //else                mprintf("OK: 0x%x read from PZU @ IFK 0x%x\n", test, IFB_ADDRESS_UNI);    
    

      //what is all this commented stuff? remove?    

    /*    status = writeToPZUUni(i);
    if (status != OKAY) mprintf("ERROR: Can't write to PZU (UNILAC side)!\n");
    else                mprintf("OK: wrote 0x%x to PZU (UNILAC side)\n",i);    
      

    status = readFromPZU(&readPZUData);
    if (status != OKAY) mprintf("ERROR: Can't read from PZU!\n");
    else                mprintf("OK: read data from PZU: 0x%x\n", readPZUData.uword); */

    // increment and update iteration counter
    i++;
    *pSharedNIterMain = i;

    // wait for 1000  milliseconds
    for (j = 0; j < (1000 * DMUNIPZ_MS_ASMNOP); ++j) { asm("nop"); }
  } // while
} /* main */
