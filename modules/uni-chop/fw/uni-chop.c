/********************************************************************************************
 *  uni-chop.c
 *
 *  created : 2024
 *  author  : Dietrich Beck, Tobias Habermann GSI-Darmstadt
 *  version : 12-Sep-2024
 *
 *  firmware required for UNILAC chopper control
 *  
 *  This firmware just takes care of writing the time critical MIL telegrams to the UNILAC
 *  'Choppersteuerung'. The data to be written is received via timing messages
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
 * Last update: 15-April-2019
 ********************************************************************************************/
#define UNICHOP_FW_VERSION      0x000001  // make this consistent with makefile

// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

// includes specific for bel_projects
#include "dbg.h"                                                        // debug outputs
#include <stack.h>                                                      // stack check
#include "pp-printf.h"                                                  // print
#include "mini_sdb.h"                                                   // sdb stuff
#include "aux.h"                                                        // cpu and IRQ
#include "uart.h"                                                       // WR console
#include "../../../top/gsi_scu/scu_mil.h"                               // register layout of 'MIL macro'

// includes for this project 
#include <common-defs.h>                                                // common defs for firmware
#include <common-fwlib.h>                                               // common routines for firmware
#include <uni-chop.h>                                                     // specific defs for uni-chop
#include <unichop_shared_mmap.h>                                          // autogenerated upon building firmware

// stuff required for environment
extern uint32_t* _startshared[];
unsigned int     cpuId, cpuQty;
#define  SHARED  __attribute__((section(".shared")))
uint64_t SHARED  dummy = 0;

// global variables 
volatile uint32_t *pShared;                // pointer to begin of shared memory region
volatile uint32_t *pSharedSetMilDev;       // pointer to a "user defined" u32 register; here: MIL device for sending MIL messages; 0: MIL Piggy; 1..: SIO in slot 1..
volatile uint32_t *pSharedGetNMilSndHi;    // pointer to a "user defined" u32 register; here: number of MIL writes, high word
volatile uint32_t *pSharedGetNMilSndLo;    // pointer to a "user defined" u32 register; here: number of MIL writes, low word
volatile uint32_t *pSharedGetNMilSndErr;   // pointer to a "user defined" u32 register; here: number of failed MIL writes
volatile uint32_t *pSharedGetNEvtsRecHi;   // pointer to a "user defined" u32 register; here: number of timing messages received, high word
volatile uint32_t *pSharedGetNEvtsRecLo;   // pointer to a "user defined" u32 register; here: number of timing messages received, low word
volatile uint32_t *pSharedGetNEvtsLate;    // pointer to a "user defined" u32 register; here: number of late timing messages received

uint32_t *cpuRamExternal;               // external address (seen from host bridge) of this CPU's RAM
volatile uint32_t *pMilSend;            // address of MIL device sending timing messages, usually this will be a SIO

uint64_t statusArray;                   // all status infos are ORed bit-wise into statusArray, statusArray is then published
uint64_t nMilSnd;                       // # of MIL writes
uint32_t nMilSndErr;                    // # of failed MIL writes                      
uint64_t nEvtsRec;                      // # of received timing messages
uint32_t nEvtsLate;                     // # of late timing messages
uint32_t offsDone;                      // offset deadline WR message to time when we are done [ns]
int32_t  comLatency;                    // latency for messages received via ECA

int32_t  maxComLatency;
uint32_t maxOffsDone;

// constants (as variables to have a defined type)
uint64_t  one_us_ns = 1000;

// debug 
uint64_t t1, t2;
int32_t  tmp1;

void init() // typical init for lm32
{
  discoverPeriphery();        // mini-sdb ...
  uart_init_hw();             // needed by WR console   
  cpuId = getCpuIdx();
} // init


// determine address and clear shared mem
void initSharedMem(uint32_t *reqState, uint32_t *sharedSize)
{
  uint32_t idx;
  uint32_t *pSharedTemp;
  int      i; 
  const uint32_t c_Max_Rams = 10;
  sdb_location   found_sdb[c_Max_Rams];
  sdb_location   found_clu;
  
  // get pointer to shared memory
  pShared                    = (uint32_t *)_startshared;

  // get address to data
  pSharedSetMilDev           = (uint32_t *)(pShared + (UNICHOP_SHARED_SET_MIL_DEV           >> 2));
  pSharedGetNMilSndHi        = (uint32_t *)(pShared + (UNICHOP_SHARED_GET_N_MIL_SND_HI      >> 2));
  pSharedGetNMilSndLo        = (uint32_t *)(pShared + (UNICHOP_SHARED_GET_N_MIL_SND_LO      >> 2));
  pSharedGetNMilSndErr       = (uint32_t *)(pShared + (UNICHOP_SHARED_GET_N_MIL_SND_ERR     >> 2));
  pSharedGetNEvtsRecHi       = (uint32_t *)(pShared + (UNICHOP_SHARED_GET_N_EVTS_REC_HI     >> 2));
  pSharedGetNEvtsRecLo       = (uint32_t *)(pShared + (UNICHOP_SHARED_GET_N_EVTS_REC_LO     >> 2));

  // find address of CPU from external perspective
  idx = 0;
  find_device_multi(&found_clu, &idx, 1, GSI, LM32_CB_CLUSTER);
  if (idx == 0) {
    *reqState = COMMON_STATE_FATAL;
    DBPRINT1("uni-chop: fatal error - did not find LM32-CB-CLUSTER!\n");
  } // if idx
  idx = 0;
  find_device_multi_in_subtree(&found_clu, &found_sdb[0], &idx, c_Max_Rams, GSI, LM32_RAM_USER);
  if (idx == 0) {
    *reqState = COMMON_STATE_FATAL;
    DBPRINT1("uni-chop: fatal error - did not find THIS CPU!\n");
  } // if idx
  else cpuRamExternal = (uint32_t *)(getSdbAdr(&found_sdb[cpuId]) & 0x7FFFFFFF); // CPU sees the 'world' under 0x8..., remove that bit to get host bridge perspective

  DBPRINT2("uni-chop: CPU RAM external 0x%8x, shared offset 0x%08x\n", cpuRamExternal, SHARED_OFFS);
  DBPRINT2("uni-chop: fw common shared begin   0x%08x\n", pShared);
  DBPRINT2("uni-chop: fw common shared end     0x%08x\n", pShared + (COMMON_SHARED_END >> 2));

  // clear shared mem
  i = 0;
  pSharedTemp        = (uint32_t *)(pShared + (COMMON_SHARED_END >> 2 ) + 1);
  DBPRINT2("uni-chop: fw specific shared begin 0x%08x\n", pSharedTemp);
  while (pSharedTemp < (uint32_t *)(pShared + (UNICHOP_SHARED_END >> 2 ))) {
    *pSharedTemp = 0x0;
    pSharedTemp++;
    i++;
  } // while pSharedTemp
  DBPRINT2("uni-chop: fw specific shared end   0x%08x\n", pSharedTemp);

  *sharedSize        = (uint32_t)(pSharedTemp - pShared) << 2;

  // basic info to wr console
  DBPRINT1("\n");
  DBPRINT1("uni-chop: initSharedMem, shared size [bytes]: %d\n", *sharedSize);
  DBPRINT1("\n");
} // initSharedMem


// read from modules connected via MIL
int16_t readFromModuleMil(uint16_t ifbAddr, uint16_t modAddr, uint16_t modReg, uint16_t *data) 
{
  uint16_t wData      = 0x0;    // data to write
  uint16_t rData      = 0x0;    // data to read
  int16_t  busStatus  = 0;      // status of bus operation

  // select module
  wData     = (modAddr << 8) | modReg;
  if ((busStatus = writeDevMil(pMilSend, ifbAddr, IFB_FC_ADDR_BUS_W, wData))  != MIL_STAT_OK) {
    DBPRINT1("uni-chop: readFromChopper failed (address), MIL error code %d\n", busStatus);
    return busStatus;
  } // if busStatus not ok

  // read data
  if ((busStatus = readDevMil(pMilSend, ifbAddr, IFB_FC_DATA_BUS_R, &rData)) == MIL_STAT_OK) *data = rData;
  if (busStatus != MIL_STAT_OK) DBPRINT1("uni-chop: readFromChopper failed (data), MIL error code %d\n", busStatus);
  
  return(busStatus);
} // readFromModuleMil 


// clear project specific diagnostics
void extern_clearDiag()
{
  statusArray   = 0x0;  
  nMilSnd       = 0x0;
  nMilSndErr    = 0x0;
  nEvtsRec      = 0x0;
  nEvtsLate     = 0x0;
  offsDone      = 0x0;
  comLatency    = 0x0;
  maxComLatency = 0x0;
  maxOffsDone   = 0x0;
} // extern_clearDiag 


// entry action 'configured' state
uint32_t extern_entryActionConfigured()
{
  uint32_t status = COMMON_STATUS_OK;

  // get and publish NIC data
  fwlib_publishNICData(); 

  // get address of MIL device sending MIL telegrams; 0 is MIL piggy
  if (*pSharedSetMilDev == 0){
    pMilSend = fwlib_getMilPiggy();
    if (!pMilSend) {
      DBPRINT1("uni-chop: ERROR - can't find MIL device\n");
      return COMMON_STATUS_OUTOFRANGE;
    } // if !pMilSend
  } // if SetMilDev
  else {
    // SCU slaves have offsets 0x20000, 0x40000... for slots 1, 2 ...
    pMilSend = fwlib_getSbMaster();
    if (!pMilSend) {
      DBPRINT1("uni-chop: ERROR - can't find MIL device\n");
      return COMMON_STATUS_OUTOFRANGE;
    } // if !pMilSend
    else pMilSend += *pSharedSetMilDev * 0x20000;
  } // else SetMilDev

  // reset MIL sender and wait
  if ((status = resetPiggyDevMil(pMilSend))  != MIL_STAT_OK) {
    DBPRINT1("uni-chop: ERROR - can't reset MIL device\n");
    return UNICHOP_STATUS_MIL;
  }  // if reset

  // if everything is ok, we must return with COMMON_STATUS_OK
  if (status == MIL_STAT_OK) status = COMMON_STATUS_OK;

  return status;
} // extern_entryActionConfigured


// entry action 'operation' state
uint32_t extern_entryActionOperation()
{
  int      i;
  uint64_t tDummy;
  uint64_t eDummy;
  uint64_t pDummy;
  uint32_t fDummy;
  uint32_t flagDummy1, flagDummy2, flagDummy3, flagDummy4;
  int      enable_fifo;

  // clear diagnostics
  fwlib_clearDiag();

  // flush ECA queue for lm32
  i = 0;
  while (fwlib_wait4ECAEvent(1000, &tDummy, &eDummy, &pDummy, &fDummy, &flagDummy1, &flagDummy2, &flagDummy3, &flagDummy4) !=  COMMON_ECADO_TIMEOUT) {i++;}
  DBPRINT1("uni-chop: ECA queue flushed - removed %d pending entries from ECA queue\n", i);
    
  // init get values
  *pShared                  = 0x0;
  *pSharedSetMilDev         = 0x0;
  *pSharedGetNMilSndHi      = 0x0;
  *pSharedGetNMilSndLo      = 0x0;
  *pSharedGetNMilSndErr     = 0x0;
  *pSharedGetNEvtsRecHi     = 0x0;
  *pSharedGetNEvtsRecLo     = 0x0;
  *pSharedGetNEvtsLate      = 0x0;

  nMilSnd                   = 0;
  nMilSndErr                = 0;
  nEvtsRec                  = 0;
  nEvtsLate                 = 0;
  offsDone                  = 0;
  comLatency                = 0;
  maxComLatency             = 0;
  maxOffsDone               = 0;

  uint16_t data;
  
  readFromModuleMil(IFB_ADDR_CU, MOD_LOGIC1_ADDR, MOD_LOGIC1_REG_STATUSGLOBAL, &data);
  pp_printf("module version 0x%x\n", data);
  

  return COMMON_STATUS_OK;
} // extern_entryActionOperation


uint32_t extern_exitActionOperation()
{ 
  return COMMON_STATUS_OK;
} // extern_exitActionOperation


// do action of state operation: This is THE central code of this firmware
uint32_t doActionOperation(uint64_t *tAct,                    // actual time
                           uint32_t actStatus)                // actual status of firmware
{
  uint32_t status;                                            // status returned by routines
  uint32_t flagIsLate;                                        // flag indicating that we received a 'late' event from ECA
  uint32_t flagIsEarly;                                       // flag 'early'
  uint32_t flagIsConflict;                                    // flag 'conflict'
  uint32_t flagIsDelayed;                                     // flag 'delayed'
  uint32_t ecaAction;                                         // action triggered by event received from ECA
  uint64_t recDeadline;                                       // deadline received from ECA
  uint64_t recEvtId;                                          // evt ID received
  uint64_t recParam;                                          // param received
  uint32_t recTEF;                                            // TEF received
  uint32_t recGid;                                            // GID received
  uint32_t recEvtNo;                                          // event number received
  uint32_t recSid;                                            // SID received
  uint32_t recBpid;                                           // BPID received
  uint64_t sendDeadline;                                      // deadline to send
  uint64_t sendEvtId;                                         // evtid to send
  uint64_t sendParam;                                         // parameter to send
  uint32_t sendTEF;                                           // TEF to send

  uint16_t strahlweg_mask;                                    // strahlweg mask
  uint16_t strahlweg_reg;                                     // strahlweg register

  int      i;
  uint64_t one_us_ns = 1000;
  uint64_t sysTime;
  uint32_t tmp32;
  
  status    = actStatus;

  ecaAction = fwlib_wait4ECAEvent(COMMON_ECATIMEOUT * 1000, &recDeadline, &recEvtId, &recParam, &recTEF, &flagIsLate, &flagIsEarly, &flagIsConflict, &flagIsDelayed);

  switch (ecaAction) {
    // received timing message containing Strahlweg data
    case UNICHOP_ECADO_STRAHLWEG_REC:
      comLatency   = (int32_t)(getSysTime() - recDeadline);
      recGid       = (uint32_t)((recEvtId >> 48) & 0x00000fff);
      recEvtNo     = (uint32_t)((recEvtId >> 36) & 0x00000fff);
      recSid       = (uint32_t)((recEvtId >> 20) & 0x00000fff);

      if (recGid != GID_LOCAL) return UNICHOP_STATUS_BADSETTING;
      if (recSid  > 15)        return COMMON_STATUS_OUTOFRANGE;

      // check if we are too late 
      if (getSysTime() > recDeadline) {
        nEvtsLate++;
        flagIsLate    = 1;
      } // if getSysTime
      else flagIsLate = 0;

      if (!flagIsLate) {
        strahlweg_reg  =  recParam        & 0xffff;
        strahlweg_mask = (recParam >> 16) & 0xffff;

        /*        // write strahlweg register
        writeToCU(UNICHOP_ADDR_MIL_64BIO, C_IO64_KANAL_1, strahlweg_reg);

        // write strahlweg mask
        writeToCU(UNICHOP_ADDR_MIL_64BIO, */

      } // if not late


      




      /*
      
      // build timing message and inject into ECA

      // deadline
      sendDeadline  = recDeadline + UNICHOP_PRETRIGGER_DM + mil_latency - UNICHOP_MILSEND_LATENCY;
       // protect from nonsense hi-frequency bursts
      if (sendDeadline < previous_time + UNICHOP_MILSEND_MININTERVAL) {
        sendDeadline = previous_time + UNICHOP_MILSEND_MININTERVAL;
        nEvtsBurst++;
      } // if sendDeadline
      previous_time = sendDeadline;

      // evtID + param
      sendEvtId     = recEvtId;
      prepMilTelegramEca(milTelegram, &sendEvtId, &sendParam);

      // clear MIL FIFO and write to ECA
      if (mil_mon) clearFifoEvtMil(pMilRec);
      fwlib_ecaWriteTM(sendDeadline, sendEvtId, sendParam, 0x0, 1);

      nEvtsSnd++;
      sysTime = getSysTime();
      // note: we pretrigger 500us the WR-messages to have time to deliver the message
      // however, some service events will come with an 'official' pretrigger of only 250us;
      // thus, the ECA will report the WR-message as late. For us however, we only need write the
      // MIL-message to the ECA 'in time'. Here, 'in time' means that we write the MIL-message
      // to the ECA sooner than its deadline. Usually this will be the case even if we receive
      // the WR-message late from the ECA. Thus the late info depends on if we manage to
      // write the MIL message in time.
      // note: we add 20us in the comparison taking into account the time it takes to write
      // the message to the ECA and the time it takes the ECA to process that message

      offsDone = sysTime - recDeadline;

      // handle UTC events; here the UTC time (- offset) is distributed as a series of MIL telegrams
      if (recEvtNo == utc_trigger) {
        // send EVT_UTC_1/2/3/4/5 telegrams
        make_mil_timestamp(sendDeadline, evt_utc, utc_offset);
        sendDeadline += trig_utc_delay * one_us_ns;
        for (i=0; i<UNICHOP_N_UTC_EVTS; i++){
          sendDeadline += utc_utc_delay * one_us_ns;
          prepMilTelegramEca(evt_utc[i], &sendEvtId, &sendParam);
          // nicer evtId for debugging only
          sendEvtId &= 0xffff000fffffffff;
          sendEvtId |= (uint64_t)(0x0e0 + i) << 36;
          fwlib_ecaWriteTM(sendDeadline, sendEvtId, sendParam, 0x0, 1);
          nEvtsSnd++;
        } // for i
      } // if utc_trigger

      // reset inhibit counter for fill events
      inhibit_fill_events = RESET_INHIBIT_COUNTER;

      break;

    // received timing message from TLU; this indicates a received MIL telegram on the MIL piggy
    case UNICHOP_ECADO_MIL_TLU:

      // in case of data monitoring, read message from MIL piggy FIFO and re-send it locally via the ECA
      if (mil_mon==2) {
        if (fwlib_wait4MILEvent(50, &recMilEvtData, &recMilEvtCode, &recMilVAcc, recMilEvts, 0) == COMMON_STATUS_OK) {

          // deadline
          sendDeadline  = recDeadline - UNICHOP_POSTTRIGGER_TLU; // time stamp when MIL telegram was decoded at MIL piggy
          sendDeadline += 1000000;                             // advance to 1ms into the future to avoid late messages
          // evtID
          sendEvtId     = fwlib_buildEvtidV1(LOC_MIL_REC, recMilEvtCode, 0x0, recMilVAcc, 0x0, recMilEvtData);
          // param
          sendParam     = ((uint64_t)mil_domain) << 32;

          fwlib_ecaWriteTM(sendDeadline, sendEvtId, sendParam, 0x0, 1);
          nEvtsRecD++;
        } // if wait4Milevent
      } // if mil_mon

      // read number of received 'broken' MIL telegrams
      readEventErrCntMil(pMilRec, &nEvtsErr);

      nEvtsRecT++;
      
      break;*/

    default :                                                         // flush ECA Queue
      flagIsLate = 0;                                                 // ignore late events
  } // switch ecaAction
 
  // check for late event
  if ((status == COMMON_STATUS_OK) && flagIsLate) {
    status = UNICHOP_STATUS_LATEMESSAGE;
    nEvtsLate++;
  } // if flagIslate
  
  // check WR sync state
  if (fwlib_wrCheckSyncState() == COMMON_STATUS_WRBADSYNC) return COMMON_STATUS_WRBADSYNC;
  else                                                     return status;
} // doActionOperation


int main(void) {
  uint64_t tActMessage;                         // time of actual message
  uint32_t status;                              // (error) status
  uint32_t actState;                            // actual FSM state
  uint32_t pubState;                            // value of published state
  uint32_t reqState;                            // requested FSM state
  uint32_t dummy1;                              // dummy parameter
  uint32_t sharedSize;                          // size of shared memory
  uint32_t *buildID;                            // build ID of lm32 firmware

  // init local variables
  buildID        = (uint32_t *)(INT_BASE_ADR + BUILDID_OFFS);                 // required for 'stack check'  

  reqState       = COMMON_STATE_S0;
  actState       = COMMON_STATE_UNKNOWN;
  pubState       = COMMON_STATE_UNKNOWN;
  status         = COMMON_STATUS_OK;

  nMilSnd        = 0;
  nMilSndErr     = 0;
  nEvtsRec       = 0; 
  nEvtsLate      = 0;

  init();                                                                     // initialize stuff for lm32
  initSharedMem(&reqState, &sharedSize);                                      // initialize shared memory
  fwlib_init((uint32_t *)_startshared, cpuRamExternal, SHARED_OFFS, sharedSize, "uni-chop", UNICHOP_FW_VERSION); // init common stuff
  fwlib_clearDiag();                                                          // clear common diagnostics data
  
  while (1) {
    check_stack_fwid(buildID);                                                // check stack status
    fwlib_cmdHandler(&reqState, &dummy1);                                     // check for commands and possibly request state changes
    status = COMMON_STATUS_OK;                                                // reset status for each iteration

    // state machine
    status = fwlib_changeState(&actState, &reqState, status);                 // handle requested state changes
    switch(actState) {                                                        // state specific do actions
      case COMMON_STATE_OPREADY :
        status = doActionOperation(&tActMessage, status);
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
    pubState = actState;
    fwlib_publishState(pubState);

    if (comLatency > maxComLatency) maxComLatency = comLatency;
    if (offsDone   > maxOffsDone)   maxOffsDone   = offsDone;
    fwlib_publishTransferStatus(0, 0, 0, nEvtsLate, maxOffsDone, maxComLatency);

    *pSharedGetNEvtsRecHi  = (uint32_t)(nEvtsRec >> 32);
    *pSharedGetNEvtsRecLo  = (uint32_t)(nEvtsRec & 0xffffffff);
    *pSharedGetNMilSndHi   = (uint32_t)(nMilSnd >> 32);
    *pSharedGetNMilSndLo   = (uint32_t)(nMilSnd & 0xffffffff);
    *pSharedGetNEvtsLate   = (uint32_t)(nEvtsLate);
    *pSharedGetNMilSndErr  = (uint32_t)(nMilSndErr);
  } // while

  return(1); // this should never happen ...
} // main
