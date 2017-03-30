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
volatile uint32_t *pSharedState;       // pointer to a "user defined" u32 register; here: get commnand from host
volatile uint32_t *pSharedData4EB ;    // pointer to a n x u32 register; here: memory region for receiving EB return values
uint32_t *pCpuRamExternal;             // external address (seen from host bridge) of this CPU's RAM            
uint32_t *pCpuRamExternalStatus;       // external address (seen from host bridge) of this CPU's RAM: status  (write)
uint32_t *pCpuRamExternalCmd;          // external address (seen from host bridge) of this CPU's RAM: command (read)
uint32_t *pCpuRamExternalState;        // external address (seen from host bridge) of this CPU's RAM: state (write)
uint32_t *pCpuRamExternalNIterMain;    // external address (seen from host bridge) of this CPU's RAM: # of iterations of main loop (write)
uint32_t *pCpuRamExternalNRecMILEvt;   // external address (seen from host bridge) of this CPU's RAM: # of received MIL events (write) 
uint32_t *pCpuRamExternalData4EB;      // external address (seen from host bridge) of this CPU's RAM: field for EB return values (read)

uint32_t timeRecMILEvtHigh;            // TAI of received MIL event 32 high bits
uint32_t timeRecMILEvtLow;             // TAI of received MIL event 32 low bits

uint16_t  uniReady2SisCode;            // event code for "UNI_READY_TO_SIS"
uint16_t  uniReady2SisAcc;             // virtual accelerator requested

uint32_t actState;                     // actual state
uint32_t reqState;                     // requested state

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
  
  //ebm_config_if(DESTINATION, 0x00267b000386, 0xc0a8a019,      0xebd0);             //Dst: scuxl0134
  ebm_config_if(DESTINATION, 0xffffffffffff, 0xffffffff ,      0xebd0);            //Dst: broadcast DANGER!!!
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
  //irq_set_mask(0x01);
  irq_disable(); 
} // init


void initSharedMem() // determine address and clear shared mem
{
  uint32_t idx;
  const uint32_t c_Max_Rams = 10;
  sdb_location found_sdb[c_Max_Rams];
  sdb_location found_clu;

  // get pointer to shared memory
  pShared           = (uint32_t *)_startshared;
  pSharedStatus     = (uint32_t *)(pShared + (DMUNIPZ_SHARED_STATUS >> 2));
  pSharedCmd        = (uint32_t *)(pShared + (DMUNIPZ_SHARED_CMD >> 2));
  pSharedState      = (uint32_t *)(pShared + (DMUNIPZ_SHARED_STATE >> 2));
  pSharedNIterMain  = (uint32_t *)(pShared + (DMUNIPZ_SHARED_NITERMAIN >> 2));
  pSharedNRecMILEvt = (uint32_t *)(pShared + (DMUNIPZ_SHARED_NRECMILEVT >> 2));
  pSharedData4EB    = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DATA_4EB_START >> 2));

  // print local pointer info to UART
  mprintf("internal shared memory: start                   @ 0x%08x\n", (uint32_t)pShared);
  mprintf("internal shared memory: status address          @ 0x%08x\n", (uint32_t)pSharedStatus);
  mprintf("internal shared memory: command address         @ 0x%08x\n", (uint32_t)pSharedCmd);
  mprintf("internal shared memory: state address           @ 0x%08x\n", (uint32_t)pSharedState);
  mprintf("internal shared memory: # of iterations address @ 0x%08x\n", (uint32_t)pSharedNIterMain);
  mprintf("internal shared memory: # of rec. MIL events    @ 0x%08x\n", (uint32_t)pSharedNRecMILEvt);
  mprintf("internal shared memory: EB return value address @ 0x%08x to 0x%08x\n", (uint32_t)pSharedData4EB, (uint32_t)(&(pSharedData4EB[DMUNIPZ_SHARED_DATA_4EB_SIZE >> 2])));

  // find address of CPU from external perspective
  idx = 0;
  find_device_multi(&found_clu, &idx, 1, GSI, LM32_CB_CLUSTER);	
  idx = 0;
  find_device_multi_in_subtree(&found_clu, &found_sdb[0], &idx, c_Max_Rams, GSI, LM32_RAM_USER);
  if(idx >= cpuId) {
    pCpuRamExternal           = (uint32_t *)(getSdbAdr(&found_sdb[cpuId]) & 0x7FFFFFFF); // CPU sees the 'world' under 0x8..., remove that bit to get host bridge perspective
    pCpuRamExternalStatus     = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_STATUS + SHARED_OFFS) >> 2));
    pCpuRamExternalCmd        = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_CMD + SHARED_OFFS) >> 2));
    pCpuRamExternalState      = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_STATE + SHARED_OFFS) >> 2));
    pCpuRamExternalNIterMain  = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_NITERMAIN + SHARED_OFFS) >> 2));
    pCpuRamExternalNRecMILEvt = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_NRECMILEVT + SHARED_OFFS) >> 2));
    pCpuRamExternalData4EB    = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_DATA_4EB_START + SHARED_OFFS) >> 2));
    
    // print external WB info to UART
    mprintf("external WB address   : start            @ 0x%08x\n", (uint32_t)(pCpuRamExternal));
    mprintf("external WB address   : status           @ 0x%08x\n", (uint32_t)(pCpuRamExternalStatus));
    mprintf("external WB address   : command          @ 0x%08x\n", (uint32_t)(pCpuRamExternalCmd));
    mprintf("external WB address   : state            @ 0x%08x\n", (uint32_t)(pCpuRamExternalState));
    mprintf("external WB address   : # of iterations  @ 0x%08x\n", (uint32_t)(pCpuRamExternalNIterMain));
    mprintf("external WB address   : # of rec. events @ 0x%08x\n", (uint32_t)(pCpuRamExternalNRecMILEvt));
    mprintf("external WB address   : EB return values @ 0x%08x to 0x%08x\n", (uint32_t)pCpuRamExternalData4EB, (uint32_t)(&(pCpuRamExternalData4EB[DMUNIPZ_SHARED_DATA_4EB_SIZE >> 2])));
  }
  else {
    pCpuRamExternal = (uint32_t *)ERROR_NOT_FOUND;
    mprintf("Could not find external WB address of my own RAM !\n");
  }
  
  // initialize values of shared memory
  *pSharedStatus     = DMUNIPZ_STATUS_UNKNOWN;
  *pSharedCmd        = 0x0;
  *pSharedState      = DMUNIPZ_STATE_UNKNOWN;
  *pSharedNIterMain  = 0x0;
  *pSharedNRecMILEvt = 0x0;
  ebmClearSharedMem();
} // initSharedMem 


void initState()
{
  actState      = DMUNIPZ_STATE_S0;
  reqState      = DMUNIPZ_STATE_S0;
  *pSharedState = DMUNIPZ_STATE_S0;
} //iniState


void findMILPiggy() //find WB address of MIL Piggy
{
  pMILPiggy = 0x0;
  
  // get Wishbone address for MIL Piggy
  pMILPiggy = find_device_adr(GSI, SCU_MIL);

  if (!pMILPiggy) {mprintf("dm-unipz: FATAL - can't find MIL piggy\n"); reqState = DMUNIPZ_STATE_FATAL;}
} // initMILPiggy


void findECAQueue() // find WB address of ECA channel for LM32
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

  if (!pECAQ) {mprintf("dm-unipz: FATAL - can't find ECA queue\n"); reqState = DMUNIPZ_STATE_FATAL; }
} // findECAQueue

void findECAControl() // find WB address of ECA Control
{
  pECACtrl = 0x0;

  // get Wishbone address for ECA Control
  pECACtrl = find_device_adr(ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID);

  if (!pECACtrl) {mprintf("dm-unipz: FATAL - can't find ECA control\n"); reqState = DMUNIPZ_STATE_FATAL; }
} // findECAControl


void getECATAI(uint32_t *timeHi, uint32_t *timeLo) // get TAI from local ECA
{
  uint32_t *pECATimeHi, *pECATimeLo;

  pECATimeHi = (uint32_t *)(pECACtrl + (ECA_TIME_HI_GET >> 2));
  pECATimeLo = (uint32_t *)(pECACtrl + (ECA_TIME_LO_GET >> 2));

  *timeHi = *pECATimeHi;
  *timeLo = *pECATimeLo;
} //getECATAI





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



int writeToPZU(uint16_t ifbAddr, uint16_t modAddr, uint16_t data) // write bit field to module bus output (linked to UNI PZ)
{
  uint16_t wData     = 0x0;     // data to write
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
 

uint32_t configMILEvent(uint16_t evtCode, uint16_t virtAcc) // configure SoC to receive events via MIL bus
{
  // initialize status and command register with intial values; disable event filtering; clear filter RAM
  if (writeCtrlStatRegEvtMil(pMILPiggy, MIL_CTRL_STAT_ENDECODER_FPGA | MIL_CTRL_STAT_INTR_DEB_ON) != MIL_STAT_OK) return 17; //DMUNIPZ_STATUS_ERROR;

  // clean up 
  if (disableLemoEvtMil(pMILPiggy, 1) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;
  if (disableLemoEvtMil(pMILPiggy, 2) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;
  if (disableFilterEvtMil(pMILPiggy)  != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;
  if (clearFilterEvtMil(pMILPiggy)    != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;

  // set filter (FIFO and LEMO1 pulsing), configure LEMO1 for pulse generation
  if (setFilterEvtMil(pMILPiggy,  evtCode, virtAcc, MIL_FILTER_EV_TO_FIFO | MIL_FILTER_EV_PULS1_S) != MIL_STAT_OK) return 18; //DMUNIPZ_STATUS_ERROR;
  if (configLemoPulseEvtMil(pMILPiggy, 1) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;

  return DMUNIPZ_STATUS_OK;
} // configMILEvent


uint16_t wait4MILEvt(uint16_t evtCode, uint16_t virtAcc, uint32_t msTimeout) // 1. check if FIFO not empty; 2. pop element of FIFO 3; if relevant: do action (if not: discard)
{
  uint32_t evtDataRec;         // data of one MIL event
  uint32_t evtCodeRec;         // "event number"
  uint32_t virtAccRec;         // virtual accelerator
  uint32_t i = 0;

  for (i=0; i < msTimeout * DMUNIPZ_MS_ASMNOP; i++) {
    while (fifoNotemptyEvtMil(pMILPiggy)) {     // drain fifo until empty
      popFifoEvtMil(pMILPiggy, &evtDataRec);    // pop element
      evtCodeRec  = evtDataRec & 0x000000ff;    // extract event code
      virtAccRec  = (evtDataRec >> 8) & 0x0f;   // extract virtual accelerator (assuming event message)
      if ((evtCodeRec == evtCode) && (virtAccRec == virtAcc)) return DMUNIPZ_STATUS_OK;
    } // while fifo not empty
    asm("nop"); // wait 4 CPU ticks
  } // for i...

  return DMUNIPZ_STATUS_TIMEDOUT;
} //wait4MILEvent


void triggerSIS18Cycle() //triggers the start of the SIS18 cycle
{
  // at present this routine is bogus and just writes to the local ECA

  uint32_t WBTargetAddress; // Wishbone address we need to address with Etherbone
  uint32_t i;
  
  setLemoOutputEvtMil(pMILPiggy, 2, 1);
  for (i=0; i< 10 * DMUNIPZ_US_ASMNOP; i++) asm("nop");
  setLemoOutputEvtMil(pMILPiggy, 2, 0);
  
  // clear shared data for EB return values
  //ebmClearSharedMem();

  
  /* for the first test, we send an event to the ECA of another node */
  WBTargetAddress = 0x7ffffff0;
  //mprintf("WBTargetAddress: 0x%08x, timeHigh: 0x%08x\n", (uint32_t)WBTargetAddress, timeRecMILEvtHigh);

  /********* work in progress ***********/
  
  // setup and commit EB cycle to remote device
  // here: timing message
  // commented: bad things happen here!!!!
  /*
  getECATAI(&timeRecMILEvtHigh, &timeRecMILEvtLow);
  */

  /* ATOMIC!!! 
  ebm_hi((uint32_t)WBTargetAddress);
  ebm_op((uint32_t)WBTargetAddress, 0xaaaaaaaa, EBM_WRITE);                  // evtID high 32 bits
  ebm_op((uint32_t)WBTargetAddress, 0xbbbbbbbb, EBM_WRITE);                  // evtID low 32 bits
  ebm_op((uint32_t)WBTargetAddress, 0x00000000, EBM_WRITE);                  // parameter field high 32 bits
  ebm_op((uint32_t)WBTargetAddress, 0x00000000, EBM_WRITE);                  // parameter field low 32 bits
  ebm_op((uint32_t)WBTargetAddress, 0x00000000, EBM_WRITE);                  // TEF field 32 bits
  ebm_op((uint32_t)WBTargetAddress, 0x00000000, EBM_WRITE);                  // reserve field 32 bits
  ebm_op((uint32_t)WBTargetAddress, timeRecMILEvtHigh, EBM_WRITE);           // timestamp high 32 bits
  ebm_op((uint32_t)WBTargetAddress, timeRecMILEvtLow, EBM_WRITE);            // timestamp low 32 bits

  ebm_flush();
  */
} // triggerSIS18Cycle


uint32_t entryActionConfigured()
{
  uint32_t status = DMUNIPZ_STATUS_OK;
 
  // check if modulbus I/O is ok
  if ((status = echoTestDevMil(pMILPiggy, IFB_ADDRESS_SIS, 0xbabe)) != DMUNIPZ_STATUS_OK) {
    mprintf("dm-unipz: ERROR - modulbus SIS IFK not available!\n");
    return status;
  }

  // configure MIL piggy for timing events
  if ((status = configMILEvent(uniReady2SisCode, uniReady2SisAcc)) != DMUNIPZ_STATUS_OK) {
    mprintf("dm-unipz: ERROR - failed to configure MIL piggy for receiving timing events! %d %d\n", status, uniReady2SisAcc);
    return status;
  }

  configLemoOutputEvtMil(pMILPiggy, 2);
 
  return status;
} // entryActionConfigured


uint32_t entryActionOperation()
{
  if (clearFifoEvtMil(pMILPiggy) != MIL_STAT_OK)    return DMUNIPZ_STATUS_ERROR;
  if (enableFilterEvtMil(pMILPiggy) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;
 
  return DMUNIPZ_STATUS_OK;
} // entryActionOperation


uint32_t exitActionOperation()
{
  if (disableFilterEvtMil(pMILPiggy) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;
  
  return DMUNIPZ_STATUS_OK;
} // exitActionOperation

uint32_t actionRecover()
{
  return DMUNIPZ_STATUS_OK;
} // actionRecover


void cmdHandler() // handle commands from the outside world
{
  uint32_t cmd;

  cmd = *pSharedCmd;
  // check, if the command is valid and request state change
  if (cmd) {
    switch (cmd) {
    case DMUNIPZ_CMD_CONFIGURE :
      reqState =  DMUNIPZ_STATE_CONFIGURED;
      mprintf("received cmd %d\n", cmd);
      break;
    case DMUNIPZ_CMD_STARTOP :
      reqState = DMUNIPZ_STATE_OPERATION;
      mprintf("received cmd %d\n", cmd);
      break;
    case DMUNIPZ_CMD_STOPOP :
      reqState = DMUNIPZ_STATE_STOPPING;
      mprintf("received cmd %d\n", cmd);
      break;
    case DMUNIPZ_CMD_IDLE :
      reqState = DMUNIPZ_STATE_IDLE;
      mprintf("received cmd %d\n", cmd);
      break;
    case DMUNIPZ_CMD_RECOVER :
      reqState = DMUNIPZ_CMD_IDLE;
      mprintf("received cmd %d\n", cmd);
      break;
    default:
      mprintf("cmdHandler: unknown command '0x%08x'\n",*pSharedCmd);
    } // switch 
    *pSharedCmd = 0x0; // reset cmd value in shared memory 
  } // if command 
} // ecaHandler


void changeState() //state machine; see dm-unipz.h for possible states and transitions
{
  uint32_t status;
  uint32_t nextState;

  // if something severe happened, perform immediate state change and return
  if ((reqState == DMUNIPZ_STATE_ERROR) || (reqState == DMUNIPZ_STATE_FATAL)) { 
    actState = reqState;
    return;
  } // if something severe happened...
  
  nextState = actState; // per default: remain in actual state without exit or entry action
  status    = DMUNIPZ_STATUS_OK; 
  
  switch (actState)     // check for allowed transitions: 1. determine next state, 2. perform exit or entry actions if required
    {
    case DMUNIPZ_STATE_S0:
      nextState = DMUNIPZ_STATE_IDLE;     //automatic transition
      break;
    case DMUNIPZ_STATE_IDLE:
      if      (reqState == DMUNIPZ_STATE_CONFIGURED)  {status = entryActionConfigured(); nextState = reqState;}
      break;
    case DMUNIPZ_STATE_CONFIGURED:
      if      (reqState == DMUNIPZ_STATE_IDLE)        {                                  nextState = reqState;}
      else if (reqState == DMUNIPZ_STATE_CONFIGURED)  {status = entryActionConfigured(); nextState = reqState;}
      else if (reqState == DMUNIPZ_STATE_OPERATION)   {status = entryActionOperation();  nextState = reqState;}
      break;
    case DMUNIPZ_STATE_OPERATION:
      if      (reqState == DMUNIPZ_STATE_STOPPING)    {status = exitActionOperation();   nextState = reqState;}
      break;
    case DMUNIPZ_STATE_STOPPING:
      nextState = DMUNIPZ_STATE_CONFIGURED; //automatic transition
    case DMUNIPZ_STATE_ERROR:
      if      (reqState == DMUNIPZ_STATE_IDLE)        {status = actionRecover();         nextState = reqState;}
      break;
    default:   nextState = DMUNIPZ_STATE_S0;  // in case we are in an undefined state, start all over again
    } // if reqState

  // in case state change was not successful transit to error state
  if (status != DMUNIPZ_STATUS_OK) nextState = DMUNIPZ_STATE_ERROR;

  if (actState != nextState) mprintf("dm-unipz: changed to state %d\n", nextState);

  // finaly ...
  actState = nextState;
  reqState = DMUNIPZ_STATE_UNKNOWN; //check
} //changeState


void doActionOperation()
{
  uint32_t i;

  if (wait4MILEvt(uniReady2SisCode, uniReady2SisAcc, 1000) == DMUNIPZ_STATUS_OK) {
      //getECATAI(&timeRecMILEvtHigh, &timeRecMILEvtLow); 
    triggerSIS18Cycle();
    //(*pSharedNRecMILEvt)++;
    //mprintf("triggered SIS 18 cycle at ECA time: high 0x%08x, low 0x%08x\n", timeRecMILEvtHigh, timeRecMILEvtLow);
  } // if wait 
  
  //for (i=0; i< DMUNIPZ_MS_ASMNOP * 1000; i++) asm("nop");
  //triggerSIS18Cycle();
} //doActionOperation


void main(void) {
  
  uint32_t i,j;

  uint32_t inSync;
  uint32_t dT;
  uint16_t test;
  int      status;


  WriteToPZU_Type  writePZUData; // Modulbus SIS, I/O-Modul 1, Bits 0..15
  ReadFromPZU_Type readPZUData;  // Modulbus SIS, I/O-Modul 3, Bits 0..15

  init();                        // initialize stuff for lm32
  initSharedMem();               // initialize shared memory
  initState();                   // init state machine
  ebmInit();                     // init EB master 
  ebmClearSharedMem();           // clear shared memory used for EB return values


  findECAQueue();                // find WB device, required to receive events from the ECA
  findECAControl();              // find WB device, required to obtain timestamp
  findMILPiggy();                // find WB device, required for device bus master and event bus slave

  initCmds();                    // init command handler

  uniReady2SisCode =  1;         // hack: change later
  uniReady2SisAcc  = 15;         // hack: change later

  i=0;
  while (1) {
    cmdHandler();    // check for commands and possibly request state changes
    changeState();   // handle requested state changes
    
    switch(actState) // state specific do actions
      {
      case DMUNIPZ_STATE_OPERATION :
        doActionOperation();
        break;
      case DMUNIPZ_STATE_FATAL :
        mprintf("dm-unipz: a FATAL error has occured. Good bye.\n");
        while (1) asm("nop"); // RIP!
        break;
      default :
        for (j = 0; j < (1000 * DMUNIPZ_MS_ASMNOP); j++) { asm("nop"); } //wait for 100 ms
      } // switch 
    
    //triggerSIS18Cycle();
    //for (j = 0; j <  DMUNIPZ_MS_ASMNOP; j++)  { asm("nop"); } //wait for 1 ms
    
    
    /* 
    if (wait4MILEvt(uniReady2SisCode, uniReady2SisAcc, 1000) == DMUNIPZ_STATUS_OK) {
      //getECATAI(&timeRecMILEvtHigh, &timeRecMILEvtLow); 
      triggerSIS18Cycle();
      //(*pSharedNRecMILEvt)++;
      //mprintf("triggered SIS 18 cycle at ECA time: high 0x%08x, low 0x%08x\n", timeRecMILEvtHigh, timeRecMILEvtLow);
    } 
    // disableFilterEvtMil(pMILPiggy);

    /* do the things that have to be done                    */
    //ecaHandler();


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

    // update status
    i++; *pSharedNIterMain = i;
    *pSharedState = actState;
  } // while
} /* main */
