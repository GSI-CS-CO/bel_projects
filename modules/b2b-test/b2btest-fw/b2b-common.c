/********************************************************************************************
 *  b2b-common.c
 *
 *  created : 2019
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 30-Apr-2019
 *
 *  common functions used by various B2B firmware
 *  
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2018  Dietrich Beck
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
 * Last update: 24-April-2019
 ********************************************************************************************/

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

/* includes specific for bel_projects */
#include "irq.h"
#include "ebm.h"
#include "aux.h"
#include "dbg.h"
#include "mprintf.h"
#include "mini_sdb.h"
#include "syscon.h"
#include "../../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"   // register layout ECA queue
#include "../../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"         // register layout ECA control
// #include "../../../ip_cores/wr-cores/modules/wr_pps_gen/pps_gen_regs.h" // useless register layout, I can't handle this wbgen stuff here
#include "../../../ip_cores/saftlib/drivers/eca_flags.h"                // definitions for ECA queue
#include "../../../tools/wb_slaves.h"                                   // Dietrichs hackish solution for defining register layout

/* includes for this project */
#include <b2b-common.h>                                                 // common stuff for b2b

// these routines are typically application specific
extern void     clearDiag();
extern uint32_t entryActionConfigured();
extern uint32_t entryActionOperation();
extern uint32_t exitActionOperation();

volatile uint32_t *pECAQ;               // WB address of ECA queue
volatile uint32_t *pPPSGen;             // WB address of PPS Gen
volatile uint32_t *pWREp;               // WB address of WR Endpoint
volatile uint32_t *pIOCtrl;             // WB address of IO Control

// global variables
uint32_t *pSharedVersion;               // pointer to a "user defined" u32 register; here: publish version
uint32_t *pSharedSumStatus;             // pointer to a "user defined" u32 register; here: publish OR of all (actual) error bits
uint32_t *pSharedNBadStatus;            // pointer to a "user defined" u32 register; here: publish # of bad status (=error) incidents
uint32_t *pSharedNBadState;             // pointer to a "user defined" u32 register; here: publish # of bad state (=FATAL, ERROR, UNKNOWN) incidents
volatile uint32_t *pSharedCmd;          // pointer to a "user defined" u32 register; here: get command from host
uint32_t *pSharedState;                 // pointer to a "user defined" u32 register; here: publish status
volatile uint32_t *pSharedData4EB;      // pointer to a n x u32 register; here: memory region for receiving EB return values
uint32_t *pSharedMacHi;                 // pointer to a "user defined" u32 register; here: high bits of MAC
uint32_t *pSharedMacLo;                 // pointer to a "user defined" u32 register; here: low bits of MAC
uint32_t *pSharedIp;                    // pointer to a "user defined" u32 register; here: IP
uint32_t *pSharedTDiagHi;               // pointer to a "user defined" u32 register; here: time when diag was cleared, high bits
uint32_t *pSharedTDiagLo;               // pointer to a "user defined" u32 register; here: time when diag was cleared, low bits
uint32_t *pSharedTS0Hi;                 // pointer to a "user defined" u32 register; here: time when FW was in S0 state, high bits
uint32_t *pSharedTS0Lo;                 // pointer to a "user defined" u32 register; here: time when FW was in S0 state, low bits

uint32_t nBadStatus;                    // # of bad status (=error) incidents
uint32_t nBadState;                     // # of bad state (=FATAL, ERROR, UNKNOWN) incidents


uint64_t wrGetMac()  // get my own MAC
{
  uint32_t macHi, macLo;
  uint64_t mac;

  macHi = (*(pWREp + (WR_ENDPOINT_MACHI >> 2))) & 0xffff;
  macLo = *(pWREp + (WR_ENDPOINT_MACLO >> 2));

  mac = macHi;
  mac = (mac << 32);
  mac = mac + macLo;

  return mac;
} // wrGetMac


uint32_t common_ioCtrlSetGate(uint32_t enable, uint32_t io)  // set gate of LVDS input
{
  uint32_t offset;
  
  if ((enable != 1) && (enable != 0)) return COMMON_STATUS_OUTOFRANGE;
  if (io > 31)                        return COMMON_STATUS_OUTOFRANGE;

  if (enable) offset = IO_CTRL_LVDSINGATESETLOW;
  else        offset = IO_CTRL_LVDSINGATERESETLOW;

  *(pIOCtrl + (offset >> 2)) = (1 << io);

  return COMMON_STATUS_OK;
} // common_ioCtrlSetGate


uint32_t common_ebmInit(uint32_t msTimeout, uint64_t dstMac, uint32_t dstIp, uint32_t eb_ops) // intialize Etherbone master
{
  uint64_t timeoutT;

  // check IP
  timeoutT = getSysTime() + (uint64_t)msTimeout * (uint64_t)1000000;
  while (timeoutT < getSysTime()) {
    if (*(pEbCfg + (EBC_SRC_IP>>2)) == EBC_DEFAULT_IP) asm("nop");
    else break;
  } // while no IP via DHCP
  if (*(pEbCfg + (EBC_SRC_IP>>2)) == EBC_DEFAULT_IP) return COMMON_STATUS_NOIP;

  // init ebm
  ebm_init();
  ebm_config_if(DESTINATION, dstMac    , dstIp,                       0xebd0); 
  ebm_config_if(SOURCE,      wrGetMac(), *(pEbCfg + (EBC_SRC_IP>>2)), 0xebd0); 
  ebm_config_meta(1500, 0x0, eb_ops);
  ebm_clr();

  return COMMON_STATUS_OK;
} // ebminit


uint32_t common_ebmWriteTM(uint64_t deadline, uint64_t evtId, uint64_t param)  
{
  int      i;
  uint32_t res, tef;
  uint32_t deadlineLo, deadlineHi, offset;
  uint32_t idLo, idHi;
  uint32_t paramLo, paramHi;

  // set high bits for EB master
  ebm_hi(COMMON_ECA_ADDRESS);

  // pack Ethernet frame with messages
  idHi       = (uint32_t)((evtId >> 32)    & 0xffffffff);
  idLo       = (uint32_t)(evtId            & 0xffffffff);
  tef     = 0x00000000;
  res     = 0x00000000;
  paramHi    = (uint32_t)((param >> 32)    & 0xffffffff);
  paramLo    = (uint32_t)(param            & 0xffffffff);
  deadlineHi = (uint32_t)((deadline >> 32) & 0xffffffff);
  deadlineLo = (uint32_t)(deadline         & 0xffffffff);
          
  // pack timing message
  atomic_on();                                  
  ebm_op(COMMON_ECA_ADDRESS, idHi,       EBM_WRITE);             
  ebm_op(COMMON_ECA_ADDRESS, idLo,       EBM_WRITE);             
  ebm_op(COMMON_ECA_ADDRESS, paramHi,    EBM_WRITE);
  ebm_op(COMMON_ECA_ADDRESS, paramLo,    EBM_WRITE);
  ebm_op(COMMON_ECA_ADDRESS, tef,        EBM_WRITE);
  ebm_op(COMMON_ECA_ADDRESS, res,        EBM_WRITE);
  ebm_op(COMMON_ECA_ADDRESS, deadlineHi, EBM_WRITE);
  ebm_op(COMMON_ECA_ADDRESS, deadlineLo, EBM_WRITE);
  atomic_off();
          
  // send timing message
  ebm_flush();
          
  // diag and status
  // ... /* chk */
  
  return COMMON_STATUS_OK;
} //ebmWriteTM


uint32_t wrCheckSyncState() //check status of White Rabbit (link up, tracking)
{
  uint32_t syncState;

  syncState =  *(pPPSGen + (WR_PPS_GEN_ESCR >> 2));                         // read status
  syncState = syncState & WR_PPS_GEN_ESCR_MASK;                             // apply mask

  if ((syncState == WR_PPS_GEN_ESCR_MASK)) return COMMON_STATUS_OK;        // check if all relevant bits are set
  else                                     return COMMON_STATUS_WRBADSYNC;
} //wrCheckStatus


void common_init(uint32_t *startShared, uint32_t fwVersion) // determine address and clear shared mem
{
  /*uint32_t idx;
    const uint32_t c_Max_Rams = 10;
  sdb_location found_sdb[c_Max_Rams];
  sdb_location found_clu;*/

  uint32_t *pSharedTemp;
  uint32_t *pShared;
  int      i; 
  
  // set pointer to shared memory
  pShared                 = startShared;

  // get address to data
  pSharedVersion          = (uint32_t *)(pShared + (COMMON_SHARED_VERSION >> 2));
  pSharedSumStatus        = (uint32_t *)(pShared + (COMMON_SHARED_SUMSTATUS >> 2));
  pSharedCmd              = (uint32_t *)(pShared + (COMMON_SHARED_CMD >> 2));
  pSharedState            = (uint32_t *)(pShared + (COMMON_SHARED_STATE >> 2));
  pSharedData4EB          = (uint32_t *)(pShared + (COMMON_SHARED_DATA_4EB >> 2));
  pSharedNBadStatus       = (uint32_t *)(pShared + (COMMON_SHARED_NBADSTATUS >> 2));
  pSharedNBadState        = (uint32_t *)(pShared + (COMMON_SHARED_NBADSTATE >> 2));
  pSharedMacHi            = (uint32_t *)(pShared + (COMMON_SHARED_MACHI >> 2));
  pSharedMacLo            = (uint32_t *)(pShared + (COMMON_SHARED_MACLO >> 2));
  pSharedIp               = (uint32_t *)(pShared + (COMMON_SHARED_IP >> 2));
  pSharedTDiagHi          = (uint32_t *)(pShared + (COMMON_SHARED_TDIAGHI >> 2));
  pSharedTDiagLo          = (uint32_t *)(pShared + (COMMON_SHARED_TDIAGLO >> 2));
  pSharedTS0Hi            = (uint32_t *)(pShared + (COMMON_SHARED_TS0HI >> 2));
  pSharedTS0Lo            = (uint32_t *)(pShared + (COMMON_SHARED_TS0LO >> 2));

  // clear shared mem
  i = 0;
  pSharedTemp        = (uint32_t *)(pShared + (COMMON_SHARED_BEGIN >> 2 ));
  while (pSharedTemp < (uint32_t *)(pShared + (COMMON_SHARED_END >> 2 ))) {
    *pSharedTemp = 0x0;
    pSharedTemp++;
    i++;
  } // while pSharedTemp
  DBPRINT2("b2b-common: common part of shared mem is %d words (uint32_t), begin %x, end %x\n", i, pShared, pSharedTemp-1);
  
  // set initial values;
  *pSharedVersion      = fwVersion; // of all the shared variabes, only VERSION is a constant. Set it now!
  *pSharedNBadStatus   = 0;
  *pSharedNBadState    = 0;
} // initCommon


uint32_t findPPSGen() //find WB address of WR PPS Gen
{
  pPPSGen = 0x0;
  
  // get Wishbone address for PPS Gen
  pPPSGen = find_device_adr(CERN, WR_PPS_GEN);

  if (!pPPSGen) {DBPRINT1("b2b-test: can't find WR PPS Gen\n"); return COMMON_STATUS_ERROR;}
  else                                                          return COMMON_STATUS_OK;
} // findPPSGen


uint32_t findWREp() //find WB address of WR Endpoint
{
  pWREp = 0x0;
  
  pWREp = find_device_adr(WR_ENDPOINT_VENDOR, WR_ENDPOINT_PRODUCT);

  if (!pWREp) {DBPRINT1("b2b-test: can't find WR Endpoint\n"); return COMMON_STATUS_ERROR;}
  else                                                         return COMMON_STATUS_OK;
} // findWREp


uint32_t findIOCtrl() // find WB address of IO Control
{
  pIOCtrl = 0x0;

  pIOCtrl = find_device_adr(IO_CTRL_VENDOR, IO_CTRL_PRODUCT);

  if (!pIOCtrl) {DBPRINT1("b2b-test: can't find IO Control\n"); return COMMON_STATUS_ERROR;}
  else                                                          return COMMON_STATUS_OK;    
} // find IOCtrol


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

  if (!pECAQ) {DBPRINT1("b2b-test: can't find ECA queue\n"); return COMMON_STATUS_ERROR;}
  else                                                       return COMMON_STATUS_OK;
} // findECAQueue


uint32_t common_wait4ECAEvent(uint32_t msTimeout, uint64_t *deadline, uint64_t *param, uint32_t *isLate)  // 1. query ECA for actions, 2. trigger activity
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

  timeoutT    = getSysTime() + (uint64_t)msTimeout * (uint64_t)1000000 + (uint64_t)1000; 
  
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
      *param       = ((uint64_t)evtParamHigh << 32) + (uint64_t)evtParamLow;
      
      // pop action from channel
      *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;

      // here: do s.th. according to tag
      nextAction = actTag;
    
      return nextAction;
    } // if data is valid
  } // while not timed out

  return  COMMON_ECADO_TIMEOUT;
} // wait for ECA event


void common_initCmds() // init stuff for handling commands, trivial for now, will be extended
{
  //  initalize command value: 0x0 means 'no command'
  *pSharedCmd     = 0x0;
} // common_initCmds

 
void common_clearDiag()// clears all statistics
{
  uint64_t now;

  clearDiag(); 

  nBadStatus = 0;
  nBadState  = 0;
  now = getSysTime();
  *pSharedTDiagHi = (uint32_t)(now >> 32);
  *pSharedTDiagLo = (uint32_t)now & 0xffffffff;

} // common_clearDiag


uint32_t common_doActionS0()
{
  uint32_t status = COMMON_STATUS_OK;
  uint64_t now;

  if (findECAQueue() != COMMON_STATUS_OK) status = COMMON_STATUS_ERROR; 
  if (findPPSGen()   != COMMON_STATUS_OK) status = COMMON_STATUS_ERROR;
  if (findWREp()     != COMMON_STATUS_OK) status = COMMON_STATUS_ERROR;
  if (findIOCtrl()   != COMMON_STATUS_OK) status = COMMON_STATUS_ERROR;

  now           = getSysTime();
  *pSharedTS0Hi = (uint32_t)(now >> 32);
  *pSharedTS0Lo = (uint32_t)now & 0xffffffff;
  
  common_initCmds();                    

  return status;
} // common_doActionS0


void common_publishNICData()
{
  uint64_t mac;
  uint32_t ip;
  
  mac = wrGetMac(pWREp);
  *pSharedMacHi = (uint32_t)(mac >> 32) & 0xffff;
  *pSharedMacLo = (uint32_t)mac         & 0xffffffff;

  ip  = *(pEbCfg + (EBC_SRC_IP>>2));
  *pSharedIp    = ip;
} //common_publishNICData


void common_publishState(uint32_t state)
{
  *pSharedState = state; 
} // common_publishState


/* void common_publishStatus(uint32_t status)
{
  *pSharedStatus = status; 
} // common_publishState
*/

void common_publishSumStatus(uint32_t sumStatus)
{
  *pSharedSumStatus = sumStatus; 
} // common_publishSumStatus


void common_incBadStatusCnt()
{
  nBadStatus++;
  
  *pSharedNBadStatus = nBadStatus;
} // common_publishNBadStatus


void common_incBadStateCnt()
{
  nBadState++;
  
  *pSharedNBadState = nBadState;
} // common_publishNBadState


uint32_t exitActionError()
{
  return COMMON_STATUS_OK;
} // exitActionError

void common_cmdHandler(uint32_t *reqState) // handle commands from the outside world
{
  uint32_t cmd;
  
  cmd = *pSharedCmd;
  // check, if the command is valid and request state change
  if (cmd) {
    switch (cmd) {
    case COMMON_CMD_CONFIGURE :
      *reqState =  COMMON_STATE_CONFIGURED;
      DBPRINT3("b2b-test: received cmd %d\n", cmd);
      break;
    case COMMON_CMD_STARTOP :
      *reqState = COMMON_STATE_OPREADY;
      DBPRINT3("b2b-test: received cmd %d\n", cmd);
      break;
    case COMMON_CMD_STOPOP :
      *reqState = COMMON_STATE_STOPPING;
      DBPRINT3("b2b-test: received cmd %d\n", cmd);
      break;
    case COMMON_CMD_IDLE :
      *reqState = COMMON_STATE_IDLE;
      DBPRINT3("b2b-test: received cmd %d\n", cmd);
      break;
    case COMMON_CMD_RECOVER :
      *reqState = COMMON_STATE_IDLE;
      DBPRINT3("b2b-test: received cmd %d\n", cmd);
      break;
    case COMMON_CMD_CLEARDIAG :
      DBPRINT3("b2b-test: received cmd %d\n", cmd);
      common_clearDiag();
      break;
    default:
      DBPRINT3("b2b-test: received unknown command '0x%08x'\n", cmd);
    } // switch 
    *pSharedCmd = 0x0; // reset cmd value in shared memory 
  } // if command 
} // common_cmdHandler


uint32_t common_changeState(uint32_t *actState, uint32_t *reqState, uint32_t actStatus)   //state machine; see b2b-test.h for possible states and transitions
{
  uint32_t statusTransition= COMMON_STATUS_OK;
  uint32_t status;
  uint32_t nextState;                   

  // if something severe happened, perform implicitely allowed transition to ERROR or FATAL states
  // else                        , handle explicitcely allowed transitions

  if ((*reqState == COMMON_STATE_ERROR) || (*reqState == COMMON_STATE_FATAL)) {statusTransition = actStatus; nextState = *reqState;}
  else {
    nextState = *actState;                       // per default: remain in actual state without exit or entry action
    switch (*actState) {                         // check for allowed transitions: 1. determine next state, 2. perform exit or entry actions if required
    case COMMON_STATE_S0:
      if      (*reqState == COMMON_STATE_IDLE)       {                                             nextState = *reqState;}      
      break;
    case COMMON_STATE_IDLE:
      if      (*reqState == COMMON_STATE_CONFIGURED)  {statusTransition = entryActionConfigured(); nextState = *reqState;}
      break;
    case COMMON_STATE_CONFIGURED:
      if      (*reqState == COMMON_STATE_IDLE)       {                                             nextState = *reqState;}
      else if (*reqState == COMMON_STATE_CONFIGURED) {statusTransition = entryActionConfigured();  nextState = *reqState;}
      else if (*reqState == COMMON_STATE_OPREADY)    {statusTransition = entryActionOperation();   nextState = *reqState;}
      break;
    case COMMON_STATE_OPREADY:
      if      (*reqState == COMMON_STATE_STOPPING)   {statusTransition = exitActionOperation();    nextState = *reqState;}
      break;
    case COMMON_STATE_STOPPING:
      nextState = COMMON_STATE_CONFIGURED;      //automatic transition but without entryActionConfigured
    case COMMON_STATE_ERROR:
      if      (*reqState == COMMON_STATE_IDLE)       {statusTransition = exitActionError();        nextState = *reqState;}
      break;
    default: 
      nextState = COMMON_STATE_S0;
    } // switch actState
  }  // else something severe happened
  
  // if the transition failed, transit to error state (except we are already in FATAL state)
  if ((statusTransition != COMMON_STATUS_OK) && (nextState != COMMON_STATE_FATAL)) nextState = COMMON_STATE_ERROR;

  // if the state changes
  if (*actState != nextState) {                   
    mprintf("b2b-test: changed to state %d\n", nextState);
    *actState = nextState;                      
    status = statusTransition;
  } // if state change
  else  status = actStatus;

  *reqState = COMMON_STATE_UNKNOWN;              // reset requested state (= no change state requested)  

  return status;
} //changeState


uint32_t common_doAutoRecovery(uint32_t actState, uint32_t *reqState)                    // do autorecovery from error state
{
  switch (actState)
    {
    case COMMON_STATE_ERROR :
      DBPRINT3("b2b-common: attempting autorecovery ERROR -> IDLE\n");
      usleep(10000000);
      *reqState = COMMON_STATE_IDLE; 
      break;
    case COMMON_STATE_IDLE :
      DBPRINT3("b2b-common: attempting autorecovery IDLE -> CONFIGURED\n");
      usleep(5000000);
      *reqState = COMMON_STATE_CONFIGURED;
      break;
    case COMMON_STATE_CONFIGURED :
      DBPRINT3("b2b-common: attempting autorecovery CONFIGURED -> OPREADY\n");
      usleep(5000000);
      *reqState = COMMON_STATE_OPREADY;
      break;
    default : ;
    } // switch actState
} // doAutoRecovery


