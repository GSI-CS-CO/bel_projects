/********************************************************************************************
 *  wr-unipz.c
 *
 *  created : 2018
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 05-Dec-2024
 *
 *  lm32 program for gateway between UNILAC Pulszentrale and a White Rabbit network
 *  this basically serves a Data Master for UNILAC
 *
 *  source code UNIPZ:
 *  - https://www-acc.gsi.de/viewvc/view/devacc/eqmodels/pz/common/src/pzus-dpr-def.h (relevant header file)
 *  - https://www-acc.gsi.de/viewvc/view/devacc/eqmodels/pz/pzu/src/pzu-eqms.c (source code for PZ 1..7)
 *  - https://www-acc.gsi.de/viewvc/view/devacc/eqmodels/pz/common/src/pzus-dpr-def.h (relevant header file)
 *  - https://www-acc.gsi.de/viewvc/view/devacc/eqmodels/pz/common/src/pzus-dpr-def.h (relevant header file)
 *  - https://www-acc.gsi.de/viewvc/view/devacc/eqmodels/pz/pzu/src/pzu-eqms.c (source code for PZ 1..7)
 *  - https://www-acc.gsi.de/viewvc/view/devacc/eqmodels/pz/pzu/src/pzu-eqms.c (source code for PZ 1..7)
 *  - https://www-acc.gsi.de/viewvc/view/devacc/eqmodels/pz/common/src/pzus-dpr-def.h (relevant header file)
 *  - https://www-acc.gsi.de/viewvc/view/devacc/eqmodels/pz/pzu/src/pzu-eqms.c (source code for PZ 1..7)
 *  - https://www-acc.gsi.de/viewvc/view/devacc/eqmodels/pz/pzus/src/pzus-helper.hh (masks and definitions)
 * 
 *   events for the next cycle (received from the SuperPZ) have the following format:
 *   see code UNIPZ (defined in pzus-dpr-def-h, lines 72 ff; example: 0x10(use 'Kanal 1'), 0x00(use 'Kanal 0')
 *       15: value 0: announce event
 *           value 1: service event
 *   12..14: in case of announce event:
 *       14: short chopper pulse                      (bits 12..15: value 0x3)
 *       13: no    chopper pulse                      (bits 12..15: value 0x2)
 *           ** this bit has to be copied to bit 14 of a MIL telegram! **
 *       12: channel                                  (bits 12..15: value 0x1)                 
             ('Kanal Nummer'; UNIPZ has max two channels - 1 bit is sufficient)
 *   12..14: in case of service event: (info: service events are sent AFTER timing events have been sent)
 *           value 111: set all magnets to zero value (bits 12..15: value 0xf)
 *           value 110: send prep event for vacc      (bits 12..15: value 0xe)
 *           value 101: send prep event NOW           (bits 12..15: value 0xd) special case: don't wait until timing events have been sent
 *           value 100: unlock A4                     (bits 12..15: value 0xc) special case: don't wait until timing events have been sent
 *    8..11: virt acc
 *    0...7: value 1..7 equals # of PZ (** counting starts at 1(!) **)
 * 
 *   a '50 Hz sync' event (received from SuperPZ) has the following format:
 *    8..15: don't care
 *    0...7: 0d33
 *
 *   a 'sync data' event (recevied from SuperPZ) has the following format:
 *    8..15: don't care
 *    0...7: 0d32
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
 * Last update: 22-November-2018
 ********************************************************************************************/
#define WRUNIPZ_FW_VERSION 0x000220                                     // make this consistent with makefile

// standard includes
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

// includes specific for bel_projects 
#include "dbg.h"                                                        // debug outputs
#include <stack.h>                                                      // stack check ...
#include "ebm.h"                                                        // EB master
#include "pp-printf.h"                                                  // print statement
#include "mini_sdb.h"                                                   // sdb stuff
#include "aux.h"                                                        // cpu, IRQ, timer...
#include "uart.h"                                                       // WR console
#include "../../../top/gsi_scu/scu_mil.h"                               // register layout of 'MIL macro'

// includes for this project
#include <common-defs.h>                                                // common defs for firmware
#include <common-fwlib.h>                                               // common routines for firmware
#include <wr-unipz.h>                                                   // defs
#include <wrunipz_shared_mmap.h>                                        // autogenerated upon building firmware

// stuff required for environment
extern uint32_t* _startshared[];
unsigned int     cpuId, cpuQty;
#define  SHARED  __attribute__((section(".shared")))
uint64_t SHARED  dummy = 0;

// global variables
// shared mem
volatile uint32_t *pShared;             // pointer to begin of shared memory region
uint32_t *pSharedNCycle;                // pointer to a "user defined" u32 register; here: number of UNILAC cycles
uint32_t *pSharedTCycleAvg;             // pointer to a "user defined" u32 register; here: period of UNILAC cycle [us] (average over one second)
uint32_t *pSharedNMessageHi;            // pointer to a "user defined" u32 register; here: high bits # of messages
uint32_t *pSharedNMessageLo;            // pointer to a "user defined" u32 register; here: lo bits # of messages
uint32_t *pSharedMsgFreqAvg;            // pointer to a "user defined" u32 register; here: message rate (average over one second)
uint32_t *pSharedDtMax;                 // pointer to a "user defined" u32 register; here: max diff between deadline and time of dispatching
uint32_t *pSharedDtMin;                 // pointer to a "user defined" u32 register; here: min diff between deadline and time of dispatching
uint32_t *pSharedCycJmpMax;             // pointer to a "user defined" u32 register; here: max diff between expected and actual start of UNILAC cycle
uint32_t *pSharedCycJmpMin;             // pointer to a "user defined" u32 register; here: min diff between expected and actual start of UNILAC cycle
uint32_t *pSharedVaccAvg;               // pointer to a "user defined" u32 register; here: virt accs played during past second
uint32_t *pSharedPzAvg;                 // pointer to a "user defined" u32 register; here: PZs used during the past second
uint32_t *pSharedEvtData;               // pointer to a "user defined" u32 register; here: config data
uint32_t *pSharedEvtFlag;               // pointer to a "user defined" u32 register; here: config flags
uint32_t *cpuRamExternal;               // external address (seen from host bridge) of this CPU's RAM            

// lots of stuff to remember
uint32_t statusArray;                   // all status infos are ORed bit-wise into sum status, sum status is then published
int32_t  dtMax;                         // dT max (deadline - dispatch time)
int32_t  dtMin;                         // dT min (deadline - dispatch time)
int32_t  cycJmpMax;                     // dT max (difference between actual and expected start of UNILAC cycle)
int32_t  cycJmpMin;                     // dT min (difference between actual and expected start of UNILAC cycle)
uint32_t nLate;                         // # of late messages
uint32_t vaccAvg;                       // virt accs played over the past second
uint32_t pzAvg;                         // PZs used over the past second
uint32_t nCycleAct;                     // number of cycles
uint32_t nCyclePrev;                    // previous number of cycles
uint64_t nMsgAct;                       // # of messages sent
uint64_t nMsgPrev;                      // previous number of messages
uint64_t syncPrevT4;                    // timestamp of previous 50Hz sync event from SPZ
uint64_t syncPrevT3;                    // timestamp of previous sync
uint64_t syncPrevT2;                    // timestamp of previous sync
uint64_t syncPrevT1;                    // timestamp of previous sync 
uint64_t syncPrevT0;                    // timestamp of previous sync

// flags
uint32_t flagClearAllPZ;                // event tables of all PZs shall be cleared

// big data contains the event tables for all PZs, and for all virtual accelerators
// there are two sets of 16 virtual accelerators ('Kanal0' and 'Kanal1')
// [va0 of chn0]..[va15 of chn0][va0 of chn1]..[va15 of chn1]
dataTable bigData[WRUNIPZ_NPZ][WRUNIPZ_NVACC * WRUNIPZ_NCHN];

// info on what will be played at which PZ during actual cycles
uint32_t actVacc[WRUNIPZ_NPZ];          // vacc
uint32_t actChan[WRUNIPZ_NPZ];          // channel

// contains info on what will be played at which PZ during next cycle
uint32_t nextVacc[WRUNIPZ_NPZ];         // vacc
uint32_t nextChan[WRUNIPZ_NPZ];         // channel
uint32_t nextChopNo[WRUNIPZ_NPZ];       // flag: 'no chopper'
uint32_t nextChopShort[WRUNIPZ_NPZ];    // flag: 'short chopper'

uint32_t gid[]     = {448, 449, 450, 451, 452, 453, 454};  // GID for UNILAC
uint32_t milEvts[] = {WRUNIPZ_EVT_PZ1,  // MIL evt codes we are listening for
                      WRUNIPZ_EVT_PZ2,
                      WRUNIPZ_EVT_PZ3,
                      WRUNIPZ_EVT_PZ4,
                      WRUNIPZ_EVT_PZ5,
                      WRUNIPZ_EVT_PZ6,
                      WRUNIPZ_EVT_PZ7,
                      WRUNIPZ_EVT_NO_BEAM,
                      WRUNIPZ_EVT_SYNCH_DATA,
                      WRUNIPZ_EVT_50HZ_SYNCH};

uint32_t nEvtsLate;                     // # of late messages
uint32_t offsDone;                      // offset deadline WR message to time when we are done [ns]
int32_t  comLatency;                    // latency for messages received via ECA

int32_t  maxComLatency;
uint32_t maxOffsDone;

// write a timing message to the WR network
uint64_t writeTM(uint32_t uniEvt, uint64_t tStart, uint32_t pz, uint32_t virtAcc, uint32_t flagNochop, uint32_t flagShortchop)
{
  uint64_t deadline;
  uint32_t offset;                                  
  uint64_t id;
  uint64_t param;
  int32_t  tDiff;                                    // diff between deadline and dispatch timed
  uint32_t t;
  uint32_t evtCode;
  uint32_t evtData;
  uint32_t flags;

  // convert UNILAC event data
  // see wr-unipz.h -> 'typedef struct dataTable' on definition of bits in data
  t         = (uint32_t)((uniEvt >> 16) & 0xffff);      // get time relative to begining of UNILAC cycle [us]
  evtCode   = (uint32_t)( uniEvt        & 0x00ff);      // get event number
  evtData   = (uint32_t)((uniEvt >> 12) & 0x000f);      // get event data from set-values
  if (evtCode != EVT_COMMAND) { 
    evtData = evtData & 0xb;                            // clear third bit; third bit will carry 'no beam' information
    evtData = evtData | ((flagNochop & 0x1) << 2);      // fill  third bit with 'no beam' information
  } // if evtCode
  flags     = (uint32_t)( flagNochop          & 0x1) |  // 'no chopper bit'     /* chk: deprecated */
              (uint32_t)((flagShortchop << 1) & 0x2);   // 'short chopper bit'  /* chk: deprecated */

  // fill timing message
  id        = ((uint64_t)0x1       << 60)     |         // FID = 1
              ((uint64_t)(gid[pz]) << 48)     |         // GID
              ((uint64_t)evtCode   << 36)     |         // EVTNO
              ((uint64_t)0x0       << 32)     |         // flags
              ((uint64_t)virtAcc   << 20)     |         // SID
              ((uint64_t)0x0       <<  6)     |         // BPID
              ((uint64_t)0x0       <<  5)     |         // reserved
              ((uint64_t)0x0       <<  4)     |         // (reqNoBeam, not here)
              ((uint64_t)evtData        );              // last four bits; see https://www-acc.gsi.de/wiki/bin/viewauth/ProjectMgmt/MappingWrMilSisEsrUnilac#A_5_Decision
                                                        // bit 0: reserved; bit 1: high b/rho, rigid beam; bit2: 'no beam'; bit3: high current
  param     = ((uint64_t)flags     << 32)     |         // parameter field high bits, does carry flags /* chk: deprecated */
              ((uint64_t)0x0            );              // parameter field low bits, compatibility to 'old' wr-unipz (to be removed)
  
  // calc deadline
  offset    = (uint64_t)t * 1000;                       // convert offset us -> ns
  deadline  = tStart + (uint64_t)offset + (uint64_t)WRUNIPZ_MILCALIBOFFSET; 

  // send message
  fwlib_ebmWriteTM(deadline, id, param, 0x0, 1);
  
  // diag and status
  tDiff = deadline - getSysTime();
  if (tDiff < 0    ) nLate++;
  if (tDiff < dtMin) dtMin = tDiff;
  if (tDiff > dtMax) dtMax = tDiff;
  
  vaccAvg = vaccAvg | (1 << virtAcc);
  pzAvg   = pzAvg   | (1 << pz);
  nMsgAct++;

  return deadline;
} // writeTm


// plays virtual accelerators for all 'Pulszentralen'
uint32_t pzRunVacc(dataTable evts, uint64_t tStart, uint32_t pz, uint32_t virtAcc, uint32_t isPrep)
{
  int      i;
  uint64_t offset;

  // pack Ethernet frame with messages
  for (i=0; i<WRUNIPZ_NEVT; i++) {                     // loop over all data fields
    if ((evts.validFlags >> i) & 0x1) {                // data is valid?
      if ((evts.evtFlags >> i) & 0x1) {                // data is an event?
        if (((evts.prepFlags >> i) & 0x1) == isPrep) { // data matches 'isPrep condition'
          offset = writeTM(evts.data[i], tStart, pz, virtAcc, nextChopNo[pz], nextChopShort[pz]);
        } // if 'isPrep'
      } // is event
    } // is valid
  } // for i

  return COMMON_STATUS_OK;
} // pzRunVacc


// get length of vacc
uint32_t getVaccLen(dataTable evts)
{
  int      i;
  uint32_t usOffset;
  uint32_t tmp;
  

  usOffset = 0;
  
  for (i=0; i<WRUNIPZ_NEVT; i++) {
    if ((evts.validFlags >> i) & 0x1) {
      tmp = (uint32_t)((evts.data[i] >> 16) & 0xffff);
      if (tmp > usOffset) usOffset = tmp;
    } // if validFlags
  } // for i

  return usOffset;
} //getVaccLen


// predict start of next UNILAC cycle
uint64_t predictNxtCycle()
{
  uint64_t predictLen;                                 // predicted length of current cycle
  uint64_t predictT;                                   // predicted start of next cycle;
  uint64_t predictT1;                                  // prediction based on T1
  uint64_t predictT2;                                  // prediction based on T2
  uint64_t predictT3;                                  // prediction based on T3
  uint64_t predictT4;                                  // prediction based on T4
  
  // predicted length of UNILAC cycle
  predictLen = ((syncPrevT4 - syncPrevT0) >> 2);       // expected length of current UNILAC cycle (diff from 4 previous cycles divided by 4)

  // predictions based on previous cycles
  predictT4  = syncPrevT4 + 1 * predictLen;
  predictT3  = syncPrevT3 + 2 * predictLen;
  predictT2  = syncPrevT2 + 3 * predictLen;
  predictT1  = syncPrevT1 + 4 * predictLen;

  // average prediction
  predictT   = (predictT4 >> 2) + (predictT3 >> 2) + (predictT2 >> 2) + (predictT1 >> 2); 

  // check for limits
  if ((predictT - syncPrevT4) > (uint64_t)WRUNIPZ_UNILACPERIODMAX) predictT = syncPrevT4 + (uint64_t)WRUNIPZ_UNILACPERIODMAX; // upper bound according to manual
  if ((predictT - syncPrevT4) < (uint64_t)WRUNIPZ_UNILACPERIODMIN) predictT = syncPrevT4 + (uint64_t)WRUNIPZ_UNILACPERIODMIN; // lower bound
  
  return predictT;
} //predictNxtCyle


// typical init for lm32
void init() 
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
  sdb_location found_sdb[c_Max_Rams];
  sdb_location found_clu;
  
  // get pointer to shared memory
  pShared           = (uint32_t *)_startshared;

  // get address to data
  pSharedNCycle           = (uint32_t *)(pShared + (WRUNIPZ_SHARED_NCYCLE >> 2));  
  pSharedTCycleAvg        = (uint32_t *)(pShared + (WRUNIPZ_SHARED_TCYCLEAVG >> 2));
  pSharedNMessageHi       = (uint32_t *)(pShared + (WRUNIPZ_SHARED_NMESSAGEHI >> 2));
  pSharedNMessageLo       = (uint32_t *)(pShared + (WRUNIPZ_SHARED_NMESSAGELO >> 2));
  pSharedMsgFreqAvg       = (uint32_t *)(pShared + (WRUNIPZ_SHARED_MSGFREQAVG >> 2));
  pSharedDtMax            = (uint32_t *)(pShared + (WRUNIPZ_SHARED_DTMAX >> 2));
  pSharedDtMin            = (uint32_t *)(pShared + (WRUNIPZ_SHARED_DTMIN >> 2));
  pSharedCycJmpMax        = (uint32_t *)(pShared + (WRUNIPZ_SHARED_CYCJMPMAX >> 2));
  pSharedCycJmpMin        = (uint32_t *)(pShared + (WRUNIPZ_SHARED_CYCJMPMIN >> 2));
  pSharedVaccAvg          = (uint32_t *)(pShared + (WRUNIPZ_SHARED_VACCAVG >> 2));
  pSharedPzAvg            = (uint32_t *)(pShared + (WRUNIPZ_SHARED_PZAVG >> 2));
  pSharedEvtData          = (uint32_t *)(pShared + (WRUNIPZ_SHARED_EVT_DATA >> 2));
  pSharedEvtFlag          = (uint32_t *)(pShared + (WRUNIPZ_SHARED_EVT_FLAGS >> 2));
  
  // find address of CPU from external perspective
  cpuRamExternal = 0x0;
  idx = 0;
  find_device_multi(&found_clu, &idx, 1, GSI, LM32_CB_CLUSTER);
  if (idx == 0) {
    *reqState = COMMON_STATE_FATAL;
    DBPRINT1("wr-unipz: fatal error - did not find LM32-CB-CLUSTER!\n");
  } // if idx     	
  idx = 0;
  find_device_multi_in_subtree(&found_clu, &found_sdb[0], &idx, c_Max_Rams, GSI, LM32_RAM_USER);
  if (idx == 0) {
    *reqState = COMMON_STATE_FATAL;
    DBPRINT1("wr-unipz: fatal error - did not find THIS CPU!\n");
  } // if idx    
  else cpuRamExternal = (uint32_t *)(getSdbAdr(&found_sdb[cpuId]) & 0x7FFFFFFF); // CPU sees the 'world' under 0x8..., remove that bit to get host bridge perspective

  DBPRINT2("wr-unipz: CPU RAM external 0x%8x, shared offset 0x%08x\n", cpuRamExternal, SHARED_OFFS);
  DBPRINT2("wr-unipz: fw common shared begin   0x%08x\n", pShared);
  DBPRINT2("wr-unipz: fw common shared end     0x%08x\n", pShared + (COMMON_SHARED_END >> 2));

  // clear shared mem
  i = 0;
  pSharedTemp        = (uint32_t *)(pShared + (COMMON_SHARED_END >> 2 ) + 1);
  DBPRINT2("wr-unipz: fw specific shared begin 0x%08x\n", pSharedTemp);
  while (pSharedTemp < (uint32_t *)(pShared + (WRUNIPZ_SHARED_END >> 2 ))) {
    *pSharedTemp = 0x0;
    pSharedTemp++;
    i++;
  } // while pSharedTemp
  DBPRINT2("dm-unipz: fw specific shared end   0x%08x\n", pSharedTemp);

  *sharedSize        = (uint32_t)(pSharedTemp - pShared) << 2;

  // basic info to wr console                                                                                                                                                                                                                
  DBPRINT1("\n");
  DBPRINT1("wr-unipz: initSharedMem, shared size [bytes]: %d\n", *sharedSize);
  DBPRINT1("\n");

  // set initial values;
} // initSharedMem 


// configure SoC to receive events via MIL bus
uint32_t configMILEvent()
{
  uint32_t i;
  volatile uint32_t *pMilPiggy;

  pMilPiggy = fwlib_getMilPiggy();

  // initialize status and command register with initial values; disable event filtering; clear filter RAM
  if (writeCtrlStatRegEvtMil(pMilPiggy, 0,  MIL_CTRL_STAT_ENDECODER_FPGA | MIL_CTRL_STAT_INTR_DEB_ON) != MIL_STAT_OK) return COMMON_STATUS_ERROR; //chk sure we go for status error?

  // clean up 
  if (disableLemoEvtMil(pMilPiggy, 0, 1) != MIL_STAT_OK) return COMMON_STATUS_ERROR;
  if (disableLemoEvtMil(pMilPiggy, 0, 2) != MIL_STAT_OK) return COMMON_STATUS_ERROR;
  if (disableFilterEvtMil(pMilPiggy, 0)  != MIL_STAT_OK) return COMMON_STATUS_ERROR; 
  if (clearFilterEvtMil(pMilPiggy, 0)    != MIL_STAT_OK) return COMMON_STATUS_ERROR; 

  for (i=0; i < (0xf+1); i++) {
    // set filter for all possible virtual accelerators; set filter and LEMO for 50 Hz sync
    if (setFilterEvtMil(pMilPiggy, 0, WRUNIPZ_EVT_50HZ_SYNCH, i, MIL_FILTER_EV_TO_FIFO | MIL_FILTER_EV_PULS1_S) != MIL_STAT_OK) return COMMON_STATUS_ERROR;
    if (setFilterEvtMil(pMilPiggy, 0, WRUNIPZ_EVT_SYNCH_DATA, i, MIL_FILTER_EV_TO_FIFO                        ) != MIL_STAT_OK) return COMMON_STATUS_ERROR;
    if (setFilterEvtMil(pMilPiggy, 0, WRUNIPZ_EVT_PZ1       , i, MIL_FILTER_EV_TO_FIFO                        ) != MIL_STAT_OK) return COMMON_STATUS_ERROR;
    if (setFilterEvtMil(pMilPiggy, 0, WRUNIPZ_EVT_PZ2       , i, MIL_FILTER_EV_TO_FIFO                        ) != MIL_STAT_OK) return COMMON_STATUS_ERROR;
    if (setFilterEvtMil(pMilPiggy, 0, WRUNIPZ_EVT_PZ3       , i, MIL_FILTER_EV_TO_FIFO                        ) != MIL_STAT_OK) return COMMON_STATUS_ERROR;
    if (setFilterEvtMil(pMilPiggy, 0, WRUNIPZ_EVT_PZ4       , i, MIL_FILTER_EV_TO_FIFO                        ) != MIL_STAT_OK) return COMMON_STATUS_ERROR;
    if (setFilterEvtMil(pMilPiggy, 0, WRUNIPZ_EVT_PZ5       , i, MIL_FILTER_EV_TO_FIFO                        ) != MIL_STAT_OK) return COMMON_STATUS_ERROR;
    if (setFilterEvtMil(pMilPiggy, 0, WRUNIPZ_EVT_PZ6       , i, MIL_FILTER_EV_TO_FIFO                        ) != MIL_STAT_OK) return COMMON_STATUS_ERROR;
    if (setFilterEvtMil(pMilPiggy, 0, WRUNIPZ_EVT_PZ7       , i, MIL_FILTER_EV_TO_FIFO                        ) != MIL_STAT_OK) return COMMON_STATUS_ERROR;
    if (setFilterEvtMil(pMilPiggy, 0, WRUNIPZ_EVT_NO_BEAM   , i, MIL_FILTER_EV_TO_FIFO                        ) != MIL_STAT_OK) return COMMON_STATUS_ERROR;
  }

  // configure LEMO1 for pulse generation
  if (configLemoPulseEvtMil(pMilPiggy, 0, 1) != MIL_STAT_OK) return COMMON_STATUS_ERROR;

  return COMMON_STATUS_OK;
} // configMILEvent


// clears all statistics
void extern_clearDiag() 
{
  dtMax          = 0x80000000;
  dtMin          = 0x7fffffff;
  cycJmpMax      = 0x80000000;
  cycJmpMin      = 0x7fffffff;
  nLate          = 0;
  nCycleAct      = 0;
  nCyclePrev     = 0;
  nMsgAct        = 0;
  nMsgPrev       = 0;
  statusArray    = 0;
  maxComLatency  = 0;
  maxOffsDone    = 0;
} // clearDiag


// submit transferred config data
uint32_t configTransactSubmit()
{
  int      i,j,k,l;
  int      vacc;                              // virt acc
  uint32_t *flags;                            // [0]: new data; [1]: valid; [2]: prep; [3]: is event
  uint32_t *data;                             // 32bit words containing offset, id, ...
  uint32_t flagOffset;                        // address offset for relevant flags
  uint32_t dataOffset;                        // address offset for relevant data

  DBPRINT2("wr-unipz: evt flags start at 0x%x\n", pSharedEvtFlag);
  DBPRINT2("wr-unipz: evt data start at 0x0%x\n", pSharedEvtData);
  
  for (i=0; i < WRUNIPZ_NVACC; i++) {         // for all vacc
    for (j=0; j < WRUNIPZ_NPZ; j++) {         // for all Pulszentralen
      for (k=0; k < WRUNIPZ_NCHN; k++) {      // for all chanels
        flagOffset  = i * WRUNIPZ_NFLAG * WRUNIPZ_NCHN * WRUNIPZ_NPZ;     // offset for vacc
        flagOffset += j * WRUNIPZ_NFLAG * WRUNIPZ_NCHN;                   // offset for pz
        flagOffset += k * WRUNIPZ_NFLAG;                                  // offset of chn
        flags       = pSharedEvtFlag + flagOffset;                        // points to flags(vacc, pz, chn)

        if (flags[0]) {                                                   // new data?
          // copy flags
          bigData[j][k * WRUNIPZ_NVACC + i].validFlags = flags[1];
          bigData[j][k * WRUNIPZ_NVACC + i].prepFlags  = flags[2];
          bigData[j][k * WRUNIPZ_NVACC + i].evtFlags   = flags[3];
          
          // copy data
          dataOffset  = i * WRUNIPZ_NEVT * WRUNIPZ_NCHN * WRUNIPZ_NPZ;    // offset for vacc
          dataOffset += j * WRUNIPZ_NEVT * WRUNIPZ_NCHN;                  // offset for pz
          dataOffset += k * WRUNIPZ_NEVT;                                 // offset for chn
          data        = pSharedEvtData + dataOffset;                      // points to data
          DBPRINT2("wr-unipz: data at 0x%x, vacc %u, pz %u, chn %u\n", data, i, j, k);
          for (l=0; l < WRUNIPZ_NEVT; l++) bigData[j][k * WRUNIPZ_NVACC + i].data[l] = data[l];

          // clear 'new data' flag
          flags[0] = 0x0;
        } // if flags[0]
      } // for k
    } // for j
  } // for i

  DBPRINT2("wr-unipz: submit completed\n");
  
  return COMMON_STATUS_OK;
} // configTransactSubmit


// clears data of all PZs
void clearAllPZ()
{
  int i,j,k;

  // clear bigData in 'user' RAM
  for (i=0; i < WRUNIPZ_NPZ; i++) {
    for (j=0; j < (WRUNIPZ_NVACC * WRUNIPZ_NCHN); j++) {
      bigData[i][j].validFlags = 0x0;
      bigData[i][j].prepFlags  = 0x0;
      bigData[i][j].evtFlags   = 0x0;
      for (k=0; k < WRUNIPZ_NEVT; k++) bigData[i][j].data[k] = 0x0;
    } // for j
  } // for i

  // clear event data and flags in DP RAM
  for (i=0; i<WRUNIPZ_NEVTDATA; i++) pSharedEvtData[i] = 0x0;
  for (i=0; i<WRUNIPZ_NEVTFLAG; i++) pSharedEvtFlag[i] = 0x0;
} // clear PZ


// entry action configured state
uint32_t extern_entryActionConfigured()
{
  uint32_t status = COMMON_STATUS_OK;

  // configure EB master (SRC and DST MAC/IP are set from host)
  if ((status = fwlib_ebmInit(2000, 0xffffffffffff, 0xffffffff, EBM_NOREPLY)) != COMMON_STATUS_OK) {
    DBPRINT1("wr-unipz: ERROR - init of EB master failed! %u\n", (unsigned int)status);
    return status;
  } 

  // get and publish NIC data
  fwlib_publishNICData();

  // reset MIL piggy and wait
  if ((status = resetDevMil(fwlib_getMilPiggy(), 0))  != MIL_STAT_OK) {
    DBPRINT1("wr-unipz: ERROR - can't reset MIL Piggy\n");
    return WRUNIPZ_STATUS_MIL;
  } 

  // configure MIL piggy for timing events for all 16 virtual accelerators
  if ((status = configMILEvent()) != COMMON_STATUS_OK) {
    DBPRINT1("wr-unipz: ERROR - failed to configure MIL piggy for receiving timing events! %u\n", (unsigned int)status);
    return status;
  } 

  DBPRINT1("wr-unipz: MIL piggy configured for receving events (eventbus)\n");

  configLemoOutputEvtMil(fwlib_getMilPiggy(), 0, 2);    // used to see a blinking LED (and optionally connect a scope) for debugging
  
  return status;
} // entryActionConfigured


// entry action state 'op ready'
uint32_t extern_entryActionOperation()
{
  int      i;
  uint64_t tDummy;
  uint64_t iDummy;
  uint64_t pDummy;
  uint32_t fDummy;
  uint32_t flagDummy1, flagDummy2, flagDummy3, flagDummy4;

  fwlib_clearDiag();
  clearAllPZ();                                              // clear all event tables

  flagClearAllPZ        = 0;
  /* flagTransactionInit   = 0;
     flagTransactionSubmit = 0;*/
  for (i=0; i < WRUNIPZ_NPZ; i++) nextVacc[i] = 0xffffffff;  // 0xffffffff: no virt acc
  for (i=0; i < WRUNIPZ_NPZ; i++) actVacc[i]  = 0xffffffff;  // 0xffffffff: no virt acc
  
  enableFilterEvtMil(fwlib_getMilPiggy(), 0);                // enable MIL event filter
  clearFifoEvtMil(fwlib_getMilPiggy(), 0);                   // clear MIL event FIFO

  // flush ECA queue for lm32
  i = 0;
  while (fwlib_wait4ECAEvent(1 * 1000, &tDummy, &iDummy, &pDummy, &fDummy, &flagDummy1, &flagDummy2, &flagDummy3, &flagDummy4) != WRUNIPZ_ECADO_TIMEOUT) {i++;}
  DBPRINT1("wr-unipz: ECA queue flushed - removed %d pending entries from ECA queue\n", i);

  return COMMON_STATUS_OK;
} // entryActionOperation

// exit action state 'op ready'
uint32_t extern_exitActionOperation(){
  if (disableFilterEvtMil(fwlib_getMilPiggy(), 0) != MIL_STAT_OK) return COMMON_STATUS_ERROR; 
  
  return COMMON_STATUS_OK;
} // exitActionOperation


// command handler, handles commands specific for this project
void cmdHandler(uint32_t *reqState, uint32_t cmd)
{
  // check, if the command is valid and request state change
  if (cmd) {                             // check, if cmd is valid
    switch (cmd) {                       // do action according to command
      case WRUNIPZ_CMD_CONFSUBMIT :
        DBPRINT3("wr-unipz: received cmd %d\n", cmd);
        if (configTransactSubmit() != COMMON_STATUS_OK) DBPRINT1("wr-unipz: submission of config data failed\n");
        // takes about 51us, possibly just set a flag here and call routine after WRUNIPZ_EVT_50HZ_SYNCH
        break;
      case WRUNIPZ_CMD_CONFCLEAR :
        DBPRINT3("wr-unipz: received cmd %d\n", cmd);
        flagClearAllPZ = 1;
        break;
      default:
        DBPRINT3("wr-unipz: received unknown command '0x%08x'\n", cmd);
        break;
    } // switch 
  } // if command 
} // cmdHandler


// do action state 'op ready' - this is the main code of this FW
uint32_t doActionOperation(uint32_t *nCycle,                  // total number of UNILAC cycle since FW start
                           uint32_t actStatus)                // actual status of firmware
{
  uint32_t status;                                            // status returned by routines
  uint32_t ecaAction;                                         // action triggered by event received from ECA
  uint64_t recDeadline;                                       // deadline of event received via ECA
  uint64_t recEvtId;                                          // event ID received via ECA
  uint64_t recParam;                                          // param received via ECA
  uint32_t recTEF;                                            // TEF field received via ECA
  uint32_t flagLate;                                          // flag indicating that we received a 'late' event from ECA
  uint32_t flagEarly;                                         // flag indicating that a 'early event' was received from data master
  uint32_t flagConflict;                                      // flag indicating that a 'conflict event' was received from data master
  uint32_t flagDelayed;                                       // flag indicating that a 'delayed event' was received from data master

  uint64_t deadline;                                          // deadline to be used for action
  uint64_t tMIL;                                              // time when MIL event was received
  uint64_t tDummy;                                            // dummy timestamp
  uint64_t iDummy;                                            // dummy evt id
  uint64_t pDummy;                                            // dummy parameter
  uint32_t fDummy;                                            // dummy TEF
  uint32_t evtData;                                           // MIL event: data
  uint32_t evtCode;                                           // MIL event: code
  uint32_t virtAcc;                                           // MIL event: virtAcc
  uint32_t milStatus;                                         // status for receiving of MIL events
  uint32_t nLateLocal;                                        // remember actual counter
  uint32_t isPrepFlag;                                        // flag 'isPrep': prep-events are sent immediately, non-prep-events are sent at 50 Hz trigger
  int      ipz;                                               // index of PZ, helper variable
  int      i;
  uint32_t servEvt;                                           // service event
  uint32_t servOffs;                                          // offset for service event
  uint32_t tmpOffs;                                           // helper variable
  int32_t  tJump;                                             // diff between expected and actual start of UNILAC cycle
  uint64_t TS_dbg;                                            // debug: send received MIL event to ECA for debugging, here: TS
  uint64_t evtId_dbg;                                         // debug: vacc as GID; evtCode as evtNo
  uint64_t param_dbg;                                         // debug: evtData

  status = actStatus;

  // wait for MIL event
  milStatus = fwlib_wait4MILEvent(COMMON_MILTIMEOUT * 1000, &evtData, &evtCode, &virtAcc, milEvts, sizeof(milEvts)/sizeof(int));
  if (milStatus == COMMON_STATUS_TIMEDOUT) return WRUNIPZ_STATUS_NOMILEVENTS; // error: no MIL event, maybe dead UNIPZ?
  if (milStatus != COMMON_STATUS_OK)       return WRUNIPZ_STATUS_MIL;         // some other MIL error

  tMIL      = getSysTime();                                     // required as backup in case timestamping via TLU fails
  ebm_clr();                                                    // clear EB master (to be on the safe side)

  // handle MIL event
  switch (evtCode) {
    case WRUNIPZ_EVT_50HZ_SYNCH :                               // next UNILAC cycle starts
      (*nCycle)++;
      DBPRINT3("wr-unipz: 50Hz, data %d, evtcode %d, virtAcc %d\n", evtData, evtCode, virtAcc);
      
      // get timestamp from TLU -> ECA
      ecaAction  = fwlib_wait4ECAEvent(COMMON_ECATIMEOUT * 1000, &recDeadline, &recEvtId, &recParam, &recTEF, &flagLate, &flagEarly, &flagConflict, &flagDelayed);
      deadline   = recDeadline;
      
      // check, if timestamping via TLU failed; if yes, continue with TS from MIL
      if (ecaAction == WRUNIPZ_ECADO_TIMEOUT) {      
        deadline = tMIL;                                        
        status   = WRUNIPZ_STATUS_NOTIMESTAMP;
      } // if ecaAction

      // check, if timestamps form TLU and MIL are out of order; if yes, continue with TS from MIL
      if (deadline > tMIL) {
      deadline = tMIL;
      status   = WRUNIPZ_STATUS_ORDERTIMESTAMP;
      } // if deadline
      
      // check, if timestamp from TLU is not reasonable; if yes, continue with TS from MIL
      if ((tMIL - deadline) > WRUNIPZ_MATCHWINDOW) {
        deadline = tMIL;
        status   = WRUNIPZ_STATUS_BADTIMESTAMP;
      } // if tMIL

      comLatency = (int32_t)(getSysTime() - deadline);
      
      // walk through all PZs and run requested virt acc (time critical stuff)
      nLateLocal  = nLate;                                      // for bookkepping of late messages
      isPrepFlag  = 0;                                          // 50 Hz synch: time critical operation - use actual deadline from TLU
      for (i=0; i < WRUNIPZ_NPZ; i++) {
        if (nextVacc[i] != 0xffffffff) {
          actVacc[i] = nextVacc[i];                             // remember vacc for actual cycle
          actChan[i] = nextChan[i];                             // remember channel for actual cycle
          pzRunVacc(bigData[i][nextChan[i] * WRUNIPZ_NVACC + nextVacc[i]], deadline, i, nextVacc[i], isPrepFlag); // run virtual accelerator
          DBPRINT3("wr-unipz: playing pz %d, vacc %d\n", i, nextVacc[i]);
        } // if nextVacc
      } // for i
      
      if ((nLate != nLateLocal) && (status == COMMON_STATUS_OK)) status = WRUNIPZ_STATUS_LATE;
      DBPRINT3("wr-unipz: vA played:  %x %x %x %x %x %x %x\n", nextVacc[0], nextVacc[1], nextVacc[2], nextVacc[3], nextVacc[4], nextVacc[5], nextVacc[6]);
      
      // at this point we have scheduled all timing messages of the running cycle and completed the real-time critical stuff
      // now we can do other things...
      
      tJump       = (int32_t)(predictNxtCycle() - deadline);    // compare prediction and deadline
      if (tJump > cycJmpMax) cycJmpMax = tJump;
      if (tJump < cycJmpMin) cycJmpMin = tJump;
      
      syncPrevT0  = syncPrevT1;                                 // remember time of a previous cycle
      syncPrevT1  = syncPrevT2;                                 // remember time of a previous cycle
      syncPrevT2  = syncPrevT3;                                 // remember time of a previous cycle
      syncPrevT3  = syncPrevT4;                                 // remember time of a previous cycle
      syncPrevT4  = deadline;                                   // remember time of the previous cycle
      
      if (flagClearAllPZ)        {clearAllPZ();           flagClearAllPZ = 0;       }
      /*if (flagTransactionInit)   {configTransactInit();   flagTransactionInit = 0;  }  /* chk: error handling */
      /* chk !!! bug: in the same cycle where data becomes commited the routine getVacclen will fail (fix after we know commit mechanism) !!! */ 
      
      // reset requested virt accs; flush ECA queue
      for (i=0; i < WRUNIPZ_NPZ; i++) nextVacc[i] = 0xffffffff; // 0xffffffff: no virt acc for PZ
      while (fwlib_wait4ECAEvent(0, &tDummy, &iDummy, &pDummy, &fDummy, &flagLate, &flagEarly, &flagConflict, &flagDelayed) !=  WRUNIPZ_ECADO_TIMEOUT) {asm("nop");}
      
      break;
      
    case WRUNIPZ_EVT_PZ1 ... WRUNIPZ_EVT_PZ7 :                  // A: super PZ announces what happens in next UNILAC cycle or B: 'Service Event'
      // extract information from event from super PZ
      ipz            = evtCode - 1;                             // PZ: sPZ counts from 1..7, we count from 0..6
      
      // there are two different types of announce events
      // A: The announce event contains information about the vacc to played in the next cycle
      //    In this case 1. handle chopper mode, 2. send all 'prep events'
      // B: The announce event contains information about a service event to be sent
      //    In this case we just take care of the service event and do nothing else
      
      if ((evtData & WRUNIPZ_EVTDATA_SERVICE) == 0) {           // A: super PZ has announced vacc for next cycle: bits 12 (channel number), bits 13..15 (chopper mode)
        DBPRINT3("wr-unipz: playing prep events, pz %d, vacc %d\n", ipz, virtAcc);
        // get chopper mode
        nextChan[ipz]      = ((evtData & WRUNIPZ_EVTDATA_CHANNEL) != 0); // bit as channel number (there are only two channels)
        nextChopNo[ipz]    = ((evtData & WRUNIPZ_EVTDATA_NOCHOP)  != 0);
        nextVacc[ipz]      = virtAcc;
        nextChopShort[ipz] = nextChan[ipz];                     // UNIPZ implements 'short chopper' via a different 'Kanal', not via bit 14 as described in the documentation
        // pp_printf("data %d\n", evtData);
        
        // PZ1..7: preperation; as the next cycle has been announced, we may send all 'prep events' already now
        // as deadline, we use the prediciton based on previous UNILAC cycles
        nLateLocal = nLate;
        isPrepFlag = 1;                                                  
        deadline   = predictNxtCycle();                         // predict start of next cycle
        pzRunVacc(bigData[ipz][nextChan[ipz] * WRUNIPZ_NVACC + nextVacc[ipz]], deadline, ipz, nextVacc[ipz], isPrepFlag);
        if ((nLate != nLateLocal) && (status == COMMON_STATUS_OK)) status = WRUNIPZ_STATUS_LATE;
      } // if !SERVICE
      else {                                                    // B: super PZ has sent info on a service event: bits 12..15 encode event type
        DBPRINT3("wr-unipz: service event for pz %d, vacc %d\n", ipz, virtAcc);
        servOffs  = getVaccLen(bigData[ipz][actChan[ipz] * WRUNIPZ_NVACC + actVacc[ipz]]) & 0xffff;    // when to play service event relative to start of cycle [us]
        servOffs += (uint32_t)WRUNIPZ_TDIFFMIL;                                                        // add minimum time difference between sending transmission of MIL telegrams [us]
        tmpOffs   = (getSysTime() - syncPrevT4 + (uint64_t)COMMON_LATELIMIT) / 1000;                   // last possibility to play service event without risking of a late message [us]
        if (servOffs < tmpOffs) {
          servOffs = tmpOffs; 
          /* pp_printf("wr-unipz: prevent late service message\n"); */
        } // if tmpOffs
        servEvt   = 0x0;
        if (evtData == WRUNIPZ_EVTDATA_PREPACC) {
          servEvt = EVT_AUX_PRP_NXT_ACC | ((virtAcc & 0xf) << 8) | ((servOffs & 0xffff)        << 16); // send after last event of current PZ cycle
          writeTM(servEvt, syncPrevT4, ipz, virtAcc, 0, 0);                                            // send message
        } // if PREPACC
        if (evtData == WRUNIPZ_EVTDATA_ZEROACC) {
          servEvt = EVT_MAGN_DOWN       | ((virtAcc & 0xf) << 8) | ((servOffs & 0xffff)        << 16); // send after last event of current PZ cycle
          writeTM(servEvt, syncPrevT4, ipz, virtAcc, 0, 0);                                            // send message
        } // if ZEROACC
        if (evtData == WRUNIPZ_EVTDATA_PREPACCNOW) {
          servEvt = EVT_AUX_PRP_NXT_ACC | ((virtAcc & 0xf) << 8) | ((uint16_t)WRUNIPZ_QQOFFSET << 16); // send 'now' /* chk QQOFFSET */
          writeTM(servEvt, getSysTime(), ipz, virtAcc, 0, 0);                                          // send message
        } // if PREPACCNOW
        if (evtData == WRUNIPZ_EVTDATA_UNLOCKA4) {
          servEvt = EVT_UNLOCK_ALVAREZ  | ((virtAcc & 0xf) << 8) | ((uint16_t)WRUNIPZ_A4OFFSET << 16); // send 'now' /* chk A4OFFSET */
          writeTM(servEvt, getSysTime(), ipz, virtAcc, 0, 0);                                          // send message
        } // if UNLOCKA4
      } // else SERVICE
      
      break;
      
    case WRUNIPZ_EVT_NO_BEAM :
      // priv. comm. P. Kainberger: this is an additional 'service event' implemented via a different mechanism
      // when this is received from the Superpulszentrale, it is played for all PZs, that play the vacc specified

      for (i=0; i<WRUNIPZ_NPZ; i++) {
        // check, if we need to play the service event
        if (actVacc[i] == virtAcc) {
          servEvt = EVT_NO_BEAM | ((virtAcc & 0xf) << 8) | ((uint16_t)WRUNIPZ_NOBEAMOFFSET << 16); // send 'now' /* chk  WRUNIPZ_NOBEAMOFFSET */
          writeTM(servEvt, getSysTime(), i, virtAcc, 0, 0);
        } // if actVacc
      } // for all PZs
      
      break;

    case WRUNIPZ_EVT_SYNCH_DATA :                               // super PZ commits recently supplied virt acc -> replace active data by the new ones
      DBPRINT3("wr-unipz: synch data event\n");
      configTransactSubmit();
      
      break;
      
    default :
      // pp_printf("default case, evtCode %d\n", evtCode);
      break;
  } // switch evtCode

  offsDone = (int32_t)(getSysTime() - tMIL);

  // write MIL Event received via internal bus to our own ECA input
  // this allows debugging using saft-ctl snoop ...
  TS_dbg    = tMIL;
  evtId_dbg = 0xcafe000000000000;
  evtId_dbg = evtId_dbg | ((uint64_t)evtCode << 36);
  evtId_dbg = evtId_dbg | ((uint64_t)virtAcc << 20);
  param_dbg = (uint64_t)evtData;
  fwlib_ecaWriteTM(TS_dbg, evtId_dbg, param_dbg, 0x0, 1);

  // check WR sync state
  if (fwlib_wrCheckSyncState() == COMMON_STATUS_WRBADSYNC) return COMMON_STATUS_WRBADSYNC;
  else                                                     return status;
  
  return status;
} // doActionOperation


int main(void) {
  uint32_t status;                                            // (error) status
  uint32_t cmd;                                               // command via shared memory
  uint32_t actState;                                          // actual FSM state
  uint32_t pubState;                                          // value of published state
  uint32_t reqState;                                          // requested FSM state
  uint32_t sharedSize;                                        // size of shared memory

  uint32_t *buildID;

  // init local variables
  reqState       = COMMON_STATE_S0;
  actState       = COMMON_STATE_UNKNOWN;
  pubState       = COMMON_STATE_UNKNOWN;
  status         = COMMON_STATUS_OK;
  buildID        = (uint32_t *)(INT_BASE_ADR + BUILDID_OFFS);

  // init
  pp_printf("wr-unipz: huhu\n");
  init();
  initSharedMem(&reqState, &sharedSize);                               // initialize shared memory THIS MUST BE CALLED FIRST
  fwlib_init((uint32_t *)_startshared, cpuRamExternal, SHARED_OFFS, sharedSize, "wr-unipz", WRUNIPZ_FW_VERSION); // init common stuff
  fwlib_clearDiag();                                                   // clear common diagnostic data

  while (1) {
    check_stack_fwid(buildID);                                         // check for stack corruption
    fwlib_cmdHandler(&reqState, &cmd);                                 // check for common commands and possibly request state changes
    cmdHandler(&reqState, cmd);                                        // check for project relevant commands
    status = COMMON_STATUS_OK;                                         // reset status for each iteration
    status = fwlib_changeState(&actState, &reqState, status);          // handle requested state changes
    switch(actState) {                                                 // state specific do actions 
      case COMMON_STATE_OPREADY :
        status = doActionOperation(&nCycleAct, status);
        if (status == COMMON_STATUS_WRBADSYNC) reqState = COMMON_STATE_ERROR;
        if (status == COMMON_STATUS_ERROR)     reqState = COMMON_STATE_ERROR;
        break;
      default :                                                        // avoid flooding WB bus with unnecessary activity
        status = fwlib_doActionState(&reqState, actState, status);    // handle do actions states
        break;
    } // switch
    
    // update shared memory for data that are averaged over a longer period
    if (nCycleAct != nCyclePrev) {                                     // update only once per cycle 
      if ((nCycleAct % WRUNIPZ_UNILACFREQ) == 0) {                     // about only once per second
        
        // length of UNILAC cycle [ns]
        *pSharedTCycleAvg  = (uint32_t)((syncPrevT4 - syncPrevT0) >> 2);
       
        // message rate [Hz]
        *pSharedMsgFreqAvg = (uint32_t)(nMsgAct - nMsgPrev);
        nMsgPrev           = nMsgAct;

        // virt acc and pz info
        *pSharedVaccAvg    = vaccAvg;
        *pSharedPzAvg      = pzAvg;
        vaccAvg            = 0;
        pzAvg              = 0;
      } // if nCycleAct %
      
      nCyclePrev = nCycleAct;
    } // if nCycleAct

    // update sum status
    switch (status) {
      case COMMON_STATUS_OK :                                                     // status OK
        statusArray = statusArray |  (0x1 << COMMON_STATUS_OK);                   // set OK bit
        break;
      default :                                                                   // status not OK
        if ((statusArray >> COMMON_STATUS_OK) & 0x1) fwlib_incBadStatusCnt();     // changing status from OK to 'not OK': increase 'bad status count'
        statusArray = statusArray & ~(0x1 << COMMON_STATUS_OK);                   // clear OK bit
        statusArray = statusArray |  (0x1 << status);                             // set status bit and remember other bits set
        break;
    } // switch status
    
    // update shared memory
    if ((pubState == COMMON_STATE_OPREADY) && (actState  != COMMON_STATE_OPREADY)) fwlib_incBadStateCnt();
    fwlib_publishStatusArray(statusArray);
    pubState             = actState;
    fwlib_publishState(pubState);

    if (comLatency > maxComLatency) maxComLatency = comLatency;
    if (offsDone   > maxOffsDone)   maxOffsDone   = offsDone;
    fwlib_publishTransferStatus(0, 0, 0, nLate, maxOffsDone, maxComLatency);

    *pSharedDtMax        = dtMax;
    *pSharedDtMin        = dtMin;
    *pSharedCycJmpMax    = cycJmpMax;
    *pSharedCycJmpMin    = cycJmpMin;
    *pSharedNCycle       = nCycleAct; 
    *pSharedNMessageHi   = (uint32_t)(nMsgAct >> 32);
    *pSharedNMessageLo   = (uint32_t)(nMsgAct & 0xffffffff);
  } // while

  return(1);
} // main
