/******************************************************************************
 *  b2blib.c
 *
 *  created : 2020
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 19-Aug-2024
 *
 * library for b2b
 *
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2013  Dietrich Beck
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
 * Last update: 17-May-2017
 ********************************************************************************************/
// standard includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

// etherbone
#include <etherbone.h>

// wr-unipz
#include <common-defs.h>                 // common definitions
#include <common-lib.h>                  // common routines
#include <common-core.h>                 // common core
#include <b2bcbu_shared_mmap.h>          // FW shared def
#include <b2b.h>                         // FW defs
#include <b2blib.h>                      // x86 library

// lm32 sdb
#define GSI           0x00000651
#define LM32_RAM_USER 0x54111351

// experimental hackish for debugging
#define DEBUGFNAME    "/tmp/b2blib.log" // name of logfile

// public variables
eb_socket_t  eb_socket;                 // EB socket
eb_address_t lm32_base;                 // lm32
eb_address_t b2b_cmd;                   // command, write
eb_address_t b2b_state;                 // state of state machine
uint32_t     b2b_flagDebug = 0;         // flag debug
FILE         *logfile = NULL;           // log file 

// application specific stuff
// set values
eb_address_t b2b_set_sidExt;            // SID for transfer    
eb_address_t b2b_set_gidExt;            // b2b GID of extraction ring
eb_address_t b2b_set_mode;              // mode of B2B transfer
eb_address_t b2b_set_TH1ExtHi;          // period of h=1 extraction, high bits
eb_address_t b2b_set_TH1ExtLo;          // period of h=1 extraction, low bits
eb_address_t b2b_set_nHExt;             // harmonic number of extraction RF
eb_address_t b2b_set_cTrigExt;          // kicker correction extraction
eb_address_t b2b_set_nBuckExt;          // bucket number extraction
eb_address_t b2b_set_cPhase;            // phase correction
eb_address_t b2b_set_fFinTune;          // flag: use fine tune
eb_address_t b2b_set_fMBTune;           // flag: use multi-beat tune
eb_address_t b2b_set_sidEInj;           // SID for transfer; value must equal sidExt    
eb_address_t b2b_set_gidInj;            // b2b GID offset of injection ring
eb_address_t b2b_set_lsidInj;           // LSA SID of injection ring
eb_address_t b2b_set_lbpidInj;          // LSA BPID of injection ring
eb_address_t b2b_set_lparamInjHi;       // LSA param of injection ring, high bits
eb_address_t b2b_set_lparamInjLo;       // LSA param of injection ring, low bits
eb_address_t b2b_set_TH1InjHi;          // period of h=1 injection, high bits
eb_address_t b2b_set_TH1InjLo;          // period of h=1 injection, low bits
eb_address_t b2b_set_nHInj;             // harmonic number of injection RF
eb_address_t b2b_set_cTrigInj;          // kicker correction injection
eb_address_t b2b_set_nBuckInj;          // bucket number injection
eb_address_t b2b_cmd;                   // command, write

// get values
eb_address_t b2b_get_gid;               // GID for transfer
eb_address_t b2b_get_sid;               // SID for transfer    
eb_address_t b2b_get_mode;              // mode of B2B transfer
eb_address_t b2b_get_TH1ExtHi;          // period of h=1 extraction, high bits
eb_address_t b2b_get_TH1ExtLo;          // period of h=1 extraction, low bits
eb_address_t b2b_get_nHExt;             // harmonic number of extraction RF
eb_address_t b2b_get_TH1InjHi;          // period of h=1 injection, high bits
eb_address_t b2b_get_TH1InjLo;          // period of h=1 injection, low bits
eb_address_t b2b_get_nHInj;             // harmonic number of injection RF
eb_address_t b2b_get_TBeatHi;           // period of beating, high bits
eb_address_t b2b_get_TBeatLo;           // period of beating, low bits
eb_address_t b2b_get_cPhase;            // phase correction
eb_address_t b2b_get_cTrigExt;          // kicker correction extraction
eb_address_t b2b_get_cTrigInj;          // kicker correction injection

#define WAITCMDDONE COMMON_DEFAULT_TIMEOUT * 1000 // use default timeout and convert to us to be sure the command is processed

uint64_t b2b_getSysTime()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec * (uint64_t)1000000000+ tv.tv_usec * (uint64_t)1000;
} // b2b_getSysTime()


const char* b2b_status_text(uint32_t code)
{  
  static char message[256];
  
  switch (code) {
    case B2B_STATUS_PHASEFAILED          : sprintf(message, "error %d, %s", code, "phase measurement failed"); break;                            
    case B2B_STATUS_TRANSFER             : sprintf(message, "error %d, %s", code, "transfer failed"); break;
    case B2B_STATUS_SAFETYMARGIN         : sprintf(message, "error %d, %s", code, "violation of safety margin for data master and timing network"); break;
    case B2B_STATUS_NORF                 : sprintf(message, "error %d, %s", code, "no RF signal detected"); break;
    case B2B_STATUS_LATEMESSAGE          : sprintf(message, "error %d, %s", code, "late timing message received"); break;
    case B2B_STATUS_NOKICK               : sprintf(message, "error %d, %s", code, "no kicker signal detected"); break;
    case B2B_STATUS_BADSETTING           : sprintf(message, "error %d, %s", code, "bad setting data"); break;
    default                              : sprintf(message, "%s", comlib_statusText(code)); break;
  } // switch code
  
  return message;
} // b2b_status_text


const char* b2b_state_text(uint32_t code)
{
  return comlib_stateText(code);
} // b2b_state_text


const char* b2b_version_text(uint32_t number)
{
  static char    version[32];

  sprintf(version, "%02x.%02x.%02x", (number & 0x00ff0000) >> 16, (number & 0x0000ff00) >> 8, number & 0x000000ff);

  return version;
} // b2b_version_text


double b2b_flsa2fdds(double flsa)
{
  double twoep32;
  double twoem32;
  double fclk;
  double fdds;

  twoep32 = pow(2,  32);
  twoem32 = pow(2, -32);
  fclk    = (double)B2B_F_CLK;

  fdds   = twoem32 * floor(twoep32 * flsa / fclk) * fclk;

  return fdds;
} // b2b_flsa2fdds


void b2b_t2secs(uint64_t ts, uint32_t *secs, uint32_t *nsecs)
{
  *nsecs = (uint32_t)(ts % 1000000000);
  *secs  = (uint32_t)(ts / 1000000000);
} // b2b_t2secs


double b2b_fixTS(double tsDiff, double   corr, uint64_t TH1As)
{
  double  ts0;                                         // timestamp with correction removed [ns]
  double  dtMatch;
  int64_t ts0as;                                       // t0 [as]
  int64_t remainder;                     
  int64_t half;
  int     flagNeg; 

  if (TH1As == 0) return tsDiff;                       // can't fix
  ts0  = tsDiff - corr;
  if (ts0 < 0) {ts0 = -ts0; flagNeg = 1;}              // make this work for negative numbers too
  else         flagNeg = 0;

  ts0as     = (int64_t)(ts0 * 1000000000.0);
  half      = TH1As >> 1;
  remainder = ts0as % TH1As;                                 
  if (remainder > half) ts0as = remainder - TH1As;
  else                  ts0as = remainder;
  dtMatch   = (double)ts0as / 1000000000.0;
  
  if (flagNeg) dtMatch = -dtMatch;

  return dtMatch + corr;                               // we have to add back the correction (!)
} //b2b_fixTS


void b2b_log(char *message){
  uint64_t ts;
  uint32_t secs;
  uint32_t nsecs;
  uint32_t msecs;

  if (!logfile) return;

  ts    = b2b_getSysTime();
  b2b_t2secs(ts, &secs, &nsecs);
  msecs = nsecs / 1000000;

  fprintf(logfile, "%12u.%03u: %s\n", secs, msecs, message);
  fflush(logfile);
} // b2b_log

void b2b_debug(uint32_t flagDebug)
{
  if (flagDebug > 0) {
    if (!logfile) logfile = fopen(DEBUGFNAME, "a");
    if (logfile) b2b_flagDebug = 1;
    else         b2b_flagDebug = 0;
  } // if flagdebug
  else {
    b2b_flagDebug = 0;
    if (logfile) fclose(logfile);
  } // else flagdebug
} // b2b_debug


uint32_t b2b_calc_max_sysdev_ps(uint64_t TH1_as, uint32_t nSamples, uint32_t printFlag)
{
  // internally: all units are fs
  // two systematic effects are considered
  // a: comb-like substructure; here the max systematic deviation is half the distance between comb-peaks
  // b: the sub-ns fractional part of TH1 is close to 1ns;
  //    example TH1 = 732.996993 ns or TH1 = 732.002932 ns
  //    however, we need to consider also higher integer of fractions of the full nanosecond;
  //    in the following following code, they are called 'harmonics'
  //    example h=2: TH1 = 732.496993 ns; h=3: TH1 = 732.332993; h=4: TH1 = 732.246993 ...
  //    (being close to the full nanosecond can be seen as h=1: TH1 = 732.996993)
  //    the problem with 'harmonics' is, that event many samples will not achieve to cover the full
  //    1ns range; in this case, a maximum systematic deviation of half the >>un<<covered range needs
  //    to be considered
  // the main problem is effect 'b', as the actual deviation depends on the position of the rf-phase (relative
  // to the full nanosecond) when the measurement starts

  uint64_t TH1_fs;                                // rf-period [fs];
  uint32_t one_ns_fs = 1000000;                   // conversion ns to fs
  uint32_t comb;                                  // distance between 'comb' peaks
  uint32_t dComb;                                 // uncertainty due to 'comb'
  uint32_t h;                                     // 'harmonic' integer divisor of sub-ns fraction of TH1 to full ns
  uint32_t hMax;                                  // maximum 'harmonic' to consider
  uint32_t fracT;                                 // sub-ns fraction of TH1
  uint32_t fracH = 0;                             // sub-ns fraction of T-harmonic
  uint32_t tmp;                                   // helper variable
  uint32_t covered = 0;                           // span of full nanosecond covered by timestamps
  uint32_t dSpan   = 0;                           // uncertainty due to uncovered span
  double   overcover;                             // overcovery of 1ns
  uint32_t dOvercover = 0;                        // uncertainty due to overcovering (a bit fudgy)
  uint32_t dSysMax;                               // maximum systematic deviation from true value

  // convert to fs
  TH1_fs = TH1_as / 1000;
  
  // ommit 1st timestamp
  nSamples--;
  
  dSysMax = 0.0;
  
  // calculate comb and respective hMax
  comb  = comcore_intdiv(one_ns_fs, nSamples);
  dComb = comb >> 1;                              // division by 2 as sub-ns fit is (max - min) / 2

  // calculate hMax and limit by jitter; we don't need to consider higher harmonics
  hMax  = nSamples;
  tmp   = one_ns_fs / B2B_WR_JITTER;
  if (tmp < hMax) hMax = tmp;

  // fractional part TH1 of ns
  fracT = one_ns_fs - TH1_fs % one_ns_fs;
  tmp   = one_ns_fs - fracT;
  if (tmp < fracT) fracT = tmp;

  // calculate harmonic
  // special treatment for h=1 requires the following if-statement
  if (fracT < (one_ns_fs / hMax)) tmp = one_ns_fs - fracT;
  else                            tmp = fracT;
  h     = comcore_intdiv(one_ns_fs, tmp);

  // only consider relevant harmonics
  if (h < hMax) {
    // fractional part of 'harmonic'
    tmp   = one_ns_fs / h;
    fracH = fracT % tmp;
    tmp   = tmp - fracH;
    if (tmp < fracH) fracH = tmp;

    covered = fracH * (nSamples - 1);
    tmp = one_ns_fs / h;
    if (covered < tmp) dSpan = tmp - covered;
    else               dSpan = 0;
    dSpan   = dSpan >> 1;                         // division by 2 as sub-ns fit is (max - min) / 2
  } // if h

  // large coverage
  overcover = (double)(covered * h) / (double)one_ns_fs;
  if (overcover > 1) dOvercover = dComb * overcover;

  dSysMax = dComb;
  if (dOvercover > dSysMax) dSysMax = dOvercover;
  if (dSpan      > dSysMax) dSysMax = dSpan;

  if (printFlag) {
    printf("calc maximum systematic devation [ps]\n");
    printf("  dJitter %13.3f\n", (double)B2B_WR_JITTER / 1000.0);
    printf("  dComb   %13.3f\n", (double)dComb         / 1000.0);
    printf("  dSpan   %13.3f\n", (double)dSpan         / 1000.0);
    printf("   fractT %13.3f\n", (double)fracT         / 1000.0);
    printf("   hMax   %13.3f\n", (double)hMax                  );
    printf("   h      %13.3f\n", (double)h                     );
    printf("   fractH %13.3f\n", (double)fracH         / 1000.0);
    printf("   covrd  %13.3f\n", (double)covered       / 1000.0);
    printf("  dOverCvd%13.3f\n", (double)dOvercover    / 1000.0);
    printf("  dSysMax %13.3f\n", (double)dSysMax       / 1000.0);
  } // if printFlag

  return dSysMax / 1000;
} //  b2b_calc_max_sysdev_ps


uint32_t b2b_firmware_open(uint64_t *ebDevice, const char* devName, uint32_t cpu, uint32_t *address)
{
  eb_status_t         status;
  eb_device_t         eb_device;  
  struct sdb_device   sdbDevice;                           // instantiated lm32 core
  int                 nDevices;                            // number of instantiated cores

  // b2b_debug(1);                                            /* enable/disable this for implicit debugging */
  b2b_log("open firmware");
  
  *ebDevice = 0x0;
  if (cpu != 0) return COMMON_STATUS_OUTOFRANGE;           // chk, only support 1st core (this is a quick hack)
  nDevices = 1;

  // open Etherbone device and socket 
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDRX|EB_DATAX, &eb_socket)) != EB_OK) return COMMON_STATUS_EB;
  if ((status = eb_device_open(eb_socket, devName, EB_ADDRX|EB_DATAX, 3, &eb_device)) != EB_OK) return COMMON_STATUS_EB;

  //  get Wishbone address of lm32 
  if ((status = eb_sdb_find_by_identity(eb_device, GSI, LM32_RAM_USER, &sdbDevice, &nDevices)) != EB_OK) return COMMON_STATUS_EB;

  lm32_base            = sdbDevice.sdb_component.addr_first;

  comlib_initShared(lm32_base, SHARED_OFFS);
  b2b_cmd              = lm32_base + SHARED_OFFS + COMMON_SHARED_CMD;
  b2b_set_sidExt       = lm32_base + SHARED_OFFS + B2B_SHARED_SET_SIDEEXT;     
  b2b_set_gidExt       = lm32_base + SHARED_OFFS + B2B_SHARED_SET_GIDEXT;     
  b2b_set_mode         = lm32_base + SHARED_OFFS + B2B_SHARED_SET_MODE;;   
  b2b_set_TH1ExtHi     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_TH1EXTHI;
  b2b_set_TH1ExtLo     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_TH1EXTLO;
  b2b_set_nHExt        = lm32_base + SHARED_OFFS + B2B_SHARED_SET_NHEXT;
  b2b_set_cTrigExt     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_CTRIGEXT;
  b2b_set_nBuckExt     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_NBUCKEXT;
  b2b_set_cPhase       = lm32_base + SHARED_OFFS + B2B_SHARED_SET_CPHASE;  
  b2b_set_nBuckExt     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_NBUCKEXT;
  b2b_set_fFinTune     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_FFINTUNE;
  b2b_set_fMBTune      = lm32_base + SHARED_OFFS + B2B_SHARED_SET_FMBTUNE;
  b2b_set_sidEInj      = lm32_base + SHARED_OFFS + B2B_SHARED_SET_SIDEINJ;     
  b2b_set_gidInj       = lm32_base + SHARED_OFFS + B2B_SHARED_SET_GIDINJ;
  b2b_set_lsidInj      = lm32_base + SHARED_OFFS + B2B_SHARED_SET_LSIDINJ;
  b2b_set_lbpidInj     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_LBPIDINJ;
  b2b_set_lparamInjHi  = lm32_base + SHARED_OFFS + B2B_SHARED_SET_LPARAMINJHI;
  b2b_set_lparamInjLo  = lm32_base + SHARED_OFFS + B2B_SHARED_SET_LPARAMINJLO;
  b2b_set_TH1InjHi     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_TH1INJHI;
  b2b_set_TH1InjLo     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_TH1INJLO;
  b2b_set_nHInj        = lm32_base + SHARED_OFFS + B2B_SHARED_SET_NHINJ;
  b2b_set_cTrigInj     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_CTRIGINJ;
  b2b_set_nBuckInj     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_NBUCKINJ;
  
  b2b_get_gid          = lm32_base + SHARED_OFFS + B2B_SHARED_GET_GID;
  b2b_get_sid          = lm32_base + SHARED_OFFS + B2B_SHARED_GET_SID;
  b2b_get_mode         = lm32_base + SHARED_OFFS + B2B_SHARED_GET_MODE;;
  b2b_get_TH1ExtHi     = lm32_base + SHARED_OFFS + B2B_SHARED_GET_TH1EXTHI;
  b2b_get_TH1ExtLo     = lm32_base + SHARED_OFFS + B2B_SHARED_GET_TH1EXTLO;
  b2b_get_nHExt        = lm32_base + SHARED_OFFS + B2B_SHARED_GET_NHEXT;
  b2b_get_TH1InjHi     = lm32_base + SHARED_OFFS + B2B_SHARED_GET_TH1INJHI;
  b2b_get_TH1InjLo     = lm32_base + SHARED_OFFS + B2B_SHARED_GET_TH1INJLO;
  b2b_get_nHInj        = lm32_base + SHARED_OFFS + B2B_SHARED_GET_NHINJ;
  b2b_get_TBeatHi      = lm32_base + SHARED_OFFS + B2B_SHARED_GET_TBEATHI;
  b2b_get_TBeatLo      = lm32_base + SHARED_OFFS + B2B_SHARED_GET_TBEATLO;
  b2b_get_cPhase       = lm32_base + SHARED_OFFS + B2B_SHARED_GET_CPHASE;
  b2b_get_cTrigExt     = lm32_base + SHARED_OFFS + B2B_SHARED_GET_CTRIGEXT;
  b2b_get_cTrigInj     = lm32_base + SHARED_OFFS + B2B_SHARED_GET_CTRIGINJ;

  // do this just at the very end
  *ebDevice = (uint64_t)eb_device;

  return COMMON_STATUS_OK;
} // b2b_firmware_open


uint32_t b2b_firmware_close(uint64_t ebDevice)
{
  eb_status_t status;
  eb_device_t eb_device;

  b2b_log("close firmware");
  
  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;
  
  // close Etherbone device and socket
  if ((status = eb_device_close(eb_device)) != EB_OK) return COMMON_STATUS_EB;
  if ((status = eb_socket_close(eb_socket)) != EB_OK) return COMMON_STATUS_EB;

  b2b_debug(0);

  return COMMON_STATUS_OK;
} // b2b_firmware_close


uint32_t b2b_version_firmware(uint64_t ebDevice, uint32_t *version)
{
  uint64_t       dummy64a;
  uint32_t       dummy32a, dummy32b, dummy32c, dummy32d;
  uint32_t       errorCode;

  if (!ebDevice) return COMMON_STATUS_EB;
  *version = 0xffffffff;
  
  errorCode = b2b_common_read(ebDevice, &dummy64a, &dummy32a, &dummy32b, &dummy32c, version, &dummy32d, 0);

  return errorCode;
} // b2b_version_firmware


uint32_t b2b_version_library(uint32_t *version)
{
  *version = (uint32_t)B2BLIB_VERSION;

  return COMMON_STATUS_OK;
} // b2b_version_library


void b2b_printDiag(uint32_t sid, uint32_t gid, uint32_t mode, uint64_t TH1Ext, uint32_t nHExt, uint64_t TH1Inj, uint32_t nHInj, uint64_t TBeat, float cPhase, float cTrigExt, float cTrigInj)
{
  printf("b2b: info  ...\n\n");

  printf("SID                                 : %012u\n"     , sid);
  printf("GID                                 : %012u\n"     , gid);
  printf("mode                                : %012u\n"     , mode);
  printf("period h=1 extraction               : %012.6f ns\n", (double)TH1Ext/1000000000.0);
  printf("period h=1 injection                : %012.6f ns\n", (double)TH1Inj/1000000000.0);
  printf("harmonic number extr.               : %012d\n"     , nHExt);
  printf("harmonic number inj.                : %012d\n"     , nHInj);
  printf("period of beating                   : %012.6f us\n", (double)TBeat/1000000000000.0);
  printf("adjust RF-phase                     : %012.3f ns\n", cPhase);
  printf("adjust ext kicker                   : %012.3f ns\n", cTrigExt);
  printf("adjust inj kicker                   : %012.3f ns\n", cTrigInj);
} // b2b_printDiags


uint32_t b2b_info_read(uint64_t ebDevice, uint32_t *sid, uint32_t *gid, uint32_t *mode, uint64_t *TH1Ext, uint32_t *nHExt, uint64_t *TH1Inj, uint32_t *nHInj, uint64_t *TBeat, double *cPhase, double*cTrigExt, double *cTrigInj, int printFlag)
{
  eb_cycle_t   eb_cycle;
  eb_status_t  eb_status;
  eb_device_t  eb_device;
  eb_data_t    data[30];

  fdat_t tmp;
  float fCPhase;
  float fCTrigExt;
  float fCTrigInj;

  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;

  if ((eb_status = eb_cycle_open(eb_device, 0, eb_block, &eb_cycle)) != EB_OK) return COMMON_STATUS_EB;
  eb_cycle_read(eb_cycle, b2b_get_gid,           EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(eb_cycle, b2b_get_sid,           EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(eb_cycle, b2b_get_mode,          EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(eb_cycle, b2b_get_TH1ExtHi,      EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(eb_cycle, b2b_get_TH1ExtLo,      EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(eb_cycle, b2b_get_nHExt,         EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(eb_cycle, b2b_get_TH1InjHi,      EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(eb_cycle, b2b_get_TH1InjLo,      EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(eb_cycle, b2b_get_nHInj,         EB_BIG_ENDIAN|EB_DATA32, &(data[8]));
  eb_cycle_read(eb_cycle, b2b_get_TBeatHi,       EB_BIG_ENDIAN|EB_DATA32, &(data[9]));
  eb_cycle_read(eb_cycle, b2b_get_TBeatLo,       EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(eb_cycle, b2b_get_cPhase,        EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  eb_cycle_read(eb_cycle, b2b_get_cTrigExt,      EB_BIG_ENDIAN|EB_DATA32, &(data[12]));
  eb_cycle_read(eb_cycle, b2b_get_cTrigInj,      EB_BIG_ENDIAN|EB_DATA32, &(data[13]));
  if ((eb_status = eb_cycle_close(eb_cycle)) != EB_OK) return COMMON_STATUS_EB;

  *gid           = data[0];
  *sid           = data[1];
  *mode          = data[2];
  *TH1Ext        = (uint64_t)(data[3]) << 32;
  *TH1Ext       += data[4];
  *nHExt         = data[5];
  *TH1Inj        = (uint64_t)(data[6]) << 32;
  *TH1Inj       += data[7];
  *nHInj         = data[8];
  *TBeat         = (uint64_t)(data[9]) << 32;
  *TBeat        += data[10];
  tmp.data       = data[11];            // copy four bytes
  *cPhase        = (double)(tmp.f);    
  fCPhase        = tmp.f;
  tmp.data       = data[12];            // see above ...
  *cTrigExt      = (double)(tmp.f);
  fCTrigExt      = tmp.f;
  tmp.data       = data[13];            // see above ...
  *cTrigInj      = (double)(tmp.f);
  fCTrigInj      = tmp.f;

  if (printFlag) b2b_printDiag(*sid, *gid, *mode, *TH1Ext, *nHExt, *TH1Inj, *nHInj, *TBeat, fCPhase, fCTrigExt, fCTrigInj);
  
  return COMMON_STATUS_OK;
} // b2b_info_read


uint32_t b2b_common_read(uint64_t ebDevice, uint64_t *statusArray, uint32_t *state, uint32_t *nBadStatus, uint32_t *nBadState, uint32_t *version, uint32_t *nTransfer, int printDiag)
{
  eb_status_t eb_status;
  eb_device_t eb_device;

  uint64_t    dummy64a, dummy64b, dummy64c;
  uint32_t    dummy32a, dummy32c, dummy32d, dummy32e, dummy32f, dummy32g, dummy32h;

  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;

  if ((eb_status = comlib_readDiag(eb_device, statusArray, state, version, &dummy64a, &dummy32a, nBadStatus, nBadState, &dummy64b, &dummy64c, nTransfer, &dummy32c,
                                   &dummy32d, &dummy32e, &dummy32f, &dummy32g, &dummy32h, printDiag)) != COMMON_STATUS_OK) return COMMON_STATUS_EB;

  return COMMON_STATUS_OK;
} // b2b_status_read
  

uint32_t b2b_context_ext_upload(uint64_t ebDevice, uint32_t sid, uint32_t gid, uint32_t mode, double nueH1, uint32_t fNueConv, uint32_t nH, 
                                double  cTrig, int32_t nBucket, double  cPhase, uint32_t  fFineTune, uint32_t fMBTune)
{
  eb_cycle_t   eb_cycle;     // eb cycle
  eb_status_t  eb_status;    // eb status
  uint32_t     gidExt;       // b2b group ID
  uint64_t     TH1;          // revolution period [as]
  char         buff[1024];

  fdat_t tmp;

  /* hack: if fine tune is enabled, always enable multi-beat tune, chk */
  if (fFineTune) fMBTune = 1;   
  
  sprintf(buff, "ext_upload: sid %u, gid %u, mode %u, fineTune %u, mbTune %u, cTrig %f", sid, gid, mode, fFineTune, fMBTune, cTrig);
  // b2b_log("ext upload start");
  
  if (!ebDevice) return COMMON_STATUS_EB;

  // convert GID of extraction ring to GID of B2B transfer (assuming simple extraction)
  switch(gid) {
    case SIS18_RING   :
      gidExt = SIS18_B2B_EXTRACT;
      break;
    case ESR_RING     :
      gidExt = ESR_B2B_EXTRACT;
      break;
    case CRYRING_RING :
      gidExt = CRYRING_B2B_EXTRACT;
      break;
    default           :
      gidExt = GID_INVALID;
      break;
  } // switch gid
  if (gidExt == GID_INVALID) return COMMON_STATUS_OUTOFRANGE;

  /* chk more range checking */

  // convert frequency [Hz] to period [as]
  if (fNueConv) TH1 = (double)1000000000000000000.0 / b2b_flsa2fdds(nueH1);
  else          TH1 = (double)1000000000000000000.0 / nueH1;

  // EB cycle
  if (eb_cycle_open(ebDevice, 0, eb_block, &eb_cycle) != EB_OK) return COMMON_STATUS_EB;
  eb_cycle_write(eb_cycle, b2b_set_sidExt,        EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)sid);
  eb_cycle_write(eb_cycle, b2b_set_gidExt,        EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)gidExt);
  eb_cycle_write(eb_cycle, b2b_set_mode,          EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)mode);
  eb_cycle_write(eb_cycle, b2b_set_TH1ExtHi,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1 >> 32));
  eb_cycle_write(eb_cycle, b2b_set_TH1ExtLo,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1 & 0xffffffff));
  eb_cycle_write(eb_cycle, b2b_set_nHExt,         EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)nH);
  tmp.f = (float)cTrig;
  eb_cycle_write(eb_cycle, b2b_set_cTrigExt,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(tmp.data));
  eb_cycle_write(eb_cycle, b2b_set_nBuckExt,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)((uint32_t)nBucket));
  tmp.f = (float)cPhase;
  eb_cycle_write(eb_cycle, b2b_set_cPhase,        EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(tmp.data));
  eb_cycle_write(eb_cycle, b2b_set_fFinTune,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)fFineTune);
  eb_cycle_write(eb_cycle, b2b_set_fMBTune,       EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)fMBTune);
  if ((eb_status = eb_cycle_close(eb_cycle)) != EB_OK) return COMMON_STATUS_EB;

  b2b_log(buff);
  
  return COMMON_STATUS_OK;
} // b2b_context_ext_upload


uint32_t b2b_context_inj_upload(uint64_t ebDevice, uint32_t sidExt, uint32_t gid, uint32_t sid, uint32_t bpid, uint64_t param, double nueH1, uint32_t fNueConv, uint32_t nH, double cTrig, int32_t nBucket)
{
  eb_cycle_t   eb_cycle;     // eb cycle
  eb_status_t  eb_status;    // eb status
  uint32_t     gidInj;       // b2b group ID
  uint64_t     TH1;          // revolution period [as]
  uint32_t     paramHi;
  uint32_t     paramLo;
  char         buff[1024];

  fdat_t       tmp;

  //b2b_log("inj_upload start");
  
  if (!ebDevice) return COMMON_STATUS_EB;

  // convert GID of injection ring to GID 'offset' of B2B transfer (assuming transfer ring-ring)
  switch(gid) {
    case ESR_RING     :
      gidInj = SIS18_B2B_ESR - SIS18_B2B_EXTRACT;
      break;
    case CRYRING_RING :
      gidInj = ESR_B2B_CRYRING - ESR_B2B_EXTRACT;
      break;
    default           :
      gidInj = GID_INVALID;
      break;
  } // switch gid
  if (gidInj == GID_INVALID) return COMMON_STATUS_OUTOFRANGE;

  /* chk more range checking */

  // convert frequency [Hz] to period [as]
  if (fNueConv) TH1 = (double)1000000000000000000.0 / b2b_flsa2fdds(nueH1);
  else          TH1 = (double)1000000000000000000.0 / nueH1;

  // parameter field
   paramHi = (uint32_t)((param >> 32) & 0xffffffff);
   paramLo = (uint32_t)( param & 0xfffffff);
  // hack: swap high/loword
  /*paramHi = (uint32_t)( param & 0xfffffff);
    paramLo = (uint32_t)((param >> 32) & 0xffffffff);*/

  // EB cycle
  if (eb_cycle_open(ebDevice, 0, eb_block, &eb_cycle) != EB_OK) return COMMON_STATUS_EB;
  eb_cycle_write(eb_cycle, b2b_set_sidEInj,       EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)sidExt);               // this looks funny but writing sidExt to the sidEInj register is not a bug
  eb_cycle_write(eb_cycle, b2b_set_gidInj,        EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)gidInj);
  eb_cycle_write(eb_cycle, b2b_set_lsidInj,       EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)sid);
  eb_cycle_write(eb_cycle, b2b_set_lbpidInj,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)bpid);
  eb_cycle_write(eb_cycle, b2b_set_lparamInjHi,   EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)paramHi);
  eb_cycle_write(eb_cycle, b2b_set_lparamInjLo,   EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)paramLo);
  eb_cycle_write(eb_cycle, b2b_set_TH1InjHi,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1 >> 32));
  eb_cycle_write(eb_cycle, b2b_set_TH1InjLo,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1 & 0xffffffff));
  eb_cycle_write(eb_cycle, b2b_set_nHInj,         EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)nH);
  tmp.f = (float)cTrig;
  eb_cycle_write(eb_cycle, b2b_set_cTrigInj,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(tmp.data));
  eb_cycle_write(eb_cycle, b2b_set_nBuckInj,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)((uint32_t)nBucket));
  if ((eb_status = eb_cycle_close(eb_cycle)) != EB_OK) return COMMON_STATUS_EB;

  sprintf(buff, "inj_upload: sidExt %u, gid %u, sid %u, bpid %u, paramHi %u, paramLo %u, param 0x%lx", sidExt, gid, sid, bpid, paramHi, paramLo, param);
  b2b_log(buff);
  
  return COMMON_STATUS_OK;
} // b2b_context_inj_upload


void b2b_cmd_configure(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CONFIGURE, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_configure


void b2b_cmd_startop(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STARTOP, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_startop


void b2b_cmd_stopop(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STOPOP, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_stopop


void b2b_cmd_recover(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_RECOVER, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_recover


void b2b_cmd_idle(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_IDLE, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_idle


void b2b_cmd_cleardiag(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CLEARDIAG, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_cleardiag


void b2b_cmd_submit(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)B2B_CMD_CONFSUBMIT, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
  b2b_log("cmd_submit done");

} // b2b_cmd_submit


void b2b_cmd_clearConfig(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)B2B_CMD_CONFCLEAR, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_clearConfig
