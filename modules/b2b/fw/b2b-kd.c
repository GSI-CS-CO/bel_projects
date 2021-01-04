/********************************************************************************************
 *  b2b-kd.c
 *
 *  created : 2020
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 04-January-2021
 *
 *  firmware required for kicker and related diagnostics
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
 * Last update: 19-November-2020
 ********************************************************************************************/
#define B2BPM_FW_VERSION 0x000214                                       // make this consistent with makefile

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

/* includes specific for bel_projects */
#include "dbg.h"                                                        // debug outputs
#include <stack.h>                                                      // stack check
#include "ebm.h"                                                        // EB master
#include "pp-printf.h"                                                  // print
#include "mini_sdb.h"                                                   // sdb stuff
#include "aux.h"                                                        // cpu and IRQ
#include "uart.h"                                                       // WR console

/* includes for this project */
#include <common-defs.h>                                                // common defs for firmware
#include <common-fwlib.h>                                               // common routines for firmware
#include <b2b.h>                                                        // specific defs for b2b
#include <b2bkd_shared_mmap.h>                                          // autogenerated upon building firmware

// stuff required for environment
extern uint32_t* _startshared[];
unsigned int     cpuId, cpuQty;
#define  SHARED  __attribute__((section(".shared")))
uint64_t SHARED  dummy = 0;

// global variables 
volatile uint32_t *pShared;                // pointer to begin of shared memory region
volatile uint32_t *pSharedGettKickTrigHi;  // pointer to a "user defined" u32 register; here: time of kicker trigger signal, high bits
volatile uint32_t *pSharedGettKickTrigLo;  // pointer to a "user defined" u32 register; here: time of kicker trigger signal, low bits
volatile uint32_t *pSharedGettKickDMon;    // pointer to a "user defined" u32 register; here: delay of monitor signal 
volatile uint32_t *pSharedGettKickDProbe;  // pointer to a "user defined" u32 register; here: delay of probe signal
volatile uint32_t *pSharedGetKickSid;      // pointer to a "user defined" u32 register; here: SID of last kicker event
volatile uint32_t *pSharedGetKickGid;      // pointer to a "user defined" u32 register; here: GID of last kicker event
volatile int32_t  *pSharedGetComLatency;   // pointer to a "user defined" u32 register; here: latency for messages received via ECA

uint32_t *cpuRamExternal;                  // external address (seen from host bridge) of this CPU's RAM            

uint64_t statusArray;                      // all status infos are ORed bit-wise into statusArray, statusArray is then published
uint32_t nTransfer;                        // # of transfers
uint32_t transStat;                        // status of transfer, here: meanDelta of 'poor mans fit'
int32_t  comLatency;                       // latency for messages received via ECA

void init() // typical init for lm32
{
  discoverPeriphery();        // mini-sdb ...
  uart_init_hw();             // needed by WR console   
  cpuId = getCpuIdx();
} // init


void initSharedMem(uint32_t *reqState) // determine address and clear shared mem
{
  uint32_t idx;
  uint32_t *pSharedTemp;
  int      i; 
  const uint32_t c_Max_Rams = 10;
  sdb_location found_sdb[c_Max_Rams];
  sdb_location found_clu;
  
  // get pointer to shared memory
  pShared                 = (uint32_t *)_startshared;

  // get address to data
  pSharedGettKickTrigHi   = (uint32_t *)(pShared + (B2B_SHARED_GET_TKTRIGHI    >> 2));
  pSharedGettKickTrigLo   = (uint32_t *)(pShared + (B2B_SHARED_GET_TKTRIGLO    >> 2));
  pSharedGettKickDMon     = (uint32_t *)(pShared + (B2B_SHARED_GET_DKMON       >> 2));
  pSharedGettKickDProbe   = (uint32_t *)(pShared + (B2B_SHARED_GET_DKPROBE     >> 2));
  pSharedGetKickSid       = (uint32_t *)(pShared + (B2B_SHARED_GET_SID         >> 2));
  pSharedGetKickGid       = (uint32_t *)(pShared + (B2B_SHARED_GET_GID         >> 2));
  pSharedGetComLatency    =  (int32_t *)(pShared + (B2B_SHARED_GET_COMLATENCY  >> 2));
  
  // find address of CPU from external perspective
  idx = 0;
  find_device_multi(&found_clu, &idx, 1, GSI, LM32_CB_CLUSTER);
  if (idx == 0) {
    *reqState = COMMON_STATE_FATAL;
    DBPRINT1("b2b-kd: fatal error - did not find LM32-CB-CLUSTER!\n");
  } // if idx
  idx = 0;
  find_device_multi_in_subtree(&found_clu, &found_sdb[0], &idx, c_Max_Rams, GSI, LM32_RAM_USER);
  if (idx == 0) {
    *reqState = COMMON_STATE_FATAL;
    DBPRINT1("b2b-kd: fatal error - did not find THIS CPU!\n");
  } // if idx
  else cpuRamExternal           = (uint32_t *)(getSdbAdr(&found_sdb[cpuId]) & 0x7FFFFFFF); // CPU sees the 'world' under 0x8..., remove that bit to get host bridge perspective

  DBPRINT2("b2b-kd: CPU RAM External 0x%08x, begin shared 0x%08x\n", (unsigned int)cpuRamExternal, (unsigned int)SHARED_OFFS);

  // clear shared mem
  i = 0;
  pSharedTemp        = (uint32_t *)(pShared + (COMMON_SHARED_END >> 2 ) + 1);
  while (pSharedTemp < (uint32_t *)(pShared + (B2B_SHARED_END >> 2 ))) {
    *pSharedTemp = 0x0;
    pSharedTemp++;
    i++;
  } // while pSharedTemp
  DBPRINT2("b2b-kd: used size of shared mem is %d words (uint32_t), begin %x, end %x\n", i, (unsigned int)pShared, (unsigned int)pSharedTemp-1);
  fwlib_publishSharedSize((uint32_t)(pSharedTemp - pShared) << 2);  
} // initSharedMem 


// clear project specific diagnostics
void extern_clearDiag()
{
  statusArray  = 0x0; 
  nTransfer    = 0;
  transStat    = 0;
  comLatency   = 0;
} // extern_clearDiag
  

uint32_t extern_entryActionConfigured()
{
  uint32_t status = COMMON_STATUS_OK;

  // disable input gate 
  fwlib_ioCtrlSetGate(0, 1);
  fwlib_ioCtrlSetGate(0, 0);

  // configure EB master (SRC and DST MAC/IP are set from host)
  if ((status = fwlib_ebmInit(2000, 0xffffffffffff, 0xffffffff, EBM_NOREPLY)) != COMMON_STATUS_OK) {
    DBPRINT1("b2b-kd: ERROR - init of EB master failed! %u\n", (unsigned int)status);
    return status;
  } 

  // get and publish NIC data
  fwlib_publishNICData();

  return status;
} // extern_entryActionConfigured


uint32_t extern_entryActionOperation()
{
  int      i;
  uint64_t tDummy;
  uint64_t eDummy;
  uint64_t pDummy;
  uint32_t fDummy;
  uint32_t flagDummy;

  // clear diagnostics
  fwlib_clearDiag();             

  // flush ECA queue for lm32
  i = 0;
  while (fwlib_wait4ECAEvent(1000, &tDummy, &eDummy, &pDummy, &fDummy, &flagDummy) !=  COMMON_ECADO_TIMEOUT) {i++;}
  DBPRINT1("b2b-kd: ECA queue flushed - removed %d pending entries from ECA queue\n", i);

  *pSharedGettKickTrigHi  = 0x0;
  *pSharedGettKickTrigLo  = 0x0;
  *pSharedGettKickDMon    = 0x0;
  *pSharedGettKickDProbe  = 0x0;
  *pSharedGetKickSid      = 0x0;
  *pSharedGetKickGid      = 0x0;
  *pSharedGetComLatency   = 0x0;

  return COMMON_STATUS_OK;
} // extern_entryActionOperation


uint32_t extern_exitActionOperation()
{
  return COMMON_STATUS_OK;
} // extern_exitActionOperation


uint32_t doActionOperation(uint64_t *tAct,                    // actual time
                           uint32_t actStatus)                // actual status of firmware
{
  uint32_t status;                                            // status returned by routines
  uint32_t flagIsLate;                                        // flag indicating that we received a 'late' event from ECA
  uint32_t ecaAction;                                         // action triggered by event received from ECA
  uint64_t recDeadline;                                       // deadline received
  uint64_t reqDeadline;                                       // deadline requested by sender
  uint64_t recEvtId;                                          // evt ID received
  uint64_t recParam;                                          // param received
  uint32_t recTEF;                                            // TEF received
  uint32_t recGid;                                            // GID received
  uint32_t recSid;                                            // SID received
  uint32_t recRes;                                            // reserved bits received
  uint64_t sendDeadline;                                      // deadline to send
  uint64_t sendEvtId;                                         // evtid to send
  uint64_t sendParam;                                         // parameter to send
  uint32_t sendEvtNo;                                         // evtNo to send

  uint64_t tKickTrig;                                         // time of kicker trigger signal
  uint64_t tKickMon;                                          // time of kicker monitor signal
  uint64_t tKickProbe1;                                       // time of kicker probe signal rising edge
  uint64_t tKickProbe2;                                       // time of kicker probe signal falling edge
  uint32_t dKickMon;                                          // delay of kicker monitor signal with respect to kicker trigger signal 
  uint32_t dKickProbe;                                        // delay of kicker probe signal with respect to kicker trigger signal 
  uint64_t lKickProbe;                                        // length of kicker probe signal with respect to kicker trigger signal
  uint32_t flagRecMon;                                        // flag: received monitor signal
  uint32_t flagRecProbe;                                      // flag: received probe signal
  uint32_t nError;                                            // # of error bit
  uint32_t flagsError;                                        // error flags
  
  status    = actStatus;
  transStat = 0x0;
  sendEvtNo = 0x0;

  ecaAction = fwlib_wait4ECAEvent(COMMON_ECATIMEOUT*1000, &recDeadline, &recEvtId, &recParam, &recTEF, &flagIsLate);
   
  switch (ecaAction) {
    case B2B_ECADO_B2B_TRIGGEREXT :                           // this is an OR, no 'break' on purpose
      sendEvtNo = B2B_ECADO_B2B_DIAGKICKEXT;
      nError    = 0;
    case B2B_ECADO_B2B_TRIGGERINJ :
      if (!sendEvtNo) {
        sendEvtNo = B2B_ECADO_B2B_DIAGKICKINJ;
        nError = 2;
      } // if TRIGGERINJ

      // NB: we need to pretrigger on this event as we need time to enable the input gates
      reqDeadline = recDeadline + (uint64_t)B2B_PRETRIGGER;  // ECA is configured to pre-trigger ahead of time!!!
      comLatency  = (int32_t)(getSysTime() - recDeadline);
      nTransfer++;

      tKickTrig          = reqDeadline;
      tKickMon           = 0;
      tKickProbe1        = 0;
      tKickProbe2        = 0;
      dKickMon           = 0;
      dKickProbe         = 0;
      lKickProbe         = 0;
      flagRecMon         = 0;
      flagRecProbe       = 0;
      recGid             = (uint32_t)((recEvtId >> 48) & 0xfff);
      recSid             = (uint32_t)((recEvtId >> 20) & 0xfff);
      recRes             = (uint32_t)(recEvtId & 0x3f);       // lowest 6 bit of EvtId
      *pSharedGetKickGid = recGid;
      *pSharedGetKickSid = recSid;
      flagsError         = recRes;

      fwlib_ioCtrlSetGate(1, 1);                              // enable input gate monitor signal
      fwlib_ioCtrlSetGate(1, 0);                              // enable input gate probe signal

      // get monitor signal
      ecaAction = fwlib_wait4ECAEvent(B2B_ACCEPTDIAG, &recDeadline, &recEvtId, &recParam, &recTEF, &flagIsLate);
      if (ecaAction == B2B_ECADO_TLUINPUT2) {tKickMon = recDeadline; flagRecMon = 1;}

      // get 1st probe signal 'rising edge'
      ecaAction = fwlib_wait4ECAEvent(B2B_ACCEPTDIAG, &recDeadline, &recEvtId, &recParam, &recTEF, &flagIsLate);
      if (ecaAction == B2B_ECADO_TLUINPUT1) {tKickProbe1 = recDeadline - (uint64_t)B2B_PRETRIGGER; flagRecProbe = 1;}

      fwlib_ioCtrlSetGate(0, 1);                              // disable input gates 
      fwlib_ioCtrlSetGate(0, 0);

      /* chk it might be a good idea to check if the received deadlines make sense .... */
    

      if (flagRecMon)    dKickMon   = (uint32_t)(tKickMon    - tKickTrig);
      else              {dKickMon   = 0x7fffffff; flagsError =  recRes | (B2B_ERRFLAG_KDEXT << nError);}
      if (flagRecProbe)  dKickProbe = (uint32_t)(tKickProbe1 - tKickTrig);
      else               dKickProbe = 0x7fffffff;

      // send command: transmit measured phase value
      sendEvtId    = 0x1000000000000000;                          // FID
      sendEvtId    = sendEvtId | ((uint64_t)recGid      << 48);   // GID 
      sendEvtId    = sendEvtId | ((uint64_t)sendEvtNo   << 36);   // EVTNO
      sendEvtId    = sendEvtId | ((uint64_t)recSid      << 20);   // SID
      sendEvtId    = sendEvtId | flagsError;                      // Reserved
      sendParam    =             ((uint64_t)dKickMon  << 32);     // delay of monitor signal
      sendParam    = sendParam |  (uint64_t)dKickProbe;           // delay of probe signal
      sendDeadline = recDeadline + (uint64_t)(2 * COMMON_AHEADT); // data shall become true 1ms after trigger event

      if (getSysTime() > (sendDeadline - COMMON_AHEADT)) return COMMON_STATUS_TIMEDOUT;  // ophs, too late

      fwlib_ebmWriteTM(sendDeadline, sendEvtId, sendParam);

      *pSharedGettKickTrigHi = (uint32_t)((tKickTrig  >> 32) & 0xffffffff);
      *pSharedGettKickTrigLo = (uint32_t)( tKickTrig         & 0xffffffff);
      *pSharedGettKickDMon   = dKickMon;
      *pSharedGettKickDProbe = dKickProbe;

      transStat    = flagRecMon + flagRecProbe;
      
      break; //  B2B_ECADO_B2B_TRIGGERINJ
    default : ;
  } // switch ecaAction
 

  status = actStatus;
  
  return status;
} // doActionOperation


int main(void) {
  uint64_t tActCycle;                           // time of actual UNILAC cycle
  uint32_t status;                              // (error) status
  uint32_t actState;                            // actual FSM state
  uint32_t pubState;                            // value of published state
  uint32_t reqState;                            // requested FSM state
  uint32_t dummy1;                              // dummy parameter
  uint32_t *buildID;                            // build ID of lm32 firmware
 
  // init local variables
  buildID        = (uint32_t *)(INT_BASE_ADR + BUILDID_OFFS);                 // required for 'stack check'  

  reqState       = COMMON_STATE_S0;
  actState       = COMMON_STATE_UNKNOWN;
  pubState       = COMMON_STATE_UNKNOWN;
  status         = COMMON_STATUS_OK;
  nTransfer      = 0;

  init();                                                                     // initialize stuff for lm32
  fwlib_init((uint32_t *)_startshared, cpuRamExternal, SHARED_OFFS, "b2b-kd", B2BPM_FW_VERSION); // init common stuff
  initSharedMem(&reqState);                                                   // initialize shared memory
  fwlib_clearDiag();                                                          // clear common diagnostics data
  
  while (1) {
    check_stack_fwid(buildID);                                                // check stack status
    fwlib_cmdHandler(&reqState, &dummy1);                                     // check for commands and possibly request state changes
    status = COMMON_STATUS_OK;                                                // reset status for each iteration

    // state machine
    status = fwlib_changeState(&actState, &reqState, status);                 // handle requested state changes
    switch(actState) {                                                        // state specific do actions
      case COMMON_STATE_OPREADY :
        status = doActionOperation(&tActCycle, status);
        if (status == COMMON_STATUS_WRBADSYNC)      reqState = COMMON_STATE_ERROR;
        if (status == COMMON_STATUS_ERROR)          reqState = COMMON_STATE_ERROR;
        break;
      default :                                                               // avoid flooding WB bus with unnecessary activity
        status = fwlib_doActionState(&reqState, actState, status);            // other 'do actions' are handled here
        break;
    } // switch
    
    switch (status) {
      case COMMON_STATUS_OK :                                                 // status OK
        statusArray = statusArray |  (0x1 << COMMON_STATUS_OK);               // set OK bit
        break;
      default :                                                               // status not OK
        if ((statusArray >> COMMON_STATUS_OK) & 0x1) fwlib_incBadStatusCnt(); // changing status from OK to 'not OK': increase 'bad status count'
        statusArray = statusArray & ~((uint64_t)0x1 << COMMON_STATUS_OK);     // clear OK bit
        statusArray = statusArray |  ((uint64_t)0x1 << status);               // set status bit and remember other bits set
        break;
    } // switch status
    
    if ((pubState == COMMON_STATE_OPREADY) && (actState  != COMMON_STATE_OPREADY)) fwlib_incBadStateCnt();
    fwlib_publishStatusArray(statusArray);
    pubState              = actState;
    fwlib_publishState(pubState);
    fwlib_publishTransferStatus(nTransfer, 0x0, transStat);
    *pSharedGetComLatency = comLatency;
  } // while

  return(1); // this should never happen ...
} // main
