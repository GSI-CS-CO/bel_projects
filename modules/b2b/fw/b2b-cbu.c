/********************************************************************************************
 *  b2b-cbu.c
 *
 *  created : 2019
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 18-September-2020
 *
 *  firmware required to implement the CBU (Central Buncht-To-Bucket Unit)
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
 * Last update: 23-April-2019
 ********************************************************************************************/
#define B2BCBU_FW_VERSION 0x000100                                      // make this consistent with makefile

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

/* includes specific for bel_projects */
#include "dbg.h"
#include <stack.h>                                                      // stack check
#include "ebm.h"
#include "pp-printf.h"                                                  // print stuff
#include "mini_sdb.h"                                                   // sdb stuff
#include "aux.h"                                                        // cpu and IRQ
#include "uart.h"                                                       // WR console

/* includes for this project */
#include <common-defs.h>                                                // common defs
#include <common-fwlib.h>                                               // fw lib
#include <b2b.h>                                                        // defs for b2b
#include <b2bcbu_shared_mmap.h>                                         // autogenerated upon building firmware

// stuff required for environment
extern uint32_t* _startshared[];
unsigned int     cpuId, cpuQty;
#define  SHARED  __attribute__((section(".shared")))
uint64_t SHARED  dummy = 0;
volatile uint32_t *pShared;             // pointer to begin of shared memory region

// public variables

volatile uint32_t *pSharedMode;         // pointer to a "user defined" u32 register; here: mode of B2B transfer
volatile uint32_t *pSharedSID;          // pointer to a "user defined" u32 register; here: sequence ID of B2B transfer
volatile uint32_t *pSharedTH1ExtHi;     // pointer to a "user defined" u32 register; here: period of h=1 extraction, high bits
volatile uint32_t *pSharedTH1ExtLo;     // pointer to a "user defined" u32 register; here: period of h=1 extraction, low bits
volatile uint32_t *pSharedNHExt;        // pointer to a "user defined" u32 register; here: harmonic number extraction
volatile uint32_t *pSharedTH1InjHi;     // pointer to a "user defined" u32 register; here: period of h=1 injection, high bits
volatile uint32_t *pSharedTH1InjLo;     // pointer to a "user defined" u32 register; here: period of h=1 injecion, low bits
volatile uint32_t *pSharedNHInj;        // pointer to a "user defined" u32 register; here: harmonic number injection
volatile uint32_t *pSharedTBeatHi;      // pointer to a "user defined" u32 register; here: period of beating, high bits
volatile uint32_t *pSharedTBeatLo;      // pointer to a "user defined" u32 register; here: period of beating, low bits
volatile int32_t *pSharedCPhase;        // pointer to a "user defined" u32 register; here: correction for phase matching ('phase knob')
volatile int32_t *pSharedCTrigExt;      // pointer to a "user defined" u32 register; here: correction for trigger extraction ('extraction kicker knob')
volatile int32_t *pSharedCTrigInj;      // pointer to a "user defined" u32 register; here: correction for trigger injection ('injction kicker knob')

uint32_t *cpuRamExternal;               // external address (seen from host bridge) of this CPU's RAM            

uint64_t statusArray;                   // all status infos are ORed bit-wise into statusArray, statusArray is then published
uint32_t nTransfer;                     // # of transfers
uint32_t transStat;                     // status of ongoing transfer
uint64_t TH1Ext;                        // h=1 period [as] of extraction machine
uint64_t TH1Inj;                        // h=1 period [as] of injection machine
uint32_t nHExt;                         // harmonic number of extraction machine 0..15
uint32_t nHInj;                         // harmonic number of injection machine 0..15
uint64_t tH1Ext;                        // h=1 phase  [ns] of extraction machine
uint64_t tH1Inj;                        // h=1 phase  [ns] of injection machine


void init() // typical init for lm32
{
  discoverPeriphery();        // mini-sdb ...
  uart_init_hw();             // needed by WR console   
  cpuId = getCpuIdx();
} // init


void initSharedMem() // determine address and clear shared mem
{
  uint32_t idx;
  uint32_t *pSharedTemp;
  int      i; 
  const uint32_t c_Max_Rams = 10;
  sdb_location found_sdb[c_Max_Rams];
  sdb_location found_clu;
  
  // get pointer to shared memory
  pShared                 = (uint32_t *)_startshared;

  pSharedMode             = (uint32_t *)(pShared + (B2B_SHARED_MODE       >> 2));
  pSharedSID              = (uint32_t *)(pShared + (B2B_SHARED_SID        >> 2));
  pSharedTH1ExtHi         = (uint32_t *)(pShared + (B2B_SHARED_TH1EXTHI   >> 2));
  pSharedTH1ExtLo         = (uint32_t *)(pShared + (B2B_SHARED_TH1EXTLO   >> 2));
  pSharedNHExt            = (uint32_t *)(pShared + (B2B_SHARED_NHEXT      >> 2));
  pSharedTH1InjHi         = (uint32_t *)(pShared + (B2B_SHARED_TH1INJHI   >> 2));
  pSharedTH1InjLo         = (uint32_t *)(pShared + (B2B_SHARED_TH1INJLO   >> 2));
  pSharedNHInj            = (uint32_t *)(pShared + (B2B_SHARED_NHINJ      >> 2));
  pSharedTBeatHi          = (uint32_t *)(pShared + (B2B_SHARED_TBEATHI    >> 2));
  pSharedTBeatLo          = (uint32_t *)(pShared + (B2B_SHARED_TBEATLO    >> 2));
  pSharedCPhase           =  (int32_t *)(pShared + (B2B_SHARED_CPHASE     >> 2));
  pSharedCTrigExt         =  (int32_t *)(pShared + (B2B_SHARED_CTRIGEXT   >> 2));
  pSharedCTrigInj         =  (int32_t *)(pShared + (B2B_SHARED_CTRIGINT   >> 2));
  
  // find address of CPU from external perspective
  idx = 0;
  find_device_multi(&found_clu, &idx, 1, GSI, LM32_CB_CLUSTER);	
  idx = 0;
  find_device_multi_in_subtree(&found_clu, &found_sdb[0], &idx, c_Max_Rams, GSI, LM32_RAM_USER);
  if(idx >= cpuId) {
    cpuRamExternal           = (uint32_t *)(getSdbAdr(&found_sdb[cpuId]) & 0x7FFFFFFF); // CPU sees the 'world' under 0x8..., remove that bit to get host bridge perspective
  }

  DBPRINT2("b2b-cbu: CPU RAM External 0x%8x, begin shared 0x%08x\n", (unsigned int)cpuRamExternal, SHARED_OFFS);

  // clear shared mem
  i = 0;
  pSharedTemp        = (uint32_t *)(pShared + (COMMON_SHARED_BEGIN >> 2 ));
  while (pSharedTemp < (uint32_t *)(pShared + (B2B_SHARED_END >> 2 ))) {
    *pSharedTemp = 0x0;
    pSharedTemp++;
    i++;
  } // while pSharedTemp
  DBPRINT2("b2b-cbu: used size of shared mem is %d words, begin %x, end %x\n", i, (unsigned int)pShared, (unsigned int)pSharedTemp-1);
  } // initSharedMem


void extern_clearDiag() // clears all statistics
{
  statusArray = 0x0;
  nTransfer   = 0x0;
  transStat   = 0x0;
} // extern_clearDiag 


uint32_t extern_entryActionConfigured()
{
  uint32_t status = COMMON_STATUS_OK;

  // configure EB master (SRC and DST MAC/IP are set from host)
  if ((status = fwlib_ebmInit(2000, 0xffffffffffff, 0xffffffff, EBM_NOREPLY)) != COMMON_STATUS_OK) {
    DBPRINT1("b2b-cbu: ERROR - init of EB master failed! %u\n", (unsigned int)status);
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
  while (fwlib_wait4ECAEvent(1, &tDummy, &eDummy, &pDummy, &fDummy, &flagDummy) !=  COMMON_ECADO_TIMEOUT) {i++;}
  DBPRINT1("b2b-cbu: ECA queue flushed - removed %d pending entries from ECA queue\n", i);

  // set initial nonsense values
  *pSharedTH1ExtHi   = 0x000000E8; // 1 kHz dummy
  *pSharedTH1ExtLo   = 0xD4A51000;
  *pSharedNHExt      = 1;
  *pSharedTH1InjHi   = 0x000001D1; // 2 kHz dummy
  *pSharedTH1InjLo   = 0xA94A2000;
  *pSharedNHInj      = 1;
  *pSharedTBeatHi    = 0x000000E8; // 1 kHz dummy
  *pSharedTBeatLo    = 0xD4A51000;
  *pSharedPcFixExt   = 0x0;
  *pSharedPcFixInj   = 0x0;
  *pSharedPcVarExt   = 0x0;
  *pSharedPcVarInj   = 0x0;
  *pSharedKcFixExt   = 0x0;
  *pSharedKcFixInj   = 0x0;

  return COMMON_STATUS_OK;
} // extern_entryActionOperation


uint32_t extern_exitActionOperation()
{
  return COMMON_STATUS_OK;
} // extern_exitActionOperation


uint32_t calcPhaseMatch(uint64_t *tPhaseMatch, uint64_t *TBeat)  // calculates when extraction and injection machines are synchronized
{
  uint64_t TSlow;                                   // period of 'slow' frequency     [as] // sic! atoseconds
  uint64_t TFast;                                   // period of 'fast' frequency     [as]
  uint64_t tSlow;                                   // phase of 'slow' frequency      [as]
  uint64_t tFast;                                   // phase of 'fast' frequency      [as]
  uint64_t nHSlow;                                  // harmonic number of 'slow' frequency
  uint64_t nHFast;                                  // harmonic number of 'fast' frequency
  uint64_t TMatch;                                  // 'period' till next match       [as]
  uint64_t tMatch;                                  // phase of best match            [as]
  uint64_t Tdiff;                                   // difference between periods     [as]
  uint64_t TFastTmp;                                // temporary variable             [as]
  uint64_t epoch;                                   // temporary epoch                [>>n<<s]
  uint64_t tNow;                                    // current time                   [ns]
  uint64_t nineO = 1000000000;                      // nine orders of magnitude, needed for conversion
  
  // define temporary epoch [ns]
  tNow    = getSysTime();
  epoch   = tNow - nineO * 1;                                                     // subtracting one second should be safe

  DBPRINT3("b2b-cbu: tNow - tH1Ext %u ns, tNow - tH1inj %u ns, nHExt %u, nHInj %u\n", (unsigned int)(tNow - tH1Ext), (unsigned int)(tNow - tH1Inj), nHExt, nHInj);

  // check for unreasonable values
  if (TH1Ext * nHInj == TH1Inj * nHExt) return COMMON_STATUS_OUTOFRANGE;           // no beating
  if (TH1Ext == 0)                      return COMMON_STATUS_OUTOFRANGE;           // no value for period
  if (TH1Inj == 0)                      return COMMON_STATUS_OUTOFRANGE;           // no value for period
  if (nHExt  == 0)                      return COMMON_STATUS_OUTOFRANGE;           // no value for harmonic number
  if (nHInj  == 0)                      return COMMON_STATUS_OUTOFRANGE;           // no value for harmonic number
  if (TH1Inj == 0)                      return COMMON_STATUS_OUTOFRANGE;           // no value for period
  if ((tH1Ext + nineO * 0.1) < tNow)    return COMMON_STATUS_OUTOFRANGE;           // value older than 100ms
  if ((tH1Inj + nineO * 0.1) < tNow)    return COMMON_STATUS_OUTOFRANGE;           // value older than 100ms

  // assign local values and convert times 't' to [as], periods 'T' are already in [as])
  //if (TH1Ext * nHInj > TH1Inj * nHExt) {
  if (TH1Ext * nHInj > TH1Inj * nHExt) {
    DBPRINT3("b2b-cbu: extraction is fast\n");
    TSlow  = TH1Ext;
    tSlow  = (tH1Ext - epoch) * nineO;
    nHSlow = nHExt;

    TFast  = TH1Inj;
    tFast  = (tH1Inj - epoch) * nineO;
    nHFast = nHInj;
  }
  else {
    DBPRINT3("b2b-cbu: injection is fast\n");
    TSlow  = TH1Inj;
    tSlow  = (tH1Inj - epoch) * nineO;
    nHSlow = nHInj;

    TFast  = TH1Ext;
    tFast  = (tH1Ext - epoch) * nineO;
    nHFast = nHExt;
  }

  // period of frequency beats [as], required if next match is too close
  *TBeat = (uint64_t)((double)TFast / (double)(TSlow * nHFast - TFast * nHSlow) * (double)TSlow);  // period of beating [as]

  // make sure tSlow is earlier than tFast; this is a must for the formula below
  // if not, subtract period
  while (tSlow > tFast)                      tFast = tFast + TFast;

  // make sure spacing between tSlow and tFast is not too large; otherwise we need to wait for too long
  while ((tFast - tSlow) > TFast           ) tFast = tFast - TFast;

  // make sure there is sufficient spacing between tSlow and tFast; this is a fudge thing
  //while ((tFast - tSlow) < (TFast / nHFast)) tFast = tFast + TFast;
  if ((tFast - tSlow) < nineO) tMatch = tSlow;   // we are closer than 1ns ==> done!
  else {
    DBPRINT3("b2b-cbu: tSlow %llu as, tFast %llu as, diff %llu as\n", tSlow, tFast, tFast - tSlow);
    
    // now, tSlow is earlier than tFast and both values are at most one period apart
    // we can now start our calculation
    Tdiff = TSlow * nHFast - TFast * nHSlow;
    DBPRINT3("b2b-cbu: TSlow * nHFast %llu as, TFast * nHSlow  %llu as, difference  %llu as\n", TSlow * nHFast, TFast * nHSlow, Tdiff);
    DBPRINT3("b2b-cbu: TSlow          %llu as, TFast           %llu as, difference  %llu as\n", TSlow, TFast, TSlow - TFast);
    DBPRINT3("b2b-cbu: tSlow          %llu as, tFast           %llu as, difference  %llu as\n", tSlow, tFast, tFast - tSlow);
    
    /*
    // brute force; keep this for explaining the algorithm
    uint64_t i;
    uint64_t tFastTmp;                                // temporary variable             [as]
    uint64_t tSlowTmp;                                // temporary variable             [as]
    uint64_t TSlowTmp;                                // temporary variable             [as] 
    
    tSlowTmp = tSlow;
    TSlowTmp = TSlow * nHFast;
    tFastTmp = tFast;
    TFastTmp = TFast * nHSlow; 
    i        = 0;
    DBPRINT3("b2b-cbu: TSlowTmp %llu as, TFastTmp %llu as \n", TSlowTmp, TFastTmp);
    while (tFastTmp > tSlowTmp) {
    tFastTmp += TFastTmp;
    tSlowTmp += TSlowTmp;
    i++;
    } //
    tMatch = tFastTmp;
    TMatch = tMatch - tSlow;
    DBPRINT3("b2b-cbu: TBeat %llu as, TMatch %llu as, i %llu\n", *TBeat, TMatch, i);
    DBPRINT3("b2b-cbu: tMatch %llu, TMatch %llu\n", tMatch, TMatch);
    */
    
    // calculate when phases will match
    TFastTmp = TFast * nHSlow; 
    TMatch = ((tFast - tSlow) / Tdiff) * TFastTmp + tFast - tSlow;
    tMatch = TMatch + tSlow;
    DBPRINT3("b2b-cbu: tMatch %llu, TMatch %llu, TBeat %llu\n", tMatch, TMatch, *TBeat);
  } // values are further apart than 1ns
  // check, that tMatch is further in the future than COMMON_AHEADT; if not, add one beating period
  if((tSlow + (uint64_t)COMMON_AHEADT * nineO) > tMatch) {
    tMatch += *TBeat;
    DBPRINT2("b2b-cbu: tMatch %llu, TMatch %llu, TBeat %llu (+ TBeat)\n", tMatch, TMatch, *TBeat);
    DBPRINT2("b2b-cbu: tSlow          %llu as, tFast           %llu as, difference  %llu as\n", tSlow, tFast, tFast - tSlow);
  }
 
  // convert back to [ns] and get rid of temporary epoch
  *tPhaseMatch = (uint64_t)((double)tMatch / (double)nineO) + epoch;

  if (*tPhaseMatch < tNow) DBPRINT3("b2b-cbu: err -- now - match %u ns\n", (unsigned int)(tNow - *tPhaseMatch));
  else                     DBPRINT3("b2b-cbu: ok  -- match - now %u ns\n", (unsigned int)(*tPhaseMatch - tNow));

  return COMMON_STATUS_OK;
    
} // calcPhaseMatch


uint32_t doActionOperation(uint32_t actStatus)                // actual status of firmware
{
  uint32_t status;                                            // status returned by routines
  uint32_t flagIsLate;                                        // flag indicating that we received a 'late' event from ECA
  uint32_t ecaAction;                                         // action triggered by event received from ECA
  uint64_t sendDeadline;                                      // deadline to send
  uint64_t sendEvtId;                                         // evtID to send
  uint64_t sendParam;                                         // param to send
  uint64_t recDeadline;                                       // deadline received
  uint64_t recId;                                             // evt ID received
  uint64_t recParam;                                          // param received
  uint32_t recTEF;                                            // TEF received
  uint64_t tMatch;                                            // time when phases of injecion and extraction match
  uint64_t TBeat;                                             // period of beating

  status = actStatus;

  ecaAction = fwlib_wait4ECAEvent(COMMON_ECATIMEOUT, &recDeadline, &recId, &recParam, &recTEF, &flagIsLate);
    
  switch (ecaAction) {
    case B2B_ECADO_B2B_START :
      // received: B2B_START from DM
      
      TH1Ext          = (uint64_t)(*pSharedTH1ExtHi) << 32;
      TH1Ext          = (uint64_t)(*pSharedTH1ExtLo) | TH1Ext;
      nHExt           = *pSharedNHExt;
      TH1Inj          = (uint64_t)(*pSharedTH1InjHi) << 32;
      TH1Inj          = (uint64_t)(*pSharedTH1InjLo) | TH1Inj;
      nHInj           = *pSharedNHInj;
      tH1Ext          = 0x0;
      tH1Inj          = 0x0;
      *pSharedTBeatHi = 0x0;
      *pSharedTBeatLo = 0x0;
      pcFixExt        = *pSharedPcFixExt;
      pcFixInj        = *pSharedPcFixInj;
      pcVarExt        = *pSharedPcVarExt;
      pcVarInj        = *pSharedPcVarInj;
      kcFixExt        = *pSharedKcFixExt;
      kcFixInj        = *pSharedKcFixInj;

      // send command: phase measurement at extraction machine
      sendEvtId    = 0x1000000000000000;                                        // FID
      sendEvtId    = sendEvtId | ((uint64_t)B2B_GID << 48);                 // GID chk hackish 
      sendEvtId    = sendEvtId | ((uint64_t)B2B_ECADO_B2B_PMEXT << 36);     // EVTNO
      sendEvtId    = sendEvtId | (uint64_t)(nHExt & 0xf);                       // RESERVED, use only four bits
      sendParam    = TH1Ext;
      sendDeadline = getSysTime() + (uint64_t)COMMON_AHEADT;
      fwlib_ebmWriteTM(sendDeadline, sendEvtId, sendParam);
      
      // send command: phase measurement at injection machine
      sendEvtId    = 0x1000000000000000;                                        // FID
      sendEvtId    = sendEvtId | ((uint64_t)B2B_GID << 48);                 // GID chk hackish 
      sendEvtId    = sendEvtId | ((uint64_t)B2B_ECADO_B2B_PMINJ << 36);     // EVTNO
      sendEvtId    = sendEvtId | (uint64_t)(nHInj & 0xf);                       // RESERVED, use only four bits
      sendParam    = TH1Inj;
      sendDeadline = getSysTime() + (uint64_t)COMMON_AHEADT;
      fwlib_ebmWriteTM(sendDeadline, sendEvtId, sendParam);
      
      DBPRINT3("b2b-cbu: got B2B_START\n");
      
      nTransfer++;
      transStat    = B2B_FLAG_TRANSACTIVE;
      
      break;
    case B2B_ECADO_B2B_PREXT :
      // received: measured phase from extraction machine
      // do some math
      tH1Ext        = recParam + pcFixExt + pcVarExt;
      sendDeadline  = tH1Ext + ((uint64_t)100000 * TH1Ext) / (uint64_t)1000000000; // project 100000 periods into the future
      
      // send DIAGEXT to extraction machine
      sendEvtId     = 0x1000000000000000;                                       // FID
      sendEvtId    = sendEvtId | ((uint64_t)B2B_GID << 48);                 // GID chk hackish 
      sendEvtId     = sendEvtId | ((uint64_t)B2B_ECADO_B2B_DIAGEXT << 36);  // EVTNO
      sendParam     = 0x0;
      fwlib_ebmWriteTM(sendDeadline, sendEvtId, sendParam);
      
      transStat     = transStat | B2B_FLAG_TRANSPEXT;
      break;
    case B2B_ECADO_B2B_PRINJ :
      // received: measured phase from injection machine
      // do some math
      tH1Inj        = recParam + pcFixInj + pcVarInj;
      sendDeadline  = tH1Inj + ((uint64_t)100000 * TH1Inj) / (uint64_t)1000000000; // project 100000 periods into the future
      
      // send DIAGEXT to injection machine
      sendEvtId     = 0x1000000000000000;                                       // FID
      sendEvtId    = sendEvtId | ((uint64_t)B2B_GID << 48);                 // GID chk hackish 
      sendEvtId     = sendEvtId | ((uint64_t)B2B_ECADO_B2B_DIAGINJ << 36);  // EVTNO
      sendParam     = 0x0;
      fwlib_ebmWriteTM(sendDeadline, sendEvtId, sendParam);
      
      transStat     = transStat | B2B_FLAG_TRANSPINJ;
      break;
      
    default :
      break;
  } // switch ecaAction

  // we have everything we need 
  if (transStat == (B2B_FLAG_TRANSACTIVE | B2B_FLAG_TRANSPEXT | B2B_FLAG_TRANSPINJ)) {

    DBPRINT2("b2b: we have everything we need\n");
    
    if ((status = calcPhaseMatch(&tMatch, &TBeat)) != COMMON_STATUS_OK) {
      transStat = 0x0;
      return status;
    } // if status

    // DIAGMATCH
    sendDeadline = tMatch;                                                    // deadline
    sendEvtId    = 0x1000000000000000;                                        // FID
    sendEvtId    = sendEvtId | ((uint64_t)B2B_GID << 48);                 // GID chk hackish     
    sendEvtId    = sendEvtId | ((uint64_t)B2B_ECADO_B2B_DIAGMATCH << 36); // EVTNO
    sendParam    = 0x0;
    fwlib_ebmWriteTM(sendDeadline, sendEvtId, sendParam);

    // KICKEXT
    sendDeadline = tMatch + kcFixExt;                                         // deadline
    sendEvtId    = 0x1000000000000000;                                        // FID
    sendEvtId    = sendEvtId | ((uint64_t)B2B_GID << 48);                 // GID chk hackish     
    sendEvtId    = sendEvtId | ((uint64_t)B2B_ECADO_B2B_KICKEXT << 36);   // EVTNO
    sendParam    = 0x0;
    fwlib_ebmWriteTM(sendDeadline, sendEvtId, sendParam);

    // KICKINJ
    sendDeadline = tMatch + kcFixInj;                                         // deadline
    sendEvtId    = 0x1000000000000000;                                        // FID
    sendEvtId    = sendEvtId | ((uint64_t)B2B_GID << 48);                 // GID chk hackish     
    sendEvtId    = sendEvtId | ((uint64_t)B2B_ECADO_B2B_KICKINJ << 36);   // EVTNO
    sendParam    = 0x0;
    fwlib_ebmWriteTM(sendDeadline, sendEvtId, sendParam);

    *pSharedTBeatHi = (uint32_t)(TBeat >> 32);
    *pSharedTBeatLo = (uint32_t)(TBeat && 0xffffffff);

    transStat = 0x0; 
  } // if transStat

  return status;
} // doActionOperation


int main(void) {
  uint32_t status;                              // (error) status
  uint32_t actState;                            // actual FSM state
  uint32_t pubState;                            // published state value
  uint32_t reqState;                            // requested FSM state
  uint32_t dummy1;                              // dummy parameter
  uint32_t *buildID;                            // WB address of build ID

  // init local variables
  buildID        = (uint32_t *)(INT_BASE_ADR + BUILDID_OFFS);                 // required for 'stack check'

  reqState       = COMMON_STATE_S0;
  actState       = COMMON_STATE_UNKNOWN;
  pubState       = COMMON_STATE_UNKNOWN;
  status         = COMMON_STATUS_OK;
  nTransfer      = 0x0;

  init();                                                                     // initialize stuff for lm32
  initSharedMem();                                                            // initialize shared memory
  fwlib_init((uint32_t *)_startshared, cpuRamExternal, SHARED_OFFS, "b2b-cbu", B2BCBU_FW_VERSION); // init common stuff
  fwlib_clearDiag();                                                          // clear common diagnostic data

  while (1) {
    check_stack_fwid(buildID);
    fwlib_cmdHandler(&reqState, &dummy1);                                     // check for commands and possibly request state changes
    status = COMMON_STATUS_OK;                                                // reset status for each iteration

    // state machine
    status = fwlib_changeState(&actState, &reqState, status);                 // handle requested state changes
    switch(actState) {                                                        // state specific do actions
      case COMMON_STATE_OPREADY :
        status = doActionOperation(status);
        if (status == COMMON_STATUS_WRBADSYNC)      reqState = COMMON_STATE_ERROR;
        if (status == COMMON_STATUS_ERROR)          reqState = COMMON_STATE_ERROR;
        break;
      default :                                                               // avoid flooding WB bus with unnecessary activity
        status = fwlib_doActionState(&reqState, actState, status);
        break;
    } // switch
    
    // update shared memory
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
    pubState          = actState;
    fwlib_publishState(pubState);
    fwlib_publishTransferStatus(nTransfer, 0x0, transStat);
  } // while

  return (1); // this should never happen ...
} // main
