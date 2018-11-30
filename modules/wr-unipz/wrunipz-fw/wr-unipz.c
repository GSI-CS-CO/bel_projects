/********************************************************************************************
 *  wr-unipz.c
 *
 *  created : 2018
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 30-Nov-2018
 *
 *  lm32 program for gateway between UNILAC Pulszentrale and a White Rabbit network
 *  this basically serves a Data Master for UNILAC
 * 
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2019  Dietrich Beck
 * GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 * Planckstrasse 1
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
 * Last update: 22-November-2018
 ********************************************************************************************/
#define WRUNIPZ_FW_VERSION 0x000001                                     // make this consistent with makefile

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
#include "../../../top/gsi_scu/scu_mil.h"                               // register layout of 'MIL macro'
#include "../../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"   // register layout ECA queue
#include "../../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"         // register layout ECA control
/*                                                                      
#include "../../../ip_cores/wr-cores/modules/wr_pps_gen/pps_gen_regs.h" // useless register layout, I can't handle this wbgen stuff here
*/
#define WR_PPS_GEN_ESCR         0x1c                                    // External Sync Control Register
#define WR_PPS_GEN_ESCR_MASK    0xC                                     // bit 2: PPS valid, bit 3: timestamp valid

#define WR_ENDPOINT             0x650c2d4f                              // WR-endpoint
#define WR_ENDPOINT_MACHI       0x24                                    // MAC high bytes
#define WR_ENDPOINT_MACLO       0x28                                    // MAC low bytes

#include "../../../ip_cores/saftlib/drivers/eca_flags.h"                // definitions for ECA queue

#include "wr-unipz.h"                                                   // defs
#include "wr-unipz_smmap.h"                                             // shared memory map for communication via Wishbone

const char* dmunipz_state_text(uint32_t code) {
  switch (code) {
  case WRUNIPZ_STATE_UNKNOWN      : return "UNKNOWN   ";
  case WRUNIPZ_STATE_S0           : return "S0        ";
  case WRUNIPZ_STATE_IDLE         : return "IDLE      ";                                       
  case WRUNIPZ_STATE_CONFIGURED   : return "CONFIGURED";
  case WRUNIPZ_STATE_OPREADY      : return "opReady   ";
  case WRUNIPZ_STATE_STOPPING     : return "STOPPING  ";
  case WRUNIPZ_STATE_ERROR        : return "ERROR     ";
  case WRUNIPZ_STATE_FATAL        : return "FATAL(RIP)";
  default                         : return "undefined ";
  }
}


// stuff required for environment
extern uint32_t* _startshared[];
unsigned int     cpuId, cpuQty;
#define  SHARED  __attribute__((section(".shared")))
uint64_t SHARED  dummy = 0;

// global variables 
volatile uint32_t *pECAQ;               // WB address of ECA queue
volatile uint32_t *pMILPiggy;           // WB address of MIL device bus (MIL piggy)
volatile uint32_t *pPPSGen;             // WB address of PPS Gen
volatile uint32_t *pWREp;               // WB address of WR Endpoint

volatile uint32_t *pShared;             // pointer to begin of shared memory region
uint32_t *pSharedVersion;               // pointer to a "user defined" u32 register; here: publish version
uint32_t *pSharedStatus;                // pointer to a "user defined" u32 register; here: publish status
uint32_t *pSharedNBadStatus;            // pointer to a "user defined" u32 register; here: publish # of bad status (=error) incidents
uint32_t *pSharedNBadState;             // pointer to a "user defined" u32 register; here: publish # of bad state (=FATAL, ERROR, UNKNOWN) incidents
volatile uint32_t *pSharedCmd;          // pointer to a "user defined" u32 register; here: get command from host
uint32_t *pSharedState;                 // pointer to a "user defined" u32 register; here: publish status
volatile uint32_t *pSharedData4EB;      // pointer to a n x u32 register; here: memory region for receiving EB return values

uint32_t *pCpuRamExternal;              // external address (seen from host bridge) of this CPU's RAM            
uint32_t *pCpuRamExternalData4EB;       // external address (seen from host bridge) of this CPU's RAM: field for EB return values

uint32_t nBadStatus;                    // # of bad status (=error) incidents
uint32_t nBadState;                     // # of bad state (=FATAL, ERROR, UNKNOWN) incidents
dataTable bigData[WRUNIPZ_NPZ][WRUNIPZ_NVIRTACC]; // data 


uint64_t wrGetMac() // get my own MAC
{
  uint32_t macHi, macLo;
  uint64_t mac;

  macHi = (*(pWREp + (WR_ENDPOINT_MACHI >> 2))) & 0xffff;
  macLo = *(pWREp + (WR_ENDPOINT_MACLO >> 2));

  mac = macHi;
  mac = (mac << 32);
  mac = mac + macLo;

  mprintf("bla %x %x %x\n", macHi, macLo, pWREp);

  return mac;
} // wrGetMac


uint32_t ebmInit(uint32_t msTimeout, uint64_t dstMac, uint32_t dstIp, uint32_t eb_ops) // intialize Etherbone master
{
  uint64_t timeoutT;

  // check IP
  timeoutT = getSysTime() + (uint64_t)msTimeout * (uint64_t)1000000;
  while (timeoutT < getSysTime()) {
    if (*(pEbCfg + (EBC_SRC_IP>>2)) == EBC_DEFAULT_IP) asm("nop");
    else break;
  } // while no IP via DHCP
  if (*(pEbCfg + (EBC_SRC_IP>>2)) == EBC_DEFAULT_IP) return WRUNIPZ_STATUS_NOIP;

  // init ebm
  ebm_init();
  ebm_config_if(DESTINATION, dstMac    , dstIp,                       0xebd0); 
  ebm_config_if(SOURCE,      wrGetMac(), *(pEbCfg + (EBC_SRC_IP>>2)), 0xebd0); 
  ebm_config_meta(1500, 0x0, eb_ops);
  ebm_clr();

  return WRUNIPZ_STATUS_OK;
} // ebminit


/*
void ebmClearSharedMem() // clear shared memory used for EB return values
{
  uint32_t i;

  for (i=0; i< (WRUNIPZ_SHARED_DATA_4EB_SIZE >> 2); i++) pSharedData4EB[i] = 0x0;
} //ebmClearSharedMem


uint32_t ebmReadN(uint32_t msTimeout, uint32_t address, uint32_t *data, uint32_t n32BitWords)
{
  uint64_t timeoutT;
  int      i;
  uint32_t handshakeIdx;

  handshakeIdx = n32BitWords + 1;

  if (n32BitWords >= (WRUNIPZ_SHARED_DATA_4EB_SIZE >> 2)) return WRUNIPZ_STATUS_OUTOFRANGE;
  if (n32BitWords == 0)                                   return WRUNIPZ_STATUS_OUTOFRANGE;

  for (i=0; i< n32BitWords; i++) data[i] = 0x0;

  ebmClearSharedMem();                                                                               // clear shared data for EB return values
  pSharedData4EB[handshakeIdx] = WRUNIPZ_EB_HACKISH;                                                 // see below

  ebm_hi(address);                                                                                   // EB operation starts here
  for (i=0; i<n32BitWords; i++) ebm_op(address + i*4, (uint32_t)(&(pCpuRamExternalData4EB[i])), EBM_READ); // put data into EB cycle
                                ebm_op(address      , (uint32_t)(&(pCpuRamExternalData4EB[handshakeIdx])), EBM_READ); // handshake data
  ebm_flush();                                                                                       // commit EB cycle via the network
  
  timeoutT = getSysTime() + (uint64_t)msTimeout * (uint64_t)1000000;                                                     
  while (getSysTime() < timeoutT) {                                                                  // wait for received data until timeout
    if (pSharedData4EB[handshakeIdx] != WRUNIPZ_EB_HACKISH) {                                        // hackish solution to determine if a reply value has been received
      for (i=0; i<n32BitWords; i++) data[i] = pSharedData4EB[i];
      // dbg mprintf("wr-unipz: ebmReadN EB_address 0x%08x, nWords %d, data[0] 0x%08x, hackish 0x%08x, return 0x%08x\n", address, n32BitWords, data[0], WRUNIPZ_EB_HACKISH, pSharedData4EB[handshakeIdx]);
      return WRUNIPZ_STATUS_OK;
    }
  } //while not timed out

  return WRUNIPZ_STATUS_EBREADTIMEDOUT; 
} //ebmReadN


uint32_t ebmWriteN(uint32_t address, uint32_t *data, uint32_t n32BitWords)
{
  int i;

  if (n32BitWords > (WRUNIPZ_SHARED_DATA_4EB_SIZE >> 2)) return WRUNIPZ_STATUS_OUTOFRANGE;
  if (n32BitWords == 0)                                  return WRUNIPZ_STATUS_OUTOFRANGE;

  ebmClearSharedMem();                                                      // clear my shared memory used for EB replies

  ebm_hi(address);                                                          // EB operation starts here
  for (i=0; i<n32BitWords; i++) ebm_op(address + i*4, data[i], EBM_WRITE);  // put data into EB cycle
  if (n32BitWords == 1)         ebm_op(address      , data[0], EBM_WRITE);  // workaround runt frame issue
  ebm_flush();                                                              // commit EB cycle via the network
  
  return WRUNIPZ_STATUS_OK;
} // ebmWriteN
*/


uint32_t data2TM(uint32_t *idLo, uint32_t *idHi, uint32_t *paramLo, uint32_t *paramHi, uint32_t *res, uint32_t *tef, uint32_t *offset, uint32_t data, uint32_t gid, uint32_t virtAcc)  //converts event UNILAC event data to timing message
{
  uint32_t  t;
  uint32_t  evtCode;
  uint32_t  status;

  t        = (uint32_t)((data >> 16) & 0xffff);      // get time relative to begining of UNILAC cycle [us]
  evtCode  = (uint32_t)(data & 0xff);                // get event number
  status   = (uint32_t)((data >> 6) & 0xff);         // get status info

  *idHi     = (uint32_t)(                             // EventID
                         0x1     << 28     |         // FID = 1
                         (gid     << 16)    |         // GID
                         (evtCode <<  4)    |         // EVTNO
                         0x0                         // flags
                        );
  *idLo     = (uint32_t)(
                         (virtAcc << 20)    |         // SID
                         (0x0     <<  6)    |         // BPID
                         (0x0     <<  5)    |         // reserved
                         (0x0     <<  4)    |         // reqNoBeam
                         0x0                          // virtAcc only for DM-UNIPZ gateway
                        );
  *paramLo  = status;                                 // parameter field
  *paramHi  = 0x0;                                 
  *res      = 0x0;                                    // reserved
  *tef      = 0x0;                                    // timing extension field
  *offset   = t * 1000;                               // convert offset -> ns
} // data2TM


uint32_t ebmWriteTM(dataTable evts, uint64_t tStart, uint32_t nPz, uint32_t virtAcc)
{
  int      i;
  uint64_t deadline;
  uint32_t res, tef;
  uint32_t deadlineLo, deadlineHi, offset;
  uint32_t idLo, idHi;
  uint32_t paramLo, paramHi;

  // set high bits for EB master
  ebm_hi(WRUNIPZ_ECA_ADDRESS);

  // pack Ethernet frame with messages
  for (i=0; i<WRUNIPZ_NEVTMAX; i++) {                 // loop over all data fields
    if (evts.validFlags & (1 << i)) {                 // data is valid?
      if (evts.evtFlags & (1 << i)) {                 // data is an event?

        // convert data
        data2TM(&idLo, &idHi, &paramLo, &paramHi, &res, &tef, &offset, evts.data[i], gid[nPz], virtAcc);  //convert data

        // calc deadline
        deadline   = tStart + (uint64_t)offset;       
        deadlineHi = (uint32_t)((deadline >> 32) & 0xffffffff);
        deadlineLo = (uint32_t)(deadline & 0xffffffff);

        //
        atomic_on();                                  
        ebm_op(WRUNIPZ_ECA_ADDRESS, idHi,       EBM_WRITE);             
        ebm_op(WRUNIPZ_ECA_ADDRESS, idLo,       EBM_WRITE);             
        ebm_op(WRUNIPZ_ECA_ADDRESS, paramHi,    EBM_WRITE);
        ebm_op(WRUNIPZ_ECA_ADDRESS, paramLo,    EBM_WRITE);
        ebm_op(WRUNIPZ_ECA_ADDRESS, tef,        EBM_WRITE);
        ebm_op(WRUNIPZ_ECA_ADDRESS, res,        EBM_WRITE);
        ebm_op(WRUNIPZ_ECA_ADDRESS, deadlineHi, EBM_WRITE);
        ebm_op(WRUNIPZ_ECA_ADDRESS, deadlineLo, EBM_WRITE);
        atomic_off();
      } // is event
    } // is valid
  } // for i

  // send Ethernet frame
  ebm_flush();

  return WRUNIPZ_STATUS_OK;
} //ebmWriteTM


uint32_t wrCheckSyncState() //check status of White Rabbit (link up, tracking)
{
  uint32_t syncState;

  syncState =  *(pPPSGen + (WR_PPS_GEN_ESCR >> 2));                         // read status
  syncState = syncState & WR_PPS_GEN_ESCR_MASK;                             // apply mask

  if ((syncState == WR_PPS_GEN_ESCR_MASK)) return WRUNIPZ_STATUS_OK;        // check if all relevant bits are set
  else                                     return WRUNIPZ_STATUS_WRBADSYNC;
} //wrCheckStatus


void init() // typical init for lm32
{
  discoverPeriphery();        // mini-sdb ...
  uart_init_hw();             // needed by WR console   
  cpuId = getCpuIdx();

  timer_init(1);              // needed by usleep_init() 
  usleep_init();              // needed by scu_mil.c

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

  // get address to data
  pSharedVersion      = (uint32_t *)(pShared + (WRUNIPZ_SHARED_VERSION >> 2));
  pSharedStatus       = (uint32_t *)(pShared + (WRUNIPZ_SHARED_STATUS >> 2));
  pSharedCmd          = (uint32_t *)(pShared + (WRUNIPZ_SHARED_CMD >> 2));
  pSharedState        = (uint32_t *)(pShared + (WRUNIPZ_SHARED_STATE >> 2));
  pSharedData4EB      = (uint32_t *)(pShared + (WRUNIPZ_SHARED_DATA_4EB_START >> 2));
  pSharedNBadStatus   = (uint32_t *)(pShared + (WRUNIPZ_SHARED_NBADSTATUS >> 2));
  pSharedNBadState    = (uint32_t *)(pShared + (WRUNIPZ_SHARED_NBADSTATE >> 2));  
  
  // find address of CPU from external perspective
  idx = 0;
  find_device_multi(&found_clu, &idx, 1, GSI, LM32_CB_CLUSTER);	
  idx = 0;
  find_device_multi_in_subtree(&found_clu, &found_sdb[0], &idx, c_Max_Rams, GSI, LM32_RAM_USER);
  if(idx >= cpuId) {
    pCpuRamExternal           = (uint32_t *)(getSdbAdr(&found_sdb[cpuId]) & 0x7FFFFFFF); // CPU sees the 'world' under 0x8..., remove that bit to get host bridge perspective
    pCpuRamExternalData4EB    = (uint32_t *)(pCpuRamExternal + ((WRUNIPZ_SHARED_DATA_4EB_START + SHARED_OFFS) >> 2));
  }

  DBPRINT2("wr-unipz: CPU RAM External 0x%8x, begin shared 0x%08x\n", pCpuRamExternal, SHARED_OFFS);

  // set initial values;
                                /* ebmClearSharedMem(); */
  *pSharedVersion    = WRUNIPZ_FW_VERSION; // of all the shared variabes, only VERSION is a constant. Set it now!
  *pSharedNBadStatus = 0;
  *pSharedNBadState  = 0;
} // initSharedMem 


uint32_t findMILPiggy() //find WB address of MIL Piggy
{
  pMILPiggy = 0x0;
  
  // get Wishbone address for MIL Piggy
  pMILPiggy = find_device_adr(GSI, SCU_MIL);

  if (!pMILPiggy) {DBPRINT1("wr-unipz: can't find MIL piggy\n"); return WRUNIPZ_STATUS_ERROR;}
  else                                                           return WRUNIPZ_STATUS_OK;
} // findMILPiggy


uint32_t findPPSGen() //find WB address of WR PPS Gen
{
  pPPSGen = 0x0;
  
  // get Wishbone address for PPS Gen
  pPPSGen = find_device_adr(CERN, WR_PPS_GEN);

  if (!pPPSGen) {DBPRINT1("wr-unipz: can't find WR PPS Gen\n"); return WRUNIPZ_STATUS_ERROR;}
  else                                                          return WRUNIPZ_STATUS_OK;
} // findPPSGen


uint32_t findWREp() //find WB address of WR Endpoint
{
  pWREp = 0x0;
  
  // get Wishbone address for WR Endpoint
  pWREp = find_device_adr(CERN, WR_ENDPOINT);

  if (!pWREp) {DBPRINT1("wr-unipz: can't find WR Endpoint\n"); return WRUNIPZ_STATUS_ERROR;}
  else                                                         return WRUNIPZ_STATUS_OK;
} // findWREp


uint32_t findECAQueue() // find WB address of ECA channel for LM32
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

  if (!pECAQ) {DBPRINT1("wr-unipz: can't find ECA queue\n"); return WRUNIPZ_STATUS_ERROR;}
  else                                                       return WRUNIPZ_STATUS_OK;
} // findECAQueue


uint32_t wait4ECAEvent(uint32_t msTimeout, uint64_t *deadline, uint32_t *isLate)  // 1. query ECA for actions, 2. trigger activity
{
  uint32_t *pECAFlag;           // address of ECA flag
  uint32_t evtIdHigh;           // high 32bit of eventID   
  uint32_t evtIdLow;            // low 32bit of eventID    
  uint32_t evtDeadlHigh;        // high 32bit of deadline  
  uint32_t evtDeadlLow;         // low 32bit of deadline   
  uint32_t evtParamHigh;        // high 32 bit of parameter field
  uint32_t evtParamLow ;        // low 32 bit of parameter field
  uint32_t evtTef;              // 32 bit TEF field
  uint32_t actTag;              // tag of action           
  uint32_t nextAction;          // describes what to do next
  uint64_t timeoutT;            // when to time out

  pECAFlag    = (uint32_t *)(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2));   // address of ECA flag
  timeoutT    = getSysTime() + (uint64_t)msTimeout * (uint64_t)1000000;

  while (getSysTime() < timeoutT) {
    if (*pECAFlag & (0x0001 << ECA_VALID)) {                        // if ECA data is valid

      // read data
      evtIdHigh    = *(pECAQ + (ECA_QUEUE_EVENT_ID_HI_GET >> 2));
      evtIdLow     = *(pECAQ + (ECA_QUEUE_EVENT_ID_LO_GET >> 2));
      evtDeadlHigh = *(pECAQ + (ECA_QUEUE_DEADLINE_HI_GET >> 2));
      evtDeadlLow  = *(pECAQ + (ECA_QUEUE_DEADLINE_LO_GET >> 2));
      actTag       = *(pECAQ + (ECA_QUEUE_TAG_GET >> 2));
      evtParamHigh = *(pECAQ + (ECA_QUEUE_PARAM_HI_GET >> 2));
      evtParamLow  = *(pECAQ + (ECA_QUEUE_PARAM_LO_GET >> 2));
      evtTef       = *(pECAQ + (ECA_QUEUE_TEF_GET >> 2));
      *isLate      = *pECAFlag & (0x0001 << ECA_LATE);

      *deadline    = ((uint64_t)evtDeadlHigh << 32) + (uint64_t)evtDeadlLow;
      
      // pop action from channel
      *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;

      // here: do s.th. according to tag
      switch (actTag) 
        {
          /*        case WRUNIPZ_ECADO_READY2SIS :
          nextAction = WRUNIPZ_ECADO_READY2SIS;
          break;
        case  WRUNIPZ_ECADO_MBTRIGGER :
          nextAction = WRUNIPZ_ECADO_MBTRIGGER;
          break;*/
        default: 
          nextAction = WRUNIPZ_ECADO_UNKOWN;
        } // switch

      return nextAction;

    } // if data is valid
  } // while not timed out

  return  WRUNIPZ_ECADO_TIMEOUT;
} // wait for ECA event


void initCmds() // init stuff for handling commands, trivial for now, will be extended
{
  //  initalize command value: 0x0 means 'no command'
  *pSharedCmd     = 0x0;
} // initCmds

 
uint32_t configMILEvent(uint16_t evtCode) // configure SoC to receive events via MIL bus
{
  uint32_t i;

  // initialize status and command register with initial values; disable event filtering; clear filter RAM
  if (writeCtrlStatRegEvtMil(pMILPiggy, MIL_CTRL_STAT_ENDECODER_FPGA | MIL_CTRL_STAT_INTR_DEB_ON) != MIL_STAT_OK) return WRUNIPZ_STATUS_ERROR; //chk sure we go for status error?

  // clean up 
  if (disableLemoEvtMil(pMILPiggy, 1) != MIL_STAT_OK) return WRUNIPZ_STATUS_ERROR;
  if (disableLemoEvtMil(pMILPiggy, 2) != MIL_STAT_OK) return WRUNIPZ_STATUS_ERROR;
  if (disableFilterEvtMil(pMILPiggy)  != MIL_STAT_OK) return WRUNIPZ_STATUS_ERROR; 
  if (clearFilterEvtMil(pMILPiggy)    != MIL_STAT_OK) return WRUNIPZ_STATUS_ERROR; 

  for (i=0; i < (0xf+1); i++) {
    // set filter (FIFO and LEMO1 pulsing) for all possible virtual accelerators
    if (setFilterEvtMil(pMILPiggy,  evtCode, i, MIL_FILTER_EV_TO_FIFO | MIL_FILTER_EV_PULS1_S) != MIL_STAT_OK) return WRUNIPZ_STATUS_ERROR;
  }

  // configure LEMO1 for pulse generation
  if (configLemoPulseEvtMil(pMILPiggy, 1) != MIL_STAT_OK) return WRUNIPZ_STATUS_ERROR;

  return WRUNIPZ_STATUS_OK;
} // configMILEvent


uint32_t wait4MILEvent(uint16_t evtCode, uint16_t virtAccReq, uint32_t *virtAccRec, uint32_t msTimeout)  // wait for MIL event or timeout
{
  uint32_t evtDataRec;         // data of one MIL event
  uint32_t evtCodeRec;         // "event number"
  uint64_t timeoutT;           // when to time out
  uint32_t virtAccTmp;         // temp value for virtAcc

  timeoutT    = getSysTime() + (uint64_t)msTimeout * (uint64_t)1000000;
  *virtAccRec = 0;             // last two digits: virtacc; count other junk in FIFO by increasing by 100;

  while(getSysTime() < timeoutT) {              // while not timed out...
    while (fifoNotemptyEvtMil(pMILPiggy)) {     // while fifo contains data
      popFifoEvtMil(pMILPiggy, &evtDataRec);    
      evtCodeRec  = evtDataRec & 0x000000ff;    // extract event code
      virtAccTmp  = (evtDataRec >> 8) & 0x0f;   // extract virtual accelerator (assuming event message)

      *virtAccRec += 100;                       // increase by 100 for each FIFO entry
      
      if (evtCodeRec == evtCode) {
        *virtAccRec += virtAccTmp;
        if (virtAccTmp == virtAccReq) return WRUNIPZ_STATUS_OK;
        else                          return WRUNIPZ_STATUS_WRONGVIRTACC;
      } // if evtCode

      // chck mprintf("wr-unipz: virtAcc %03d, evtCode %03d\n", virtAcc, evtCodeRec);
    } // while fifo contains data
    asm("nop");                                 // wait a bit...
  } // while not timed out

  // up to here: no matching evtCode AND matching virtAcc
  *virtAccRec += 42;                            // mark with illegal virtAcc number
  return WRUNIPZ_STATUS_TIMEDOUT;  
} //wait4MILEvent


void pulseLemo2() //for debugging with scope
{
  uint32_t i;

  setLemoOutputEvtMil(pMILPiggy, 2, 1);
  for (i=0; i< 10 * WRUNIPZ_US_ASMNOP; i++) asm("nop");
  setLemoOutputEvtMil(pMILPiggy, 2, 0);
} // pulseLemo2


uint32_t doActionS0()
{
  uint32_t status = WRUNIPZ_STATUS_OK;

  if (findECAQueue() != WRUNIPZ_STATUS_OK) status = WRUNIPZ_STATUS_ERROR; 
  if (findMILPiggy() != WRUNIPZ_STATUS_OK) status = WRUNIPZ_STATUS_ERROR;
  if (findPPSGen()   != WRUNIPZ_STATUS_OK) status = WRUNIPZ_STATUS_ERROR;
  if (findWREp()     != WRUNIPZ_STATUS_OK) status = WRUNIPZ_STATUS_ERROR;
  initCmds();                    

  return status;
} // entryActionS0


uint32_t entryActionConfigured()
{
  uint32_t status = WRUNIPZ_STATUS_OK;
  uint32_t virtAcc;
  uint32_t dryRunFlag;
  uint32_t i;
  uint32_t data;
  uint64_t timestamp;
  uint32_t isLate;

  // configure EB master (SRC and DST MAC/IP are set from host)
  if ((status = ebmInit(2000, 0xffffffffffff, 0xffffffff, EBM_NOREPLY)) != WRUNIPZ_STATUS_OK) {
    DBPRINT1("wr-unipz: ERROR - init of EB master failed! %d\n", status);
    return status;
  } 
 
  // reset MIL piggy and wait
  /* comment while developping on exploder
  if ((status = resetPiggyDevMil(pMILPiggy))  != MIL_STAT_OK) {
    DBPRINT1("wr-unipz: ERROR - can't reset MIL Piggy\n");
    return WRUNIPZ_STATUS_DEVBUSERROR;
    } 
  
  // configure MIL piggy for timing events for all 16 virtual accelerators
  if ((status = configMILEvent(WRUNIPZ_EVT_DUMMY)) != WRUNIPZ_STATUS_OK) {
    DBPRINT1("wr-unipz: ERROR - failed to configure MIL piggy for receiving timing events! %d\n", status);
    return status;
  } 

  DBPRINT1("wr-unipz: MIL piggy configured for receving events (eventbus)\n");

  configLemoOutputEvtMil(pMILPiggy, 2);    // used to see a blinking LED (and optionally connect a scope) for debugging
  */
  // empty ECA queue for lm32
  i = 0;
  while (wait4ECAEvent(1, &timestamp, &isLate) !=  WRUNIPZ_ECADO_TIMEOUT) {i++;}
  DBPRINT1("wr-unipz: ECA queue flushed - removed %d pending entries from ECA queue\n", i);

  return status;
} // entryActionConfigured


uint32_t entryActionOperation()
{
  return WRUNIPZ_STATUS_OK;
} // entryActionOperation


uint32_t exitActionOperation()
{/* comment while developing on exploder 
    if (disableFilterEvtMil(pMILPiggy) != MIL_STAT_OK) return WRUNIPZ_STATUS_ERROR; */
  
  return WRUNIPZ_STATUS_OK;
} // exitActionOperation

uint32_t exitActionError()
{
  return WRUNIPZ_STATUS_OK;
} // exitActionError


void cmdHandler(uint32_t *reqState, uint32_t *statusTransfer) // handle commands from the outside world
{
  uint32_t cmd;

  cmd = *pSharedCmd;
  // check, if the command is valid and request state change
  if (cmd) {
    switch (cmd) {
    case WRUNIPZ_CMD_CONFIGURE :
      *reqState =  WRUNIPZ_STATE_CONFIGURED;
      DBPRINT3("wr-unipz: received cmd %d\n", cmd);
      break;
    case WRUNIPZ_CMD_STARTOP :
      *reqState = WRUNIPZ_STATE_OPREADY;
      DBPRINT3("wr-unipz: received cmd %d\n", cmd);
      break;
    case WRUNIPZ_CMD_STOPOP :
      *reqState = WRUNIPZ_STATE_STOPPING;
      DBPRINT3("wr-unipz: received cmd %d\n", cmd);
      break;
    case WRUNIPZ_CMD_IDLE :
      *reqState = WRUNIPZ_STATE_IDLE;
      DBPRINT3("wr-unipz: received cmd %d\n", cmd);
      break;
    case WRUNIPZ_CMD_RECOVER :
      *reqState = WRUNIPZ_STATE_IDLE;
      DBPRINT3("wr-unipz: received cmd %d\n", cmd);
      break;
    default:
      DBPRINT3("wr-unipz: received unknown command '0x%08x'\n", cmd);
    } // switch 
    *pSharedCmd = 0x0; // reset cmd value in shared memory 
  } // if command 
} // cmdHandler


uint32_t changeState(uint32_t *actState, uint32_t *reqState, uint32_t actStatus)   //state machine; see wr-unipz.h for possible states and transitions
{
  uint32_t statusTransition= WRUNIPZ_STATUS_OK;
  uint32_t status;
  uint32_t nextState;                   

  // if something severe happened, perform implicitely allowed transition to ERROR or FATAL states
  // else                        , handle explicitcely allowed transitions

  if ((*reqState == WRUNIPZ_STATE_ERROR) || (*reqState == WRUNIPZ_STATE_FATAL)) {statusTransition = actStatus; nextState = *reqState;}
  else {
    nextState = *actState;                       // per default: remain in actual state without exit or entry action
    switch (*actState) {                         // check for allowed transitions: 1. determine next state, 2. perform exit or entry actions if required
    case WRUNIPZ_STATE_S0:
      if      (*reqState == WRUNIPZ_STATE_IDLE)       {                                            nextState = *reqState;}      
      break;
    case WRUNIPZ_STATE_IDLE:
      if      (*reqState == WRUNIPZ_STATE_CONFIGURED)  {statusTransition = entryActionConfigured(); nextState = *reqState;}
      break;
    case WRUNIPZ_STATE_CONFIGURED:
      if      (*reqState == WRUNIPZ_STATE_IDLE)       {                                            nextState = *reqState;}
      else if (*reqState == WRUNIPZ_STATE_CONFIGURED) {statusTransition = entryActionConfigured(); nextState = *reqState;}
      else if (*reqState == WRUNIPZ_STATE_OPREADY)    {statusTransition = entryActionOperation();  nextState = *reqState;}
      break;
    case WRUNIPZ_STATE_OPREADY:
      if      (*reqState == WRUNIPZ_STATE_STOPPING)   {statusTransition = exitActionOperation();   nextState = *reqState;}
      break;
    case WRUNIPZ_STATE_STOPPING:
      nextState = WRUNIPZ_STATE_CONFIGURED;      //automatic transition but without entryActionConfigured
    case WRUNIPZ_STATE_ERROR:
      if      (*reqState == WRUNIPZ_STATE_IDLE)       {statusTransition = exitActionError();       nextState = *reqState;}
      break;
    default: 
      nextState = WRUNIPZ_STATE_S0;
    } // switch actState
  }  // else something severe happened
  
  // if the transition failed, transit to error state (except we are already in FATAL state)
  if ((statusTransition != WRUNIPZ_STATUS_OK) && (nextState != WRUNIPZ_STATE_FATAL)) nextState = WRUNIPZ_STATE_ERROR;

  // if the state changes
  if (*actState != nextState) {                   
    mprintf("wr-unipz: changed to state %d\n", nextState);
    *actState = nextState;                      
    status = statusTransition;
  } // if state change
  else  status = actStatus;

  *reqState = WRUNIPZ_STATE_UNKNOWN;             // reset requested state (= no change state requested)  

  return status;
} //changeState


uint32_t doActionOperation(uint32_t *nCycle,                  // total number of UNILAC cycle since FW start
                           uint32_t actStatus)                // actual status of firmware
{
  uint32_t status;                                                                 // status returned by routines
  uint32_t flagMilEvtValid;                                                        // flag indicating that we recevied a valid MIL event
  uint32_t flagIsLate;                                                             // flag indicating that we received a 'late' event from ECA
  uint32_t nextAction;                                                             // action triggered by event received from ECA
  uint64_t deadline;                                                               // deadline of event received via ECA

  status = actStatus; 

  nextAction = wait4ECAEvent(WRUNIPZ_DEFAULT_TIMEOUT, &deadline, &flagIsLate);     // 'do action' is driven by actions issued by the ECA

  switch (nextAction) 
    {
    case WRUNIPZ_ECADO_DUMMY :                                                     // whatever
      ; // do something

      break;
    
    default:
      ; // do something
    } // switch nextAction

  return status;
} // doActionOperation

uint32_t doAutoRecovery(uint32_t actState, uint32_t *reqState)                    // do autorecovery from error state
{
  switch (actState)
    {
    case WRUNIPZ_STATE_ERROR :
      DBPRINT3("wr-unipz: attempting autorecovery ERROR -> IDLE\n");
      usleep(10000000);
      *reqState = WRUNIPZ_STATE_IDLE; 
      break;
    case WRUNIPZ_STATE_IDLE :
      DBPRINT3("wr-unipz: attempting autorecovery IDLE -> CONFIGURED\n");
      usleep(5000000);
      *reqState =  WRUNIPZ_STATE_CONFIGURED;
      break;
    case WRUNIPZ_STATE_CONFIGURED :
      DBPRINT3("wr-unipz: attempting autorecovery CONFIGURED -> OPREADY\n");
      usleep(5000000);
      *reqState =  WRUNIPZ_STATE_OPREADY;
      break;
    default : ;
    } // switch actState
} // doAutoRecovery


void main(void) {
 
  uint32_t j;
 
  uint32_t i;                                   // counter for iterations of main loop
  uint32_t status;                              // (error) status
  uint32_t actState;                            // actual FSM state
  uint32_t reqState;                            // requested FSM state
  uint32_t flagRecover;                         // flag indicating auto-recovery from error state;

  uint32_t statusCycle;                         // status of cycle
  uint32_t nCycle;                              // number of cycles

  mprintf("\n");
  mprintf("wr-unipz: ***** firmware v %06d started from scratch *****\n", WRUNIPZ_FW_VERSION);
  mprintf("\n");
  
  // init local variables
  i              = 0;
  nCycle         = 0;                           
  reqState       = WRUNIPZ_STATE_S0;
  actState       = WRUNIPZ_STATE_UNKNOWN;
  status         = WRUNIPZ_STATUS_UNKNOWN;
  nBadState      = 0;
  nBadStatus     = 0;
  flagRecover    = 0;

  init();                                                                   // initialize stuff for lm32
  initSharedMem();                                                          // initialize shared memory

  /* 
  
  while (1) {
    cmdHandler(&reqState, &statusTransfer);                                 // check for commands and possibly request state changes
    status = changeState(&actState, &reqState, status);                     // handle requested state changes
    switch(actState)                                                        // state specific do actions
      {
      case WRUNIPZ_STATE_S0 :
        status = doActionS0();                                              // important initialization that must succeed!
        if (status != WRUNIPZ_STATUS_OK) reqState = WRUNIPZ_STATE_FATAL;    // failed:  -> FATAL
        else                             reqState = WRUNIPZ_STATE_IDLE;     // success: -> IDLE
        break;
      case WRUNIPZ_STATE_OPREADY :
        flagRecover = 0;
        status = doActionOperation(&statusTransfer, &virtAccReq, &virtAccRec, &noBeam, &dtStart, &dtSync, &dtInject, &dtTransfer, &dtTkreq, &dtBreq, &dtBprep, &dtReady2Sis, &nTransfer, &nInject, status);
        if (status == WRUNIPZ_STATUS_WRBADSYNC)      reqState = WRUNIPZ_STATE_ERROR;
        if (status == WRUNIPZ_STATUS_ERROR)          reqState = WRUNIPZ_STATE_ERROR;
        break;
      case WRUNIPZ_STATE_ERROR :
        flagRecover = 1;                                                    // start autorecovery
        break; 
      case WRUNIPZ_STATE_FATAL :
        *pSharedState  = actState;
        *pSharedStatus = status;
        mprintf("wr-unipz: a FATAL error has occured. Good bye.\n");
        while (1) asm("nop"); // RIP!
        break;
      default :                                                             // avoid flooding WB bus with unnecessary activity
        for (j = 0; j < (WRUNIPZ_DEFAULT_TIMEOUT * WRUNIPZ_MS_ASMNOP); j++) { asm("nop"); }
      } // switch

    // autorecovery from state ERROR
    if (flagRecover) doAutoRecovery(actState, &reqState);

    // update shared memory 
    if ((*pSharedStatus == WRUNIPZ_STATUS_OK)     && (status    != WRUNIPZ_STATUS_OK))     {nBadStatus++; *pSharedNBadStatus = nBadStatus;}
    if ((*pSharedState  == WRUNIPZ_STATE_OPREADY) && (actState  != WRUNIPZ_STATE_OPREADY)) {nBadState++;  *pSharedNBadState  = nBadState;}
    *pSharedStatus       = status;
    *pSharedState        = actState;
    i++;
  
  } // while   */
  
  /*  ebmInit(0);
  ebmClearSharedMem();
  // ebmReadN(2000, 0x60308, test, 1); 
  //*pSharedSrcMacHi = test[0];
  usleep(10000); */

  uint64_t d1;

  bigData[0][0].validFlags  = 0xffffffff;
  bigData[0][0].prepFlags   = 0x0f;
  bigData[0][0].evtFlags    = 0xffffffff;
  bigData[0][0].data[0]     = 0x00010010;
  bigData[0][0].data[1]     = 0x00020011; 
  bigData[0][0].data[2]     = 0x00030012;
  bigData[0][0].data[3]     = 0x00040013;
  bigData[0][0].data[4]     = 0x00050014;
  bigData[0][0].data[5]     = 0x00060015;
  bigData[0][0].data[6]     = 0x00070016;
  bigData[0][0].data[7]     = 0x00080017;
  bigData[0][0].data[8]     = 0x00110110;
  bigData[0][0].data[9]     = 0x00120111; 
  bigData[0][0].data[10]    = 0x00130112;
  bigData[0][0].data[11]    = 0x00140113;
  bigData[0][0].data[12]    = 0x00150114;
  bigData[0][0].data[13]    = 0x00160115;
  bigData[0][0].data[14]    = 0x00170116;
  bigData[0][0].data[15]    = 0x00180117;
  bigData[0][0].data[16]    = 0x00210210;
  bigData[0][0].data[17]    = 0x00220211; 
  bigData[0][0].data[18]    = 0x00230212;
  bigData[0][0].data[19]    = 0x00240213;
  bigData[0][0].data[20]    = 0x00250214;
  bigData[0][0].data[21]    = 0x00260215;
  bigData[0][0].data[22]    = 0x00270216;
  bigData[0][0].data[23]    = 0x00280217;
  bigData[0][0].data[24]    = 0x00310310;
  bigData[0][0].data[25]    = 0x00320311; 
  bigData[0][0].data[26]    = 0x00330312;
  bigData[0][0].data[27]    = 0x00340313;
  bigData[0][0].data[28]    = 0x00350314;
  bigData[0][0].data[29]    = 0x00360315;
  bigData[0][0].data[30]    = 0x00370316;
  bigData[0][0].data[31]    = 0x00380317;
  

  
  status = doActionS0();                                
  ebmInit(100, 0xffffffffffff, 0xffffffff, EBM_NOREPLY);
  
  ebm_clr();

  d1 = getSysTime();
  ebmWriteTM(bigData[0][0], d1, 0, 12);
} // main
