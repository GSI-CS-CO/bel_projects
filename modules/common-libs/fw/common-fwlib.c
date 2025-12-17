/********************************************************************************************
 *  common-fwlib.c
 *
 *  created : 2019
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 17-Dec-2025
 *
 *  common functions used by various firmware projects
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
/*#include "irq.h"*/
#include "dbg.h"
#include "ebm.h"
#include "pp-printf.h"
#include "mini_sdb.h"
#include "aux.h"
#include "uart.h"
#include "../../../top/gsi_scu/scu_mil.h"                               // register layout MIL piggy
#include "../../oled_display/oled_regs.h"                               // register layout of OLED display
#include "../../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"   // register layout ECA queue
#include "../../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"         // register layout ECA control
// #include "../../../ip_cores/wr-cores/modules/wr_pps_gen/pps_gen_regs.h" // useless register layout, I can't handle this wbgen stuff here
#include "../../ip_cores/saftlib/src/eca_flags.h"                       // definitions for ECA queue
#include "../../../tools/wb_slaves.h"                                   // Dietrichs hackish solution for defining register layout

/* includes for this project */
#include <common-defs.h>                                                // common definitions
#include <common-fwlib.h>                                               // fwlib

#include <common-core.c>

// these routines are typically application specific
extern void     extern_clearDiag();
extern uint32_t extern_entryActionConfigured();
extern uint32_t extern_entryActionOperation();
extern uint32_t extern_exitActionOperation();

volatile uint32_t *pECAQ;               // WB address of ECA queue
volatile uint32_t *pEca;                // WB address of ECA event input (discoverPeriphery())
volatile uint32_t *pPPSGen;             // WB address of PPS Gen
volatile uint32_t *pWREp;               // WB address of WR Endpoint
volatile uint32_t *pIOCtrl;             // WB address of IO Control
volatile uint32_t *pMILPiggy;           // WB address of MIL device bus (MIL piggy)
volatile uint32_t *pOLED;               // WB address of OLED (display)
volatile uint32_t *pSbMaster;           // WB address of SCU bus master

// global variables
uint32_t *pSharedVersion;               // pointer to a "user defined" u32 register; here: publish version
uint32_t *pSharedStatusArrayLo;         // pointer to a "user defined" u32 register; here: publish OR of all (actual) error bits; low word
uint32_t *pSharedStatusArrayHi;         // pointer to a "user defined" u32 register; here: publish OR of all (actual) error bits; high word
uint32_t *pSharedNBadStatus;            // pointer to a "user defined" u32 register; here: publish # of bad status (=error) incidents
uint32_t *pSharedNBadState;             // pointer to a "user defined" u32 register; here: publish # of bad state (=FATAL, ERROR, UNKNOWN) incidents
volatile uint32_t *pSharedCmd;          // pointer to a "user defined" u32 register; here: get command from host
uint32_t *pSharedState;                 // pointer to a "user defined" u32 register; here: publish status
volatile uint32_t *pSharedData4EB;      // pointer to a "user defined" u32 register; here: memory region for receiving EB return values
uint32_t *pSharedMacHi;                 // pointer to a "user defined" u32 register; here: high bits of MAC
uint32_t *pSharedMacLo;                 // pointer to a "user defined" u32 register; here: low bits of MAC
uint32_t *pSharedIp;                    // pointer to a "user defined" u32 register; here: IP
uint32_t *pSharedTDiagHi;               // pointer to a "user defined" u32 register; here: time when diag was cleared, high bits
uint32_t *pSharedTDiagLo;               // pointer to a "user defined" u32 register; here: time when diag was cleared, low bits
uint32_t *pSharedTS0Hi;                 // pointer to a "user defined" u32 register; here: time when FW was in S0 state, high bits
uint32_t *pSharedTS0Lo;                 // pointer to a "user defined" u32 register; here: time when FW was in S0 state, low bits
uint32_t *pSharedNTransfer;             // pointer to a "user defined" u32 register; here: # of transfers
uint32_t *pSharedNInject;               // pointer to a "user defined" u32 register; here: # of injections within current transfer
uint32_t *pSharedTransStat;             // pointer to a "user defined" u32 register; here: status of transfer
uint32_t *pSharedNLate;                 // pointer to a "user defined" u32 register; here: number of ECA 'late' incidents                                            
uint32_t *pSharedNEarly;                // pointer to a "user defined" u32 register; here: number of ECA 'early' incidents                                           
uint32_t *pSharedNConflict;             // pointer to a "user defined" u32 register; here: number of ECA 'conflict' incidents                                        
uint32_t *pSharedNDelayed;              // pointer to a "user defined" u32 register; here: number of ECA 'delayed' incidents                                         
uint32_t *pSharedNMissed;               // pointer to a "user defined" u32 register; here: number of incidents, when 'wait4eca' was called after the deadline        
uint32_t *pSharedOffsMissed;            // pointer to a "user defined" u32 register; here: if 'missed': offset deadline to start wait4eca; else '0'                  
uint32_t *pSharedComLatency;            // pointer to a "user defined" u32 register; here: if 'missed': offset start to stop wait4eca; else deadline to stop wait4eca
uint32_t *pSharedOffsDone;              // pointer to a "user defined" u32 register; here: offset event deadline to time when we are done [ns]
uint32_t *pSharedUsedSize;              // pointer to a "user defined" u32 register; here: size of (used) shared memory

uint32_t *cpuRamExternalData4EB;        // external address (seen from host bridge) of this CPU's RAM: field for EB return values

uint32_t nBadStatus;                    // # of bad status (=error) incidents
uint32_t nBadState;                     // # of bad state (=FATAL, ERROR, UNKNOWN) incidents
uint32_t flagRecover;                   // flag indicating auto-recovery from error state;


//---------------------------------------------------
// private routines
//---------------------------------------------------
void ebmClearSharedMem()
{
  uint32_t i;

  for (i=0; i< (COMMON_DATA4EBSIZE >> 2); i++) pSharedData4EB[i] = 0x0;
} //ebmClearSharedMem


uint32_t findPPSGen() //find WB address of WR PPS Gen
{
  pPPSGen = 0x0;

  // get Wishbone address for PPS Gen
  pPPSGen = find_device_adr(CERN, WR_PPS_GEN);

  if (!pPPSGen) {DBPRINT1("common-fwlib: can't find WR PPS Gen\n"); return COMMON_STATUS_ERROR;}
  else                                                           return COMMON_STATUS_OK;
} // findPPSGen


uint32_t findWREp() //find WB address of WR Endpoint
{
  pWREp = 0x0;

  pWREp = find_device_adr(WR_ENDPOINT_VENDOR, WR_ENDPOINT_PRODUCT);

  if (!pWREp) {DBPRINT1("common-fwlib: can't find WR Endpoint\n"); return COMMON_STATUS_ERROR;}
  else                                                          return COMMON_STATUS_OK;
} // findWREp


uint32_t findIOCtrl() // find WB address of IO Control
{
  pIOCtrl = 0x0;

  pIOCtrl = find_device_adr(IO_CTRL_VENDOR, IO_CTRL_PRODUCT);

  if (!pIOCtrl) {DBPRINT1("common-fwlib: can't find IO Control\n"); return COMMON_STATUS_ERROR;}
  else                                                           return COMMON_STATUS_OK;
} // findIOCtrol


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

  if (!pECAQ) {DBPRINT1("common-fwlib: can't find ECA queue\n"); return COMMON_STATUS_ERROR;}
  else                                                        return COMMON_STATUS_OK;
} // findECAQueue


uint32_t findMILPiggy() //find WB address of MIL Piggy
{
  pMILPiggy = 0x0;

  // get Wishbone address for MIL Piggy
  pMILPiggy = find_device_adr(GSI, SCU_MIL);

  if (!pMILPiggy) {DBPRINT1("common-fwlib: can't find MIL piggy\n"); return COMMON_STATUS_ERROR;}
  else                                                            return COMMON_STATUS_OK;
} // findMILPiggy


uint32_t findOLED() //find WB address of OLED
{
  pOLED = 0x0;

  // get Wishbone address for OLED
  pOLED = find_device_adr(OLED_SDB_VENDOR_ID, OLED_SDB_DEVICE_ID);

  if (!pOLED) {DBPRINT1("common-fwlib: can't find OLED\n"); return COMMON_STATUS_ERROR;}
  else                                                      return COMMON_STATUS_OK;
} // findOLED

uint32_t findSbMaster() //find WB address of SCU bus master
{
  pSbMaster = 0x0;

  // get Wishbone address for SCU bus master
  pSbMaster = find_device_adr(GSI, SCU_BUS_MASTER);

  if (!pSbMaster) {DBPRINT1("common-fwlib: can't find SCU bus master\n"); return COMMON_STATUS_ERROR;}
  else                                                      return COMMON_STATUS_OK;
} // findSbMaster

uint32_t exitActionError()
{
  return COMMON_STATUS_OK;
} // exitActionError

//---------------------------------------------------
// public routines
//---------------------------------------------------
b2bt_t fwlib_cleanB2bt(b2bt_t t_ps)
{
  while (t_ps.ps < -500) {t_ps.ns -= 1; t_ps.ps += 1000;}
  while (t_ps.ps >= 500) {t_ps.ns += 1; t_ps.ps -= 1000;}

  return t_ps;
} // alignB2bt


uint64_t fwlib_advanceTime(uint64_t t1, uint64_t t2, uint64_t Tas) // advance t2 to t > t1 [ns]
{
  uint64_t dtns;                // approximate time interval to advance [ns]
  uint64_t dtas;                // approximate time interval to advance [as]
  uint64_t nPeriods;            // # of periods
  uint64_t intervalAs;          // interval [as]
  uint64_t intervalNs;          // interval [ns]
  uint64_t tAdvanced;           // result
  uint64_t nineO = 1000000000;  // nine order of magnitude
  uint64_t half;                // helper variable

  if (Tas == 0)          return 0;
  if (t2 < t1)           return 0;               // order ok ?
  if ((t2 - t1) > nineO) return 0;               // not more than 1s! (~18 s max!)

  dtns       = t2 - t1;
  dtas       = dtns * nineO;
  nPeriods   = (uint64_t)((double)dtas / (double)Tas) + 1;
  intervalAs = nPeriods * Tas;
  half       = nineO >> 1;
  intervalNs = intervalAs / nineO;
  if (intervalAs % nineO > half) intervalNs++;
  tAdvanced  = t1 + intervalNs;

  return tAdvanced; // [ns]
} //fwlib_advanceTime


b2bt_t fwlib_advanceTimePs(b2bt_t t1_t, b2bt_t t2_t, uint64_t  T_as)
{
  uint64_t dt_ps;                    // approximate time interval to advance [ps]
  uint64_t dt_as;                    // approximate time interval to advance [as]
  uint64_t nPeriods;                 // # of periods
  uint64_t interval_as;              // interval [as]
  uint64_t interval_ps;              // interval [ps]
  uint64_t interval_ns;              // inverval [ns]
  b2bt_t   tAdvanced_t;              // result
  uint64_t half;                     // helper variable
  int64_t  fraction_as;              // helper variable
  uint64_t nineO = 1000000000;       // 9 orders of magnitude

  tAdvanced_t.ns  = 0;
  tAdvanced_t.ps  = 0;
  tAdvanced_t.dps = 0;

  if (T_as == 0)                       return tAdvanced_t; // no valid RF period
  if (t2_t.ns < t1_t.ns)               return tAdvanced_t; // order ok ?
  if ((t2_t.ns - t1_t.ns) > nineO)     return tAdvanced_t; // not more than 1s! (~18 s max!)

  dt_ps           = (t2_t.ns - t1_t.ns)*1000 + (uint64_t)(t2_t.ps - t1_t.ps);
  dt_as           = dt_ps * 1000000;
  nPeriods        = dt_as / T_as + 1;                      // division does an implicit 'floor': need to increment
  interval_as     = nPeriods * T_as;
  half            = nineO >> 1;
  interval_ns     = interval_as / nineO;
  fraction_as     = interval_as % nineO;

  if (fraction_as > half) {                                // rounding to ns
    interval_ns++;
    fraction_as -= nineO;
  } // if fraction
  tAdvanced_t.ns  = t1_t.ns + interval_ns;
  tAdvanced_t.ps  = t1_t.ps + fraction_as / 1000000;       // no rounding to ps
  tAdvanced_t.dps = t1_t.dps;

  return tAdvanced_t; // [ps]
} // fwlib_advanceTimePs


b2bt_t fwlib_tfns2tps(float t_ns)
{
  b2bt_t t_ps;

  t_ps.ns = t_ns;
  t_ps.ps = (t_ns - (float)(t_ps.ns)) * 1000.0;
  t_ps    = fwlib_cleanB2bt(t_ps);

  return t_ps;
} // tfns2ps


float fwlib_tps2tfns(b2bt_t t_ps)
{  
  float  tmp1, tmp2;
  
  tmp1 = (float)(t_ps.ns);
  tmp2 = (float)(t_ps.ps) / 1000.0;

  return tmp1 + tmp2;;
} // fwlib_tps2tfns


b2bt_t fwlib_tns2tps(uint64_t t_ns)
{
  b2bt_t t_ps;

  t_ps.ns  = t_ns;
  t_ps.ps  = 0;
  t_ps.dps = 0;

  return t_ps;
} // tns2tps


uint64_t fwlib_tps2tns(b2bt_t t_ps)              // time [ps]
{
  uint64_t t_ns;
  b2bt_t   ts_t;

  ts_t = fwlib_cleanB2bt(t_ps);                  // clean, includes rounding

  t_ns = ts_t.ns;

  return t_ns;
} // tps2tns


uint64_t fwlib_wrGetMac()  // get my own MAC
{
  uint32_t macHi, macLo;
  uint64_t mac;

  macHi = (*(pWREp + (WR_ENDPOINT_MACHI >> 2))) & 0xffff;
  macLo = *(pWREp + (WR_ENDPOINT_MACLO >> 2));

  mac = macHi;
  mac = (mac << 32);
  mac = mac + macLo;

  return mac;
} // fwlib_wrGetMac


uint32_t fwlib_ioCtrlSetGate(uint32_t enable, uint32_t io)  // set gate of LVDS input
{
  uint32_t offset;

  if ((enable != 1) && (enable != 0)) return COMMON_STATUS_OUTOFRANGE;
  if (io > 31)                        return COMMON_STATUS_OUTOFRANGE;

  if (enable) offset = IO_CTRL_LVDSINGATESETLOW;
  else        offset = IO_CTRL_LVDSINGATERESETLOW;

  *(pIOCtrl + (offset >> 2)) = (1 << io);

  return COMMON_STATUS_OK;
} // fwlib_ioCtrlSetGate


uint32_t fwlib_ebmInit(uint32_t msTimeout, uint64_t dstMac, uint32_t dstIp, uint32_t eb_ops) // intialize Etherbone master
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
  ebm_config_if(DESTINATION,  dstMac    , dstIp,                       0xebd0);
  ebm_config_if(SOURCE, fwlib_wrGetMac(), *(pEbCfg + (EBC_SRC_IP>>2)), 0xebd0);
  ebm_config_meta(1500, 0x0, eb_ops);
  ebm_clr();

  return COMMON_STATUS_OK;
} // fwlib_ebminit


uint32_t fwlib_ebmWriteN(uint32_t address, uint32_t *data, uint32_t n32BitWords)
{
  int i;

  if (n32BitWords > (COMMON_DATA4EBSIZE >> 2)) return COMMON_STATUS_OUTOFRANGE;
  if (n32BitWords == 0)                        return COMMON_STATUS_OUTOFRANGE;

  ebmClearSharedMem();                                                      // clear my shared memory used for EB replies

  ebm_hi(address);                                                          // EB operation starts here
  for (i=0; i<n32BitWords; i++) ebm_op(address + i*4, data[i], EBM_WRITE);  // put data into EB cycle
  if (n32BitWords == 1)         ebm_op(address      , data[0], EBM_WRITE);  // workaround runt frame issue
  ebm_flush();                                                              // commit EB cycle via the network

  return COMMON_STATUS_OK;
} // fwlib_ebmWriteN


uint32_t fwlib_ebmReadN(uint32_t msTimeout, uint32_t address, uint32_t *data, uint32_t n32BitWords)
{
  uint64_t timeoutT;
  int      i;
  uint32_t handshakeIdx;

  handshakeIdx = n32BitWords + 1;

  if (n32BitWords >= ( COMMON_DATA4EBSIZE >> 2)) return COMMON_STATUS_OUTOFRANGE;
  if (n32BitWords == 0)                          return COMMON_STATUS_OUTOFRANGE;
  if (pSharedData4EB == 0x0)                     return COMMON_STATUS_OUTOFRANGE;

  for (i=0; i< n32BitWords; i++) data[i] = 0x0;

  ebmClearSharedMem();                                                                               // clear shared data for EB return values
  pSharedData4EB[handshakeIdx] = COMMON_EB_HACKISH;                                                  // see below

  ebm_hi(address);                                                                                   // EB operation starts here
  for (i=0; i<n32BitWords; i++) ebm_op(address + i*4, (uint32_t)(&(cpuRamExternalData4EB[i])), EBM_READ);            // put data into EB cycle
                                ebm_op(address      , (uint32_t)(&(cpuRamExternalData4EB[handshakeIdx])), EBM_READ); // handshake data
  ebm_flush();                                                                                       // commit EB cycle via the network

  timeoutT = getSysTime() + (uint64_t)msTimeout * (uint64_t)1000000;
  while (getSysTime() < timeoutT) {                                                                  // wait for received data until timeout
    if (pSharedData4EB[handshakeIdx] != COMMON_EB_HACKISH) {                                         // hackish solution to determine if a reply value has been received
      for (i=0; i<n32BitWords; i++) data[i] = pSharedData4EB[i];
      // dbg mprintf("fwlib: ebmReadN EB_address 0x%08x, nWords %d, data[0] 0x%08x, hackish 0x%08x, return 0x%08x\n", address, n32BitWords, data[0], DMUNIPZ_EB_HACKISH, pSharedData4EB[handshakeIdx]);
      return COMMON_STATUS_OK;
    }
  } //while not timed out

  return COMMON_STATUS_EBREADTIMEDOUT;
} //fwlib_ebmReadN


uint64_t fwlib_buildEvtidV1(uint32_t gid, uint32_t evtno, uint32_t flags, uint32_t sid, uint32_t bpid, uint32_t reserved)
{
  uint64_t evtId;

  evtId    = 0x1000000000000000;                               //  1 bit
  evtId    = evtId | ((uint64_t)(gid      & 0x0fff) << 48);    // 12 bit
  evtId    = evtId | ((uint64_t)(evtno    & 0x0fff) << 36);    // 12 bit
  evtId    = evtId | ((uint64_t)(flags    & 0x000f) << 32);    //  4 bit
  evtId    = evtId | ((uint64_t)(sid      & 0x0fff) << 20);    // 12 bit
  evtId    = evtId | ((uint64_t)(bpid     & 0x3fff) <<  6);    // 14 bit
  evtId    = evtId | ((uint64_t)(reserved & 0x003f)      );    //  6 bit

  return evtId;
}
// fwlib_buildEvtidV1


uint32_t fwlib_ebmWriteTM(uint64_t deadline, uint64_t evtId, uint64_t param, uint32_t tef, uint32_t flagForceLate)
{
  uint32_t res;                        // temporary variables for bit shifting etc
  uint32_t deadlineLo, deadlineHi;
  uint32_t idLo, idHi;
  uint32_t paramLo, paramHi;

  uint32_t status;                     // return value

  // check deadline
  if ((deadline < getSysTime() + (uint64_t)(COMMON_LATELIMIT)) && !flagForceLate) {
    deadline = getSysTime() + (uint64_t)COMMON_AHEADT;
    status   = COMMON_STATUS_OUTOFRANGE;
  } // if deadline
  else status = COMMON_STATUS_OK;

  // set high bits for EB master
  ebm_hi(COMMON_ECA_ADDRESS);

  // pack Ethernet frame with message data
  idHi       = (uint32_t)((evtId >> 32)    & 0xffffffff);
  idLo       = (uint32_t)(evtId            & 0xffffffff);
  // tef already in proper format
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
  ebm_op(COMMON_ECA_ADDRESS, res,        EBM_WRITE);
  ebm_op(COMMON_ECA_ADDRESS, tef,        EBM_WRITE);
  ebm_op(COMMON_ECA_ADDRESS, deadlineHi, EBM_WRITE);
  ebm_op(COMMON_ECA_ADDRESS, deadlineLo, EBM_WRITE);
  atomic_off();

  // send timing message
  ebm_flush();

  return status;
} //fwlib_ebmWriteTM


uint32_t fwlib_ecaWriteTM(uint64_t deadline, uint64_t evtId, uint64_t param, uint32_t tef, uint32_t flagForceLate)
{
  uint32_t res;                        // temporary variables for bit shifting etc
  uint32_t deadlineLo, deadlineHi;
  uint32_t idLo, idHi;
  uint32_t paramLo, paramHi;

  uint32_t status;                     // return value

  // check deadline
  if ((deadline < getSysTime() + (uint64_t)(COMMON_LATELIMIT)) && !flagForceLate) {
    deadline = getSysTime() + (uint64_t)COMMON_AHEADT;
    status   = COMMON_STATUS_OUTOFRANGE;
  } // if deadline
  else status = COMMON_STATUS_OK;

  // pack 32bit words of message data
  idHi       = (uint32_t)((evtId >> 32)    & 0xffffffff);
  idLo       = (uint32_t)(evtId            & 0xffffffff);
  // tef already in proper format
  res     = 0x00000000;
  paramHi    = (uint32_t)((param >> 32)    & 0xffffffff);
  paramLo    = (uint32_t)(param            & 0xffffffff);
  deadlineHi = (uint32_t)((deadline >> 32) & 0xffffffff);
  deadlineLo = (uint32_t)(deadline         & 0xffffffff);

  // write timing message to ECA input
  atomic_on();
  *pEca = idHi;
  *pEca = idLo;
  *pEca = paramHi;
  *pEca = paramLo;
  *pEca = res;
  *pEca = tef;
  *pEca = deadlineHi;
  *pEca = deadlineLo;
  atomic_off();

  return status;
} //fwlib_ecaWriteTM


uint32_t fwlib_wrCheckSyncState() //check status of White Rabbit (link up, tracking)
{
  uint32_t syncState;

  syncState =  *(pPPSGen + (WR_PPS_GEN_ESCR >> 2));                         // read status
  syncState = syncState & WR_PPS_GEN_ESCR_MASK;                             // apply mask

  if ((syncState == WR_PPS_GEN_ESCR_MASK)) return COMMON_STATUS_OK;         // check if all relevant bits are set
  else                                     return COMMON_STATUS_WRBADSYNC;
} //fwlib_wrCheckStatus


void fwlib_publishSharedSize(uint32_t size)
{
  *pSharedUsedSize = size;
  DBPRINT2("common-fwlib: %u bytes of shared mem are actually used\n", size);

} // fwlib_publishSharedInfo


void fwlib_init(uint32_t *startShared, uint32_t *cpuRamExternal, uint32_t sharedOffs, uint32_t sharedSize, char * name, uint32_t fwVersion) // determine address and clear shared mem
{
  uint32_t *pSharedTemp;
  uint32_t *pShared;
  uint32_t commonSharedSize;
  int      i;

  // set pointer to shared memory
  pShared                 = startShared;

  // set pointer ('external view') for EB return values
  if (cpuRamExternal) cpuRamExternalData4EB   = (uint32_t *)(cpuRamExternal + ((COMMON_SHARED_DATA_4EB + sharedOffs) >> 2));
  else                DBPRINT1("common-fwlib: ERROR cpuRamExternal undefined\n");

  // get address to data
  pSharedVersion          = (uint32_t *)(pShared + (COMMON_SHARED_VERSION >> 2));
  pSharedStatusArrayHi    = (uint32_t *)(pShared + (COMMON_SHARED_STATUSHI >> 2));
  pSharedStatusArrayLo    = (uint32_t *)(pShared + (COMMON_SHARED_STATUSLO >> 2));
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
  pSharedNTransfer        = (uint32_t *)(pShared + (COMMON_SHARED_NTRANSFER >> 2));
  pSharedNInject          = (uint32_t *)(pShared + (COMMON_SHARED_NINJECT >> 2));
  pSharedTransStat        = (uint32_t *)(pShared + (COMMON_SHARED_TRANSSTAT >> 2));
  pSharedNLate            = (uint32_t *)(pShared + (COMMON_SHARED_NLATE >> 2));
  pSharedNEarly           = (uint32_t *)(pShared + (COMMON_SHARED_NEARLY >> 2));
  pSharedNConflict        = (uint32_t *)(pShared + (COMMON_SHARED_NCONFLICT >> 2));
  pSharedNDelayed         = (uint32_t *)(pShared + (COMMON_SHARED_NDELAYED >> 2));
  pSharedNMissed          = (uint32_t *)(pShared + (COMMON_SHARED_NMISSED >> 2));
  pSharedOffsMissed       = (uint32_t *)(pShared + (COMMON_SHARED_OFFSMISSED >> 2));
  pSharedComLatency       = (uint32_t *)(pShared + (COMMON_SHARED_COMLATENCY >> 2));
  pSharedOffsDone         = (uint32_t *)(pShared + (COMMON_SHARED_OFFSDONE >> 2));
  pSharedUsedSize         = (uint32_t *)(pShared + (COMMON_SHARED_USEDSIZE >> 2));

  // clear shared mem
  i = 0;
  pSharedTemp        = (uint32_t *)(pShared + (COMMON_SHARED_BEGIN >> 2 ));
  while (pSharedTemp < (uint32_t *)(pShared + (COMMON_SHARED_END >> 2 ))) {
    *pSharedTemp = 0x0;
    pSharedTemp++;
    i++;
  } // while pSharedTemp

  fwlib_publishSharedSize(sharedSize);                              // set shared size total
  commonSharedSize   = (uint32_t)(pSharedTemp - pShared) << 2;     // calc shared size common

  // basic info to wr console
  DBPRINT1("\n");
  DBPRINT1("common-fwlib: ***** firmware %s v%06x started from scratch *****\n", name, (unsigned int)fwVersion);
  DBPRINT1("common-fwlib: fwlib_init, shared size [bytes], common part %d, total %d\n", commonSharedSize, sharedSize);
  DBPRINT1("common-fwlib: cpuRamExternal 0x%08x,  cpuRamExternalData4EB 0x%08x\n", cpuRamExternal, cpuRamExternalData4EB);
  DBPRINT1("\n");

  // set initial values;
  ebmClearSharedMem();
  *pSharedVersion      = fwVersion; // of all the shared variables, only VERSION is a constant. Set it now!
  *pSharedNBadStatus   = 0;
  *pSharedNBadState    = 0;
  flagRecover          = 0;
} // fwlib_init


void fwlib_printOLED(char *chars)
{
  uint32_t i;

  if (!pOLED) return;                         // no OLED: just return

  for (i=0;i<strlen(chars);i++) *(pOLED + (OLED_UART_OWR >> 2)) = chars[i];
} // fwlib_printOLED


void fwlib_clearOLED()
{
  if (!pOLED) return;                         // no OLED: just return

  *(pOLED + (OLED_UART_OWR >> 2)) = 0xc;      // clear display
} // fwlib_clearOLED


uint32_t fwlib_wait4ECAEvent(uint32_t timeout_us, uint64_t *deadline, uint64_t *evtId, uint64_t *param, uint32_t *tef, uint32_t *isLate, uint32_t *isEarly, uint32_t *isConflict, uint32_t *isDelayed)  // 1. query ECA for actions, 2. trigger activity
{
  uint32_t *pECAFlag;           // address of ECA flag
  uint32_t ecaFlag;             // ECA flag
  uint32_t evtIdHigh;           // high 32bit of eventID
  uint32_t evtIdLow;            // low 32bit of eventID
  uint32_t evtDeadlHigh;        // high 32bit of deadline
  uint32_t evtDeadlLow;         // low 32bit of deadline
  uint32_t evtParamHigh;        // high 32 bit of parameter field
  uint32_t evtParamLow ;        // low 32 bit of parameter field
  uint32_t actTag;              // tag of action
  uint32_t nextAction;          // describes what to do next
  uint64_t timeoutT;            // when to time out
  uint64_t timeout;             // timeout [ns]


  pECAFlag    = (uint32_t *)(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2));   // address of ECA flag

  // conversion from ns -> us: use shift by 10 bits instead of multiplication by '1000'
  // reduces time per read from ~6.5 us to ~4.8 us
  //timeout     = ((uint64_t)timeout_us + 1) * 1000;
  timeout     = ((uint64_t)timeout_us + 1) << 10;
  timeoutT    = getSysTime() + timeout;

  while (getSysTime() < timeoutT) {
    ecaFlag = *pECAFlag;                                            // we'll need this value more than once per iteration
    if (ecaFlag & (0x0001 << ECA_VALID)) {                          // if ECA data is valid

      // read data
      evtIdHigh    = *(pECAQ + (ECA_QUEUE_EVENT_ID_HI_GET >> 2));
      evtIdLow     = *(pECAQ + (ECA_QUEUE_EVENT_ID_LO_GET >> 2));
      evtDeadlHigh = *(pECAQ + (ECA_QUEUE_DEADLINE_HI_GET >> 2));
      evtDeadlLow  = *(pECAQ + (ECA_QUEUE_DEADLINE_LO_GET >> 2));
      actTag       = *(pECAQ + (ECA_QUEUE_TAG_GET >> 2));
      evtParamHigh = *(pECAQ + (ECA_QUEUE_PARAM_HI_GET >> 2));
      evtParamLow  = *(pECAQ + (ECA_QUEUE_PARAM_LO_GET >> 2));
      *tef         = *(pECAQ + (ECA_QUEUE_TEF_GET >> 2));

      *isLate      = ecaFlag & (0x0001 << ECA_LATE);
      *isEarly     = ecaFlag & (0x0001 << ECA_EARLY);
      *isConflict  = ecaFlag & (0x0001 << ECA_CONFLICT);
      *isDelayed   = ecaFlag & (0x0001 << ECA_DELAYED);
      *deadline    = ((uint64_t)evtDeadlHigh << 32) + (uint64_t)evtDeadlLow;
      *evtId       = ((uint64_t)evtIdHigh    << 32) + (uint64_t)evtIdLow;
      *param       = ((uint64_t)evtParamHigh << 32) + (uint64_t)evtParamLow;

      // pop action from channel
      *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;

      // here: do s.th. according to tag
      nextAction = actTag;

      return nextAction;
    } // if data is valid
  } // while not timed out

  *deadline   = 0x0;
  *evtId      = 0x0;
  *param      = 0x0;
  *tef        = 0x0;
  *isLate     = 0x0;
  *isEarly    = 0x0;
  *isConflict = 0x0;
  *isDelayed  = 0x0;

  return COMMON_ECADO_TIMEOUT;
} // fwlib_wait4ECAEvent


uint32_t fwlib_wait4ECAEvent2(uint32_t timeout_us, uint64_t *deadline, uint64_t *evtId, uint64_t *param, uint32_t *tef, // 1. query ECA for actions, 2. trigger activity
                              uint32_t *isLate, uint32_t *isEarly, uint32_t *isConflict, uint32_t *isDelayed,
                              uint32_t *isMissed, uint32_t *offsMissed, uint32_t *comLatency)  
{
  uint32_t *pECAFlag;           // address of ECA flag
  uint32_t ecaFlag;             // ECA flag
  uint32_t evtIdHigh;           // high 32bit of eventID
  uint32_t evtIdLow;            // low 32bit of eventID
  uint32_t evtDeadlHigh;        // high 32bit of deadline
  uint32_t evtDeadlLow;         // low 32bit of deadline
  uint32_t evtParamHigh;        // high 32 bit of parameter field
  uint32_t evtParamLow ;        // low 32 bit of parameter field
  uint32_t actTag;              // tag of action
  uint32_t nextAction;          // describes what to do next
  uint64_t timeoutT;            // when to time out
  uint64_t timeout;             // timeout [ns]
  uint64_t startT;              // time when starting this routine
  uint64_t stopT;               // time when finishing this routine


  pECAFlag    = (uint32_t *)(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2));   // address of ECA flag

  // conversion from ns -> us: use shift by 10 bits instead of multiplication by '1000'
  // reduces time per read from ~6.5 us to ~4.8 us
  //timeout     = ((uint64_t)timeout_us + 1) * 1000;
  timeout     = ((uint64_t)timeout_us + 1) << 10;
  startT      = getSysTime();
  timeoutT    = startT + timeout;

  while (getSysTime() < timeoutT) {
    ecaFlag = *pECAFlag;                                            // we'll need this value more than once per iteration
    if (ecaFlag & (0x0001 << ECA_VALID)) {                          // if ECA data is valid

      // read data
      evtIdHigh    = *(pECAQ + (ECA_QUEUE_EVENT_ID_HI_GET >> 2));
      evtIdLow     = *(pECAQ + (ECA_QUEUE_EVENT_ID_LO_GET >> 2));
      evtDeadlHigh = *(pECAQ + (ECA_QUEUE_DEADLINE_HI_GET >> 2));
      evtDeadlLow  = *(pECAQ + (ECA_QUEUE_DEADLINE_LO_GET >> 2));
      actTag       = *(pECAQ + (ECA_QUEUE_TAG_GET >> 2));
      evtParamHigh = *(pECAQ + (ECA_QUEUE_PARAM_HI_GET >> 2));
      evtParamLow  = *(pECAQ + (ECA_QUEUE_PARAM_LO_GET >> 2));
      *tef         = *(pECAQ + (ECA_QUEUE_TEF_GET >> 2));

      *isLate      = ecaFlag & (0x0001 << ECA_LATE);
      *isEarly     = ecaFlag & (0x0001 << ECA_EARLY);
      *isConflict  = ecaFlag & (0x0001 << ECA_CONFLICT);
      *isDelayed   = ecaFlag & (0x0001 << ECA_DELAYED);
      *deadline    = ((uint64_t)evtDeadlHigh << 32) + (uint64_t)evtDeadlLow;
      *evtId       = ((uint64_t)evtIdHigh    << 32) + (uint64_t)evtIdLow;
      *param       = ((uint64_t)evtParamHigh << 32) + (uint64_t)evtParamLow;

      // pop action from channel
      *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;

      stopT        = getSysTime();

      // monitoring stuff
      if (*deadline < startT) {
        *isMissed   = 1;
        *offsMissed = (uint32_t)(startT - *deadline);
        *comLatency = (uint32_t)(stopT - startT);
      } // if missed
      else {
        *isMissed   = 0;
        *offsMissed = 0;
        *comLatency = (uint32_t)(stopT - *deadline);
      } // else missed

      // here: do s.th. according to tag
      nextAction = actTag;

      return nextAction;
    } // if data is valid
  } // while not timed out

  *deadline   = 0x0;
  *evtId      = 0x0;
  *param      = 0x0;
  *tef        = 0x0;
  *isLate     = 0x0;
  *isEarly    = 0x0;
  *isConflict = 0x0;
  *isDelayed  = 0x0;
  *isMissed   = 0x0;
  *offsMissed = 0x0;
  *comLatency = 0x0;

  return COMMON_ECADO_TIMEOUT;
} // fwlib_wait4ECAEvent2


// wait for MIL event or timeout
uint32_t fwlib_wait4MILEvent(uint32_t usTimeout, uint32_t *evtData, uint32_t *evtCode, uint32_t *virtAcc, uint32_t *validEvtCodes, uint32_t nValidEvtCodes)
{
  uint32_t evtRec;             // one MIL event
  uint32_t evtCodeRec;         // "event number"
  uint32_t evtDataRec;         // "event data"
  uint32_t virtAccRec;         // "virt Acc"
  uint64_t timeoutT;           // when to time out
  int      valid;              // evt is valid
  int      i;

  timeoutT    = getSysTime() + (uint64_t)usTimeout * (uint64_t)1000;
  *virtAcc    = 0xffff;
  *evtData    = 0xffff;
  *evtCode    = 0xffff;
  if (nValidEvtCodes == 0) valid = 1;           // just return with the first element read from FIFO
  else                     valid = 0;           // only return if element read from FIFO matches one of the validEvtcodes

  while(getSysTime() < timeoutT) {              // while not timed out...
    while (fifoNotemptyEvtMil(pMILPiggy, 0)) {  // while fifo contains data
      popFifoEvtMil(pMILPiggy, 0, &evtRec);
      evtCodeRec  = evtRec & 0x000000ff;        // extract event code
      virtAccRec  = (evtRec >> 8)  & 0x0f;      // extract virtual accelerator
      evtDataRec  = (evtRec >> 12) & 0x0f;      // extract event data

      for (i=0; i<nValidEvtCodes; i++) {        // loop over valid evt codes
        if (evtCodeRec == validEvtCodes[i]) {valid = 1; break;}
      } // for i

      if (valid) {
        *evtData     = evtDataRec;
        *evtCode     = evtCodeRec;
        *virtAcc     = virtAccRec;
        return COMMON_STATUS_OK;
      } // if valid;
    } // while fifo contains data
    asm("nop");                                 // wait a bit...
  } // while not timed out

  return COMMON_STATUS_TIMEDOUT;
} // fwlib_wait4MILEvent


void fwlib_milPulseLemo(uint32_t nLemo) // pulse lemo for debugging with scope
{
  uint32_t i;

  setLemoOutputEvtMil(pMILPiggy, 0, nLemo, 1);
  for (i=0; i< 10 * COMMON_US_ASMNOP; i++) asm("nop");
  setLemoOutputEvtMil(pMILPiggy, 0, nLemo, 0);
} // fwlib_milPulseLemo


void fwlib_initCmds() // init stuff for handling commands, trivial for now, will be extended
{
  //  initalize command value: 0x0 means 'no command'
  *pSharedCmd     = 0x0;
} // fwlib_initCmds


void fwlib_clearDiag()// clears all statistics
{
  uint64_t now;

  extern_clearDiag();

  nBadStatus = 0;
  nBadState  = 0;
  now = getSysTime();
  *pSharedTDiagHi = (uint32_t)(now >> 32);
  *pSharedTDiagLo = (uint32_t)now & 0xffffffff;

} // fwlib_clearDiag


uint32_t fwlib_doActionS0()
{
  uint32_t status = COMMON_STATUS_OK;
  uint64_t now;

  if (findECAQueue() != COMMON_STATUS_OK) status = COMMON_STATUS_ERROR;
  if (findPPSGen()   != COMMON_STATUS_OK) status = COMMON_STATUS_ERROR;
  if (findWREp()     != COMMON_STATUS_OK) status = COMMON_STATUS_ERROR;
  if (findIOCtrl()   != COMMON_STATUS_OK) status = COMMON_STATUS_ERROR;
  findOLED();       // ignore error; not every TR has OLED device
  findMILPiggy();   // ignore error; not every TR has a MIL piggy
  findSbMaster();   // ignore error; not every TR has SCU bus master

  now           = getSysTime();
  *pSharedTS0Hi = (uint32_t)(now >> 32);
  *pSharedTS0Lo = (uint32_t)now & 0xffffffff;

  fwlib_publishNICData();

  fwlib_initCmds();

  return status;
} // fwlib_doActionS0


volatile uint32_t* fwlib_getMilPiggy()
{
  return pMILPiggy;
} // fwlib_getMilPiggy


volatile uint32_t* fwlib_getOLED()
{
  return pOLED;
} // fwlib_getMilOLED

volatile uint32_t* fwlib_getSbMaster()
{
  return pSbMaster;
} // fwlib_getSbMaster

void fwlib_publishNICData()
{
  uint64_t mac;
  uint32_t ip;

  mac = fwlib_wrGetMac(pWREp);
  *pSharedMacHi = (uint32_t)(mac >> 32) & 0xffff;
  *pSharedMacLo = (uint32_t)(mac        & 0xffffffff);

  ip  = *(pEbCfg + (EBC_SRC_IP>>2));
  *pSharedIp    = ip;
} //fwlib_publishNICData


void fwlib_publishState(uint32_t state)
{
  *pSharedState = state;
} // fwlib_publishState


void fwlib_publishStatusArray(uint64_t statusArray)
{
  if (flagRecover) statusArray = statusArray |  ((uint64_t)0x1 << COMMON_STATUS_AUTORECOVERY);
  else             statusArray = statusArray & ~((uint64_t)0x1 << COMMON_STATUS_AUTORECOVERY);
  *pSharedStatusArrayHi = (uint32_t)(statusArray >> 32);
  *pSharedStatusArrayLo = (uint32_t)(statusArray & 0xffffffff);
} // fwlib_publishStatusArray


void fwlib_publishTransferStatus(uint32_t nTransfer, uint32_t nInject, uint32_t transStat, uint32_t nLate, uint32_t offsDone, uint32_t comLatency)
{
  *pSharedNTransfer  = nTransfer;
  *pSharedNInject    = nInject;
  *pSharedTransStat  = transStat;
  *pSharedNLate      = nLate;
  *pSharedOffsDone   = offsDone;
  *pSharedComLatency = comLatency;
} // fwlib_publishTransferStatus


void fwlib_publishTransferStatus2(uint32_t nTransfer, uint32_t nInject, uint32_t transStat, uint32_t nLate, uint32_t nEarly, uint32_t nConflict,
                                 uint32_t nDelayed, uint32_t nMissed, uint32_t offsMissed, uint32_t comLatency, uint32_t offsDone)
{
  *pSharedNTransfer  = nTransfer;
  *pSharedNInject    = nInject;
  *pSharedTransStat  = transStat;
  *pSharedNLate      = nLate;
  *pSharedNEarly     = nEarly;
  *pSharedNConflict  = nConflict;
  *pSharedNDelayed   = nDelayed;
  *pSharedNMissed    = nMissed;
  *pSharedOffsMissed = offsMissed;
  *pSharedComLatency = comLatency;
  *pSharedOffsDone   = offsDone;
} // fwlib_publishTransferStatus2


void fwlib_incBadStatusCnt()
{
  nBadStatus++;

  *pSharedNBadStatus = nBadStatus;
} // fwlib_incBadStatusCnt


void fwlib_incBadStateCnt()
{
  nBadState++;

  *pSharedNBadState = nBadState;
} // fwlib_incBadStateCnt


void fwlib_cmdHandler(uint32_t *reqState, uint32_t *cmd) // handle commands from the outside world
{

  *cmd = *pSharedCmd;

  if (*cmd) {                                // check, if the command is valid
    switch (*cmd) {                          // request state changes according to cmd
      case COMMON_CMD_CONFIGURE :
        *reqState =  COMMON_STATE_CONFIGURED;
        DBPRINT3("common-fwlib: received cmd %d\n", *cmd);
        break;
      case COMMON_CMD_STARTOP :
        *reqState = COMMON_STATE_OPREADY;
        DBPRINT3("common-fwlib: received cmd %d\n", *cmd);
        break;
      case COMMON_CMD_STOPOP :
        *reqState = COMMON_STATE_STOPPING;
        DBPRINT3("common-fwlib: received cmd %d\n", *cmd);
        break;
      case COMMON_CMD_IDLE :
        *reqState = COMMON_STATE_IDLE;
        DBPRINT3("common-fwlib: received cmd %d\n", *cmd);
        break;
      case COMMON_CMD_RECOVER :
        *reqState = COMMON_STATE_IDLE;
        DBPRINT3("common-fwlib: received cmd %d\n", *cmd);
        break;
      case COMMON_CMD_CLEARDIAG :
        DBPRINT3("common-fwlib: received cmd %d\n", *cmd);
        fwlib_clearDiag();
        break;
      default:
        DBPRINT3("common-fwlib: common_cmdHandler received unknown command '0x%08x'\n", *cmd);
        break;
    } // switch
    *pSharedCmd = 0x0;                       // reset cmd value in shared memory
  } // if command
} // fwlib_cmdHandler


uint32_t fwlib_changeState(uint32_t *actState, uint32_t *reqState, uint32_t actStatus)   //state machine; see common-defs.h for possible states and transitions
{
  uint64_t statusTransition = COMMON_STATUS_OK;
  uint32_t status;
  uint32_t nextState;

  // if something severe happened, perform implicitely allowed transition to ERROR or FATAL states
  // else                        , handle explicitcely allowed transitions

  if ((*reqState == COMMON_STATE_ERROR) || (*reqState == COMMON_STATE_FATAL)) {statusTransition = actStatus; nextState = *reqState;}
  else {
    nextState = *actState;                       // per default: remain in actual state without exit or entry action
    switch (*actState) {                         // check for allowed transitions: 1. determine next state, 2. perform exit or entry actions if required
      case COMMON_STATE_S0:
        if      (*reqState == COMMON_STATE_IDLE)       {                                                    nextState = *reqState;}
        break;
      case COMMON_STATE_IDLE:
        if      (*reqState == COMMON_STATE_CONFIGURED) {statusTransition = extern_entryActionConfigured();  nextState = *reqState;}
        break;
      case COMMON_STATE_CONFIGURED:
        if      (*reqState == COMMON_STATE_IDLE)       {                                                    nextState = *reqState;}
        else if (*reqState == COMMON_STATE_CONFIGURED) {statusTransition = extern_entryActionConfigured();  nextState = *reqState;}
        else if (*reqState == COMMON_STATE_OPREADY)    {statusTransition = extern_entryActionOperation();   nextState = *reqState;}
        break;
      case COMMON_STATE_OPREADY:
        if      (*reqState == COMMON_STATE_STOPPING)   {statusTransition = extern_exitActionOperation();    nextState = *reqState;}
        break;
      case COMMON_STATE_STOPPING:
        nextState = COMMON_STATE_CONFIGURED;      //automatic transition but without entryActionConfigured
      case COMMON_STATE_ERROR:
        if      (*reqState == COMMON_STATE_IDLE)       {statusTransition = exitActionError();               nextState = *reqState;}
        break;
      default:
        nextState = COMMON_STATE_S0;
        break;
    } // switch actState
  }  // else something severe happened

  // if the transition failed, transit to error state (except we are already in FATAL state)
  if ((statusTransition != COMMON_STATUS_OK) && (nextState != COMMON_STATE_FATAL)) nextState = COMMON_STATE_ERROR;

  // if the state changes
  if (*actState != nextState) {
    pp_printf("common-fwlib: changed to state %u\n", (unsigned int)nextState);
    *actState = nextState;
    status = statusTransition;
  } // if state change
  else  status = actStatus;

  *reqState = COMMON_STATE_UNKNOWN;              // reset requested state (= no change state requested)

  return status;
} //fwlib_changeState


// do state specific do action
uint32_t fwlib_doActionState(uint32_t *reqState, uint32_t actState, uint32_t status)
{
  int j;

  switch(actState) {                                                   // state specific do actions
    case COMMON_STATE_S0 :
      status = fwlib_doActionS0();                                     // important initialization that must succeed!
      if (status != COMMON_STATUS_OK) *reqState = COMMON_STATE_FATAL;  // failed:  -> FATAL
      else                            *reqState = COMMON_STATE_IDLE;   // success: -> IDLE
      break;
    case COMMON_STATE_ERROR :
      flagRecover = 1;                                                 // start autorecovery
      break;
    case COMMON_STATE_FATAL :
      fwlib_publishState(actState);
      pp_printf("common-fwlib: a FATAL error has occured. Good bye.\n");
      while (1) asm("nop"); // RIP!
        break;
    default :                                                             // avoid flooding WB bus with unnecessary activity
      for (j = 0; j < (COMMON_DEFAULT_TIMEOUT * COMMON_MS_ASMNOP); j++) { asm("nop"); }
  } // switch

  // autorecovery from state ERROR
  if (flagRecover) fwlib_doAutoRecovery(actState, reqState);

  return status;
} // fwlib_doActionState

// do autorecovery from error state
void fwlib_doAutoRecovery(uint32_t actState, uint32_t *reqState)
{
  switch (actState) {
    case COMMON_STATE_ERROR :
      DBPRINT3("common-fwlib: attempting autorecovery ERROR -> IDLE\n");
      uwait(10000000);
      *reqState = COMMON_STATE_IDLE;
      break;
    case COMMON_STATE_IDLE :
      DBPRINT3("common-fwlib: attempting autorecovery IDLE -> CONFIGURED\n");
      uwait(5000000);
      *reqState = COMMON_STATE_CONFIGURED;
      break;
    case COMMON_STATE_CONFIGURED :
      DBPRINT3("common-fwlib: attempting autorecovery CONFIGURED -> OPREADY\n");
      uwait(5000000);
      *reqState = COMMON_STATE_OPREADY;
      flagRecover = 0;
      break;
    default :
      break;
    } // switch actState
} // fwlib_doAutoRecovery


uint16_t fwlib_float2half(float f)
{
  return comcore_float2half(f);
} // fwlib_float2half


float fwlib_half2float(uint16_t h)
{
  return comcore_half2float(h);
} // fwlib_half2float
