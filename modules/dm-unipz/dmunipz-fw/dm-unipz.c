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
uint32_t *pSharedNTransfer;            // pointer to a "user defined" u32 register; here: publish # of transfers
uint32_t *pSharedVirtAcc;              // pointer to a "user defined" u32 register; here: publish # of virtual accelerator
uint32_t *pSharedStatTrans;            // pointer to a "user defined" u32 register; here: publish status of ongoing transfer
volatile uint32_t *pSharedCmd;         // pointer to a "user defined" u32 register; here: get command from host
uint32_t *pSharedState;                // pointer to a "user defined" u32 register; here: publish status
volatile uint32_t *pSharedData4EB ;    // pointer to a n x u32 register; here: memory region for receiving EB return values
uint32_t *pCpuRamExternal;             // external address (seen from host bridge) of this CPU's RAM            
uint32_t *pCpuRamExternalStatus;       // external address (seen from host bridge) of this CPU's RAM: status  (write)
uint32_t *pCpuRamExternalCmd;          // external address (seen from host bridge) of this CPU's RAM: command (read)
uint32_t *pCpuRamExternalState;        // external address (seen from host bridge) of this CPU's RAM: state (write)
uint32_t *pCpuRamExternalNIterMain;    // external address (seen from host bridge) of this CPU's RAM: # of iterations of main loop (write)
uint32_t *pCpuRamExternalNTransfer;    // external address (seen from host bridge) of this CPU's RAM: # of transfers (write) 
uint32_t *pCpuRamExternalVirtAcc;      // external address (seen from host bridge) of this CPU's RAM: # of virtual accelarator (write)
uint32_t *pCpuRamExternalStatTrans;    // external address (seen from host bridge) of this CPU's RAM: status of ongoing transfer
uint32_t *pCpuRamExternalData4EB;      // external address (seen from host bridge) of this CPU's RAM: field for EB return values (read)

uint32_t timeRecMILEvtHigh;            // TAI of received MIL event 32 high bits
uint32_t timeRecMILEvtLow;             // TAI of received MIL event 32 low bits

uint32_t actStatus;                    // actual (error) status, see DMUNIPZ_STATUS_...
uint32_t actState;                     // actual state,          see DMUNIPZ_STATE_...
uint32_t reqState;                     // requested state

WriteToPZU_Type  writePZUData;         // Modulbus SIS, I/O-Modul 1, Bits 0..15

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
  
  ebm_config_if(DESTINATION, 0x00267b000386, 0xc0a8a019,      0xebd0);             //Dst: scuxl0134
  //ebm_config_if(DESTINATION, 0xffffffffffff, 0xffffffff ,      0xebd0);            //Dst: broadcast DANGER!!!
  //ebm_config_if(SOURCE,      0x00267b000401, *(pEbCfg + (EBC_SRC_IP>>2)), 0xebd0); //Src: MAC is a hack!, WR IP lxdv54
  ebm_config_if(SOURCE,      0x00267b000321, *(pEbCfg + (EBC_SRC_IP>>2)), 0xebd0); //Src: MAC is a hack!, WR IP scuxl0033

  // mprintf("my IP:  0x%08x\n",  *(pEbCfg + (EBC_SRC_IP>>2)));
  // mprintf("pEbCfg: 0x%08x\n",  pEbCfg);
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
  pSharedNTransfer  = (uint32_t *)(pShared + (DMUNIPZ_SHARED_TRANSN >> 2));
  pSharedVirtAcc    = (uint32_t *)(pShared + (DMUNIPZ_SHARED_TRANSVIRTACC >> 2));
  pSharedStatTrans  = (uint32_t *)(pShared + (DMUNIPZ_SHARED_TRANSSTATUS >> 2));
  pSharedData4EB    = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DATA_4EB_START >> 2));

  // print local pointer info to UART
  mprintf("internal shared memory: start                   @ 0x%08x\n", (uint32_t)pShared);
  mprintf("internal shared memory: status address          @ 0x%08x\n", (uint32_t)pSharedStatus);
  mprintf("internal shared memory: command address         @ 0x%08x\n", (uint32_t)pSharedCmd);
  mprintf("internal shared memory: state address           @ 0x%08x\n", (uint32_t)pSharedState);
  mprintf("internal shared memory: # of iterations address @ 0x%08x\n", (uint32_t)pSharedNIterMain);
  mprintf("internal shared memory: # of transfers          @ 0x%08x\n", (uint32_t)pSharedNTransfer);
  mprintf("internal shared memory: # virtual accelerator   @ 0x%08x\n", (uint32_t)pSharedVirtAcc);
  mprintf("internal shared memory: status of transfer      @ 0x%08x\n", (uint32_t)pSharedStatTrans);
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
    pCpuRamExternalNTransfer  = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_TRANSN + SHARED_OFFS) >> 2));
    pCpuRamExternalVirtAcc    = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_TRANSVIRTACC + SHARED_OFFS) >> 2));
    pCpuRamExternalStatTrans  = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_TRANSSTATUS + SHARED_OFFS) >> 2));
    pCpuRamExternalData4EB    = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_DATA_4EB_START + SHARED_OFFS) >> 2));
    
    // print external WB info to UART
    mprintf("external WB address   : start            @ 0x%08x\n", (uint32_t)(pCpuRamExternal));
    mprintf("external WB address   : status           @ 0x%08x\n", (uint32_t)(pCpuRamExternalStatus));
    mprintf("external WB address   : command          @ 0x%08x\n", (uint32_t)(pCpuRamExternalCmd));
    mprintf("external WB address   : state            @ 0x%08x\n", (uint32_t)(pCpuRamExternalState));
    mprintf("external WB address   : # of iterations  @ 0x%08x\n", (uint32_t)(pCpuRamExternalNIterMain));
    mprintf("external WB address   : # of transfers   @ 0x%08x\n", (uint32_t)(pCpuRamExternalNTransfer));
    mprintf("external WB address   : # virtual acc.   @ 0x%08x\n", (uint32_t)(pCpuRamExternalVirtAcc));
    mprintf("external WB address   : status transfer  @ 0x%08x\n", (uint32_t)(pCpuRamExternalStatTrans));
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
  *pSharedNTransfer  = 0x0;
  *pSharedVirtAcc    = 0xFF;
  *pSharedStatTrans  = DMUNIPZ_TRANS_UNKNOWN;
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
  
  /* check: registers not protected against overflow of low32bits. To Do: read high bits twice! */

  pECATimeHi = (uint32_t *)(pECACtrl + (ECA_TIME_HI_GET >> 2));
  pECATimeLo = (uint32_t *)(pECACtrl + (ECA_TIME_LO_GET >> 2));

  *timeHi = *pECATimeHi;
  *timeLo = *pECATimeLo;
} //getECATAI


uint32_t wait4ECAEvent(uint32_t msTimeout, uint32_t *virtAcc)  // 1. query ECA for actions, 2. trigger activity
{
  uint32_t *pECAFlag;           // address of ECA flag
  uint32_t evtIdHigh;           // high 32bit of eventID   
  uint32_t evtIdLow;            // low 32bit of eventID    
  uint32_t evtDeadlHigh;        // high 32bit of deadline  
  uint32_t evtDeadlLow;         // low 32bit of deadline   
  uint32_t actTag;              // tag of action           
  uint32_t nextAction;          // describes what to do next
  uint64_t timeoutT;            // when to time out

  *virtAcc = 0xff;              // 0xff: virt acc is not yet set
  pECAFlag     = (uint32_t *)(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2));   // address of ECA flag
  timeoutT = getSysTime() + msTimeout * 1000000;

  while (getSysTime() < timeoutT) {
    if (*pECAFlag & (0x0001 << ECA_VALID)) {               // if ECA data is valid
      
      // read data
      evtIdHigh    = *(pECAQ + (ECA_QUEUE_EVENT_ID_HI_GET >> 2));
      evtIdLow     = *(pECAQ + (ECA_QUEUE_EVENT_ID_LO_GET >> 2));
      evtDeadlHigh = *(pECAQ + (ECA_QUEUE_DEADLINE_HI_GET >> 2));
      evtDeadlLow  = *(pECAQ + (ECA_QUEUE_DEADLINE_LO_GET >> 2));
      actTag       = *(pECAQ + (ECA_QUEUE_TAG_GET >> 2));
    
      // pop action from channel
      *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;

      // here: do s.th. according to tag
      switch (actTag) 
        {
        case DMUNIPZ_ECADO_REQTK :
          nextAction = DMUNIPZ_ECADO_REQTK;
          *virtAcc = evtIdLow & 0xf;   // check: later we need to extract this info from the data delivered by the ECA 
          // check: later we also need to extract address and information we need to write to the Datamaster
          // mprintf("dm-unipz: received ECA event request TK\n");
          break;
        case DMUNIPZ_ECADO_REQBEAM :
          nextAction = DMUNIPZ_ECADO_REQBEAM;
          *virtAcc = evtIdLow & 0xf;   // check: later we need to extract this info from the data delivered by the ECA 
          // check: later we also need to extract address and information we need to write to the Datamaster
          // mprintf("dm-unipz: received ECA event request beam\n");
          break;
        default: 
          nextAction = DMUNIPZ_ECADO_UNKOWN;
        } // switch

      return nextAction;

    } // if data is valid
  } // while not timed out

  return  DMUNIPZ_ECADO_TIMEOUT;
} // wait for ECA event


void initCmds() // init stuff for handling commands, trivial for now, will be extended
{
  //  initalize command value: 0x0 means 'no command'
  *pSharedCmd     = 0x0;

  mprintf("Waiting for commands...\n");
} // initCmds


int16_t writeToPZU(uint16_t ifbAddr, uint16_t modAddr, uint16_t data) // write bit field to module bus output (linked to UNI PZ)
{
  uint16_t wData     = 0x0;     // data to write
  int16_t  busStatus = 0;       // status of bus operation
  
  // select module
  wData     = (modAddr << 8) | C_IO32_KANAL_0;
  if ((busStatus = writeDevMil(pMILPiggy, ifbAddr, IFB_ADR_BUS_W, wData)) != MIL_STAT_OK) return busStatus;

  // write data word
  wData     = data;
  busStatus = writeDevMil(pMILPiggy, ifbAddr, IFB_DATA_BUS_W, wData);
  // mprintf("dm-unipz: writeToPZU, wrote wData %d\n", wData);

  return (busStatus);
} // writeToPZU


int16_t readFromPZU(uint16_t ifbAddr, uint16_t modAddr, uint16_t *data) // read bit field from module bus input (linked to UNI PZ)
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


uint32_t checkClearReqNotOk(uint32_t msTimeout)
{
  ReadFromPZU_Type readPZUData;  // Modulbus SIS, I/O-Modul 3, Bits 0..15
  int16_t          status;       // status MIL device bus operation
  uint64_t         timeoutT;     // when to time out

  timeoutT = getSysTime() + msTimeout * 1000000;

  if ((status = readFromPZU(IFB_ADDRESS_SIS, IO_MODULE_3, &(readPZUData.uword))) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;     // read from modulbus I/O (UNIPZ)
  if (readPZUData.bits.Req_not_ok == true) {
    // mprintf("dm-unipz: UNILAC says 'req_not_ok' \n");
    writePZUData.uword               = 0x0;
    writePZUData.bits.Req_not_ok_Ack = true;
    if ((status = writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword)) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;      // request to clear not_ok flag

    while (getSysTime() < timeoutT) {
      if ((status = readFromPZU(IFB_ADDRESS_SIS, IO_MODULE_3, &(readPZUData.uword))) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR; // read from modulbus I/O (UNIPZ)
      if (readPZUData.bits.Req_not_ok == false) {
        writePZUData.bits.Req_not_ok_Ack = false;                     // chk
        writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword); // release request to clear flag
        return DMUNIPZ_STATUS_REQNOTOK;        
      } // if flag successfully cleared
    } // while not timed out
  
    return DMUNIPZ_STATUS_TIMEDOUT;
  } // if 'req_no_ok'

  return DMUNIPZ_STATUS_OK;
} // checkClearReqNotOk


uint32_t requestTK(uint32_t msTimeout, uint32_t virtAcc, uint32_t dryRun)
{
  ReadFromPZU_Type readPZUData;  // Modulbus SIS, I/O-Modul 3, Bits 0..15
  int16_t          status;       // status MIL device bus operation
  uint64_t         timeoutT;     // when to time out

  if (virtAcc > 0xf) return DMUNIPZ_STATUS_OUTOFRANGE;  

  timeoutT = getSysTime() + msTimeout * 1000000;

  // send request to modulbus I/O (UNIPZ)
  writePZUData.uword               = 0x0;
  writePZUData.bits.TK_Request     = true;
  writePZUData.bits.SIS_Acc_Select = virtAcc;
  writePZUData.bits.ReqNoBeam      = dryRun;
  if ((status = writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword)) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;

  while (getSysTime() < timeoutT) {                                                                                                   // wait for acknowledgement from UNIPZ
    if ((status = readFromPZU(IFB_ADDRESS_SIS, IO_MODULE_3, &(readPZUData.uword))) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR; // read from modulbus I/O (UNIPZ)
    if (readPZUData.bits.TK_Req_Ack == true) return DMUNIPZ_STATUS_OK;                                                                // check for acknowledgement
    if ((status = checkClearReqNotOk(msTimeout)) != DMUNIPZ_STATUS_OK) return DMUNIPZ_STATUS_REQTKFAILED;                             // check for 'request not ok'
  } // while not timed out

  // mprintf("dm-unipz: requestTK looks like timeout; I have read %d from modulbus I/O \n", readPZUData.uword);

  // check for "request not ok"
  // if (readPZUData.bits.Req_not_ok == true) return DMUNIPZ_STATUS_REQTKFAILED;  // check for "Request not ok"
  
  return DMUNIPZ_STATUS_TIMEDOUT;
} // requestTK


uint32_t releaseTK()
{
  int16_t          status;       // status MIL device bus operation

  // send request to modulbus I/O (UNIPZ)
  writePZUData.bits.TK_Request     = false;
  if ((status = writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword)) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;
  
  return DMUNIPZ_STATUS_OK;
} // releaseTK


uint32_t requestBeam(uint32_t msTimeout)
{
  ReadFromPZU_Type readPZUData;  // Modulbus SIS, I/O-Modul 3, Bits 0..15
  int16_t          status;       // status MIL device bus operation
  uint32_t         i,j;          

  // send request to modulbus I/O (UNIPZ)
  writePZUData.bits.SIS_Request  = true;
  
  if ((status = writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword)) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;
  // mprintf("dm-unipz: requested beam with PZUData %d\n", writePZUData.uword);
  // wait for acknowledgement from UNIPZ
  /* don't wait for acknoledgement, code commented
  for (i=0; i < msTimeout; i++) {      // while not timed out, poll reply from UNIPZ

    if ((status = readFromPZU(IFB_ADDRESS_SIS, IO_MODULE_3, &(readPZUData.uword))) != MIL_STAT_OK) return DMUNIPZ_STATUS_REQBEAMFAILED; // read from modulbus I/O (UNIPZ)

    if (readPZUData.bits.SIS_Req_Ack == true) {
      mprintf("dm-unipz: got acknowledgement for beam request: readPUZData.uword %d\n", readPZUData.uword);
      return DMUNIPZ_STATUS_OK;        // check for acknowledgement
    } // if request acknowledged 
    
    for (j=0; j<DMUNIPZ_MS_ASMNOP;j++) asm("nop");                             // not yet acknowledged: wait for 1ms 
  } // for i: loop until timeout
  
  return DMUNIPZ_STATUS_TIMEDOUT;
  */

  return DMUNIPZ_STATUS_OK;
} // requestBeam


uint32_t releaseBeam()
{
  int16_t          status;       // status MIL device bus operation

  // send request to modulbus I/O (UNIPZ)
  writePZUData.bits.SIS_Request  = false;
  writePZUData.bits.ReqNoBeam    = false;
  if ((status = writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword)) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;
 
  return DMUNIPZ_STATUS_OK;
} // releaseBeam

 
uint32_t configMILEvent(uint16_t evtCode) // configure SoC to receive events via MIL bus
{
  uint32_t i;

  // initialize status and command register with initial values; disable event filtering; clear filter RAM
  if (writeCtrlStatRegEvtMil(pMILPiggy, MIL_CTRL_STAT_ENDECODER_FPGA | MIL_CTRL_STAT_INTR_DEB_ON) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;

  // clean up 
  if (disableLemoEvtMil(pMILPiggy, 1) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;
  if (disableLemoEvtMil(pMILPiggy, 2) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;
  if (disableFilterEvtMil(pMILPiggy)  != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;
  if (clearFilterEvtMil(pMILPiggy)    != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;

  // set filter (FIFO and LEMO1 pulsing) for all possible vitual accelerators
  for (i=0; i < (0xf+1); i++) if (setFilterEvtMil(pMILPiggy,  evtCode, i, MIL_FILTER_EV_TO_FIFO | MIL_FILTER_EV_PULS1_S) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;

  // configure LEMO1 for pulse generation
  if (configLemoPulseEvtMil(pMILPiggy, 1) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;

  return DMUNIPZ_STATUS_OK;
} // configMILEvent


uint16_t wait4MILEvt(uint16_t evtCode, uint16_t virtAcc, uint32_t msTimeout) // 1. check if FIFO not empty; 2. pop element of FIFO 3; if relevant: do action (if not: discard)
{
  uint32_t evtDataRec;         // data of one MIL event
  uint32_t evtCodeRec;         // "event number"
  uint32_t virtAccRec;         // virtual accelerator
  uint64_t timeoutT;           // when to time out
  uint32_t j = 0;       //chk: debug

  // mprintf("dm-unipz: wait 4 mil event, max i %d, msTimeout %d, evtCode %d, virtAcc %d\n",  msTimeout * DMUNIPZ_MS_ASMNOP, msTimeout, evtCode, virtAcc);

  timeoutT = getSysTime() + msTimeout * 1000000;

  while(getSysTime() < timeoutT) {
    while (fifoNotemptyEvtMil(pMILPiggy)) {     // drain fifo until empty
      popFifoEvtMil(pMILPiggy, &evtDataRec);    // pop element
      evtCodeRec  = evtDataRec & 0x000000ff;    // extract event code
      virtAccRec  = (evtDataRec >> 8) & 0x0f;   // extract virtual accelerator (assuming event message)
      // if (virtAccRec == virtAcc) mprintf("dm-unipz: got virtAcc %d, evtCode %d\n", virtAccRec, evtCodeRec);
      if ((evtCodeRec == evtCode) && (virtAccRec == virtAcc)) return DMUNIPZ_STATUS_OK;
      j++;
    } // while fifo not empty
    asm("nop"); // wait 4 CPU ticks
  } // while not timed out

  // mprintf("dm-unipz: wait 4 mil event, timed out, j %d\n",j);

  return DMUNIPZ_STATUS_TIMEDOUT;
} //wait4MILEvent


void pulseLemo2() //for debugging with scope
{
  uint32_t i;

  setLemoOutputEvtMil(pMILPiggy, 2, 1);
  for (i=0; i< 10 * DMUNIPZ_US_ASMNOP; i++) asm("nop");
  setLemoOutputEvtMil(pMILPiggy, 2, 0);
} // pulseLemo2


void replyRequestTK()
{
  /* check: not yet implemented */
} // replyRequestTK


void replyRequestBeam()
{
  /* check: not yet implemented */
} // replyRequestBeam


uint32_t entryActionConfigured()
{
  uint32_t status = DMUNIPZ_STATUS_OK;
  uint32_t virtAcc;
  uint32_t i;

  // check if modulbus I/O is ok
  if ((status = echoTestDevMil(pMILPiggy, IFB_ADDRESS_SIS, 0xbabe)) != DMUNIPZ_STATUS_OK) {
    mprintf("dm-unipz: ERROR - modulbus SIS IFK not available!\n");
    return status;
  }

  // configure MIL piggy for timing events for all 16 virtual accelerators
  //for (i=0; i < (0xf + 1); i++)
    if ((status = configMILEvent(DMUNIPZ_EVT_UNI_READY)) != DMUNIPZ_STATUS_OK) {
      mprintf("dm-unipz: ERROR - failed to configure MIL piggy for receiving timing events! %d %d\n", status, i);
      return status;
    } 

  configLemoOutputEvtMil(pMILPiggy, 2);    // used to see a blinking LED (and optionally connect a scope) for debugging
  checkClearReqNotOk(DMUNIPZ_REQTIMEOUT);  // in case a 'req_not_ok' flag has been set at UNIPZ, try to clear it

  // clear bits for modulbus I/O to UNILAC
  writePZUData.uword               = 0x0;
  writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword);

  // empty ECA queue for lm32
  i = 0;
  while (wait4ECAEvent(1, &virtAcc) !=  DMUNIPZ_ECADO_TIMEOUT) {i++;}
  mprintf("dm-unipz: removed %d entries from ECA queue\n", i);

  return status;
} // entryActionConfigured


uint32_t entryActionOperation()
{
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

  // in case state change can not be done, transit to error state
  if (status != DMUNIPZ_STATUS_OK) nextState = DMUNIPZ_STATE_ERROR;

  // state change: update info
  if (actState != nextState) {  
    mprintf("dm-unipz: changed to state %d\n", nextState);
    *pSharedState = nextState;
  }
  // finaly ...
  actState = nextState;
  reqState = DMUNIPZ_STATE_UNKNOWN; //check
} //changeState


uint32_t doActionOperation(uint32_t *statusTransfer, uint32_t *virtAcc, uint32_t *nTransfer)
{
  uint32_t i;
  uint32_t status;
  uint32_t nextAction;
  uint32_t regValue;
  uint32_t virtAccTmp;

  status = actStatus; 

  nextAction = wait4ECAEvent(DMUNIPZ_DEFAULT_TIMEOUT, &virtAccTmp);

  switch (nextAction) 
    {
    case DMUNIPZ_ECADO_REQTK :                // request TK
      *virtAcc        = virtAccTmp;
      *statusTransfer = DMUNIPZ_TRANS_REQTK; 
      (*nTransfer)++;                 
      status = requestTK(DMUNIPZ_REQTIMEOUT, virtAccTmp, 0);  // talk to UNIPZ
      mprintf("dm-unipz: status requesting TK; status %d for virtAcc %d\n", status, virtAccTmp);
      if (status !=  DMUNIPZ_STATUS_OK) {}    // no error handling, see https://www-acc.gsi.de/wiki/FAIR/CCT/Minutes300317 
      replyRequestTK();                       // reply to DM 
      break;
    case DMUNIPZ_ECADO_REQBEAM :              // request beam from UNILAC
      *statusTransfer = *statusTransfer | DMUNIPZ_TRANS_REQBEAM;
      mprintf("dm-unipz: status requesting beam; status %d for virtAcc %d\n", status, virtAccTmp);

      requestBeam(DMUNIPZ_REQTIMEOUT);        // talk to UNIPZ
      enableFilterEvtMil(pMILPiggy);
      clearFifoEvtMil(pMILPiggy);
      status = wait4MILEvt(DMUNIPZ_EVT_UNI_READY, virtAccTmp, DMUNIPZ_REQTIMEOUT); // wait for MIL Event
      /* todo/check: get WR timestamp for DM */                                    // timestamp MIL event
      pulseLemo2();                                                                // for hardware debugging with scope
        
      if (status == DMUNIPZ_STATUS_OK) *statusTransfer = *statusTransfer | DMUNIPZ_TRANS_SUCCESS;
      else                             *statusTransfer = *statusTransfer | DMUNIPZ_TRANS_FAIL;
        
      disableFilterEvtMil(pMILPiggy);
        
      mprintf("dm-unipz: status waiting for beam; status %d for virtAcc %d\n", status, virtAccTmp);

      if (status !=  DMUNIPZ_STATUS_OK) {}    // no error handling, see https://www-acc.gsi.de/wiki/FAIR/CCT/Minutes300317
      replyRequestBeam();                     // reply to DM
      releaseBeam();                          // release beam request
      releaseTK();                            // release TK 
      
      // the following is a hack for auto-recovery in case no beam could be delivered, chk
      checkClearReqNotOk(DMUNIPZ_REQTIMEOUT); // in case a 'req_not_ok' flag has been set at UNIPZ, try to clear it

      break;
    default: ;
    } // switch nextAction

  return status;
} //doActionOperation


void main(void) {
  
  uint32_t i,j;

  uint32_t inSync;
  uint32_t dT;
  uint16_t test;
  int      status;
  uint32_t statusTransfer;
  uint32_t nTransfer;
  uint32_t virtAcc;


  init();                        // initialize stuff for lm32
  initSharedMem();               // initialize shared memory
  initState();                   // init state machine
  ebmInit();                     // init EB master 
  ebmClearSharedMem();           // clear shared memory used for EB return values
  
  status    = DMUNIPZ_STATUS_OK;       // init (error) status 
  actStatus = DMUNIPZ_STATUS_UNKNOWN;  // init actual (error) status 

  findECAQueue();                // find WB device, required to receive events from the ECA
  findECAControl();              // find WB device, required to obtain timestamp
  findMILPiggy();                // find WB device, required for device bus master and event bus slave

  initCmds();                    // init command handler

  i=0;
  nTransfer = 0;

  while (1) {
    cmdHandler();    // check for commands and possibly request state changes
    changeState();   // handle requested state changes
    
    switch(actState) // state specific do actions
      {
      case DMUNIPZ_STATE_OPERATION :
        status = doActionOperation(&statusTransfer, &virtAcc, &nTransfer);
        //mprintf("dm-unipz: status transfer %d, virtual accelerator %d, 
        break;
      case DMUNIPZ_STATE_FATAL :
        mprintf("dm-unipz: a FATAL error has occured. Good bye.\n");
        while (1) asm("nop"); // RIP!
        break;
      default :
        for (j = 0; j < (DMUNIPZ_DEFAULT_TIMEOUT * DMUNIPZ_MS_ASMNOP); j++) { asm("nop"); } // wait: use value for default timeout
      } // switch 

    // update status and counter for main loop
    if (actStatus != status) {
      *pSharedStatus = status;
      actStatus = status;
    }
    i++; *pSharedNIterMain = i;
    *pSharedStatTrans = statusTransfer;
    *pSharedVirtAcc   = virtAcc;
    *pSharedNTransfer = nTransfer;
  } // while
} /* main */
