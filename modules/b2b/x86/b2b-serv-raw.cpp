/*******************************************************************************************
 *  b2b-serv-raw.cpp
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 14-feb-2025
 *
 * publishes raw data of the b2b system
 *
 * ------------------------------------------------------------------------------------------
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
 * Last update: 15-April-2019
 *********************************************************************************************/
#define B2B_SERV_RAW_VERSION 0x000808

#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS

// dim includes
#include <dis.h>
#include <dis.hxx>

// standard includes
#include <iostream>
#include <iomanip>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <math.h>

// saftlib includes
#include "SAFTd.h"
#include "TimingReceiver.h"
//#include "SoftwareActionSink.h"
//#include "SoftwareCondition.h"
#include "EmbeddedCPUActionSink.h"
#include "EmbeddedCPUCondition.h"
#include "iDevice.h"
#include "iOwned.h"
#include "CommonFunctions.h"

// b2b includes
#include <common-lib.h>                 // COMMON
#include <b2blib.h>                     // API
#include <b2b.h>                        // FW

using namespace saftlib;
using namespace std;

#define FID          0x1                // format ID of timing messages

static const char* program;

// dim
#define DIMCHARSIZE 32                  // standard size for char services
#define DIMMAXSIZE  1024                // max size for service names
#define NAMELEN     256                 // max size for names

double    no_link_dbl   = NAN;          // indicates "no link" for missing DIM services of type double

// services published
char      disVersion[DIMCHARSIZE];
char      disState[DIMCHARSIZE];
char      disHostname[DIMCHARSIZE];
uint64_t  disStatus;
uint32_t  disNTransfer;
setval_t  disSetval[B2B_NSID];
getval_t  disGetval[B2B_NSID];

uint32_t  disVersionId      = 0;
uint32_t  disStateId        = 0;
uint32_t  disHostnameId     = 0;
uint32_t  disStatusId       = 0;
uint32_t  disNTransferId    = 0;
uint32_t  disSetvalId[B2B_NSID];
uint32_t  disGetvalId[B2B_NSID];

// services subscribed
// 
// aehem ...
// its a bit difficult to organize all combinations of injection machines; moreover, there are always new requirements
// let's use the brute force approach and subscribe to kicker information of all ring machines at GSI
double    dicKickLevelSIS18Ext[B2B_NSID];
double    dicKickLevelESRInj[B2B_NSID];
double    dicKickLevelESRExt[B2B_NSID];
double    dicKickLevelYRInj[B2B_NSID];
double    dicKickLevelYRExt[B2B_NSID];
double    dicKickLevelSIS100Inj[B2B_NSID];
double    dicKickLevelSIS100Ext[B2B_NSID];
double    dicKickLenSIS18Ext[B2B_NSID];
double    dicKickLenESRInj[B2B_NSID];
double    dicKickLenESRExt[B2B_NSID];
double    dicKickLenYRInj[B2B_NSID];
double    dicKickLenYRExt[B2B_NSID];
double    dicKickLenSIS100Inj[B2B_NSID];
double    dicKickLenSIS100Ext[B2B_NSID];

uint32_t  dicKickLevelSIS18ExtId[B2B_NSID];
uint32_t  dicKickLevelESRInjId[B2B_NSID];
uint32_t  dicKickLevelESRExtId[B2B_NSID];
uint32_t  dicKickLevelYRInjId[B2B_NSID];
uint32_t  dicKickLevelYRExtId[B2B_NSID];
uint32_t  dicKickLevelSIS100InjId[B2B_NSID];
uint32_t  dicKickLevelSIS100ExtId[B2B_NSID];
uint32_t  dicKickLenSIS18ExtId[B2B_NSID];
uint32_t  dicKickLenESRInjId[B2B_NSID];
uint32_t  dicKickLenESRExtId[B2B_NSID];
uint32_t  dicKickLenYRInjId[B2B_NSID];
uint32_t  dicKickLenYRExtId[B2B_NSID];
uint32_t  dicKickLenSIS100InjId[B2B_NSID];
uint32_t  dicKickLenSIS100ExtId[B2B_NSID];

// local variables
uint32_t reqExtRing;                    // requested extraction ring

uint32_t sid;                           // Sequence ID
uint32_t bpid;                          // Beam Process ID


// init setval
void initSetval(setval_t *setval)
{
  setval->mode                  = -1;
  setval->ext_T                 = -1;
  setval->ext_h                 = -1;
  setval->ext_cTrig             = NAN;
  setval->ext_sid               = -1;
  setval->ext_gid               = -1;
  setval->inj_T                 = -1;
  setval->inj_h                 = -1;
  setval->inj_cTrig             = NAN;
  setval->inj_sid               = -1;
  setval->inj_gid               = -1;
  setval->cPhase                = NAN;
} // initSetval

// init getval
void initGetval(getval_t *getval)
{
  getval->ext_phase             = -1;
  getval->ext_phaseFract        = NAN;
  getval->ext_phaseErr          = NAN;
  getval->ext_phaseSysmaxErr    = NAN;
  getval->ext_dKickMon          = NAN;
  getval->ext_dKickProb         = NAN;
  getval->ext_dKickProbLen      = NAN;
  getval->ext_dKickProbLevel    = NAN;
  getval->ext_diagPhase         = NAN;
  getval->ext_diagMatch         = NAN;
  getval->ext_phaseShift        = NAN;
  getval->inj_phase             = -1;
  getval->inj_phaseFract        = NAN;
  getval->inj_phaseErr          = NAN;
  getval->inj_phaseSysmaxErr    = NAN;
  getval->inj_dKickMon          = NAN;
  getval->inj_dKickProb         = NAN;
  getval->inj_dKickProbLen      = NAN;
  getval->inj_dKickProbLevel    = NAN;
  getval->inj_diagPhase         = NAN;
  getval->inj_diagMatch         = NAN;
  getval->inj_phaseShift        = NAN;
  getval->flagEvtRec            = 0;
  getval->flagEvtErr            = 0;
  getval->flagEvtLate           = 0;
  getval->tCBS                  = -1;
  getval->finOff                = NAN;
  getval->prrOff                = NAN;
  getval->preOff                = NAN;
  getval->priOff                = NAN;
  getval->kteOff                = NAN;
  getval->ktiOff                = NAN;
} // initGetval


// update set value
void disUpdateSetval(uint32_t sid, uint64_t tStart, setval_t setval)
{
  uint32_t secs;
  uint32_t msecs;
  
  b2b_t2secs(tStart, &secs, &msecs);
  msecs  /= 1000000;
  
  disSetval[sid] = setval;
  dis_set_timestamp(disSetvalId[sid], secs, msecs);
  dis_update_service(disSetvalId[sid]);
  
  disNTransfer++;
  dis_update_service(disNTransferId);
} // disUpdateSetval
 

// update get value
 void disUpdateGetval(uint32_t sid, uint64_t tStart, getval_t getval)
{
  uint32_t secs;
  uint32_t msecs;

  b2b_t2secs(tStart, &secs, &msecs);
  msecs  /= 1000000;
  
  disGetval[sid] = getval;
  dis_set_timestamp(disGetvalId[sid], secs, msecs);
  dis_update_service(disGetvalId[sid]);
} // disUpdateGetval


// handle received timing message
static void timingMessage(uint32_t tag, saftlib::Time deadline, uint64_t evtId, uint64_t param, uint32_t tef, uint32_t isLate, uint32_t isEarly, uint32_t isConflict, uint32_t isDelayed)
{
  uint32_t            recSid;          // received SID
  uint32_t            recGid;          // receiver GID
  int                 flagErr;

  static int          flagActive;      // flag: b2b is active
  static setval_t     setval;          // set values
  static getval_t     getval;          // get values
  static uint64_t     tStart;          // time of transfer

  uint64_t one_ns_as = 1000000000;
  fdat_t   tmp;
  float    tmpf;
  uint32_t tmpu;

  recSid      = ((evtId  & 0x00000000fff00000) >> 20);
  recGid      = ((evtId  & 0x0fff000000000000) >> 48);

  // check ranges
  if (recSid  > B2B_NSID)                 return;
  if (tag > tagStop)                      return;
  if ((!flagActive) && (tag != tagStart)) return;
  //printf("tag %d\n", tag);
  // mark message as received
  getval.flagEvtRec  |= 0x1 << tag;
  getval.flagEvtLate |= isLate << tag;
  
  switch (tag) {
    case tagStart   :
      sid                          = recSid;
      tStart                       = deadline.getUTC();
      flagActive                   = 1;
      
      initSetval(&setval);
      setval.mode                  = B2B_MODE_OFF;    // in the simplest case
      setval.ext_sid               = sid;
      setval.ext_gid               = recGid;

      initGetval(&getval);
      getval.flagEvtRec            = 0x1 << tag;
      getval.flagEvtLate           = isLate << tag;
      getval.tCBS                  = deadline.getTAI();
      break;
    case tagStop    :
      flagActive       = 0;
      disUpdateSetval(sid, tStart, setval);
      disUpdateGetval(sid, tStart, getval);      
      break;
    case tagPme     :
      setval.mode              = ((param & 0x00f0000000000000) >> 52);
      switch(recGid) {
        case SIS18_B2B_ESR    : setval.inj_gid = ESR_RING;     break;
        case SIS18_B2B_SIS100 : setval.inj_gid = SIS100_RING;  break;
        case ESR_B2B_CRYRING  : setval.inj_gid = CRYRING_RING; break;
        default               : setval.inj_gid = -1;           break;
      } // switch gid
      
      setval.ext_h             = ((param & 0xff00000000000000) >> 56);
      setval.ext_T             = ((param & 0x000fffffffffffff));    // [as]
      if (setval.mode > B2B_MODE_OFF) {
        tmpf                   = comlib_half2float((uint16_t)((tef & 0xffff0000)  >> 16)); // [us, hfloat]; chk for NAN?
        setval.ext_cTrig       = tmpf * 1000.0;                     // [ns]
      } // if mode
      if ((setval.mode >  B2B_MODE_B2E) && (setval.mode != B2B_MODE_B2C)) {
        tmpf                   = comlib_half2float((uint16_t)( tef & 0x0000ffff));         // [us, hfloat]; chk for NAN?
        setval.cPhase          = tmpf  * 1000;                      // [ns]
      } // if mode
      break;
    case tagPmi     :
      setval.inj_h             = ((param & 0xff00000000000000) >> 56);
      setval.inj_T             = ((param & 0x000fffffffffffff));    // [as]
      if (setval.mode > B2B_MODE_B2E) {
        tmpf                   = comlib_half2float((uint16_t)((tef & 0xffff0000) >> 16));              // [us, hfloat]]
        setval.inj_cTrig       = tmpf * 1000.0;                     // [ns]
      } // if mode
      break;
    case tagPre     :
      getval.preOff             = (float)(param - getval.tCBS);
      getval.ext_phase          = param;
      getval.ext_phaseFract     = (float)( tef & 0x0000ffff) / 1000.0 ;
      getval.ext_phaseErr       = (float)((tef & 0xffff0000) >> 16) / 1000.0;
      getval.ext_phaseSysmaxErr = (float)b2b_calc_max_sysdev_ps(setval.ext_T, B2B_NSAMPLES, 0) / 1000.0;
      flagErr                   = ((evtId & B2B_ERRFLAG_PMEXT) != 0);
      getval.flagEvtErr        |= flagErr << tag;
      break;
    case tagPri     :
      getval.priOff             = (float)(param - getval.tCBS);
      getval.inj_phase          = param;
      getval.inj_phaseFract     = (float)( tef & 0x0000ffff) / 1000.0;
      getval.inj_phaseErr       = (float)((tef & 0xffff0000) >> 16) / 1000.0;
      getval.inj_phaseSysmaxErr = (float)b2b_calc_max_sysdev_ps(setval.inj_T, B2B_NSAMPLES, 0) / 1000.0;
      flagErr                   = ((evtId & B2B_ERRFLAG_PMINJ) != 0);
      getval.flagEvtErr        |= flagErr << tag;
      break;
    case tagKte     :
      getval.kteOff            = deadline.getTAI() - getval.tCBS;
      tmpf                     = comlib_half2float((uint16_t)((tef & 0xffff0000) >> 16));        // [us, hfloat]
      getval.finOff            = tmpf * 1000.0;
      tmpf                     = comlib_half2float((uint16_t)(tef & 0x0000ffff));                // [us, hfloat]
      getval.prrOff            = tmpf * 1000.0;
      flagErr                  = ((evtId & B2B_ERRFLAG_CBU) != 0);
      getval.flagEvtErr       |= flagErr << tag;
      break;
    case tagKti     :
      setval.inj_sid           = recSid;;
      getval.ktiOff            = deadline.getTAI() - getval.tCBS;
      flagErr                  = ((evtId    & 0x0000000000000010) >> 4);
      getval.flagEvtErr       |= flagErr << tag;
      break;
    case tagKde     :          // data types within parameter field are 'int' as this a. should be easy for the users b. granularity is 1ns only
      tmpu                     = (uint32_t)(param & 0x00000000ffffffff);
      if (tmpu == 0x7fffffff) getval.ext_dKickProb = NAN;
      else                    getval.ext_dKickProb = tmpu;
      tmpu                     = (uint32_t)((param & 0xffffffff00000000) >> 32);
      if (tmpu == 0x7fffffff) getval.ext_dKickMon  = NAN;
      else                    getval.ext_dKickMon  = tmpu;
      flagErr                  = ((evtId & B2B_ERRFLAG_KDEXT) != 0);
      getval.flagEvtErr       |= flagErr << tag;

      switch(setval.ext_gid) {
        case SIS18_RING   : getval.ext_dKickProbLen = dicKickLenSIS18Ext[setval.ext_sid]; getval.ext_dKickProbLevel = dicKickLevelSIS18Ext[setval.ext_sid]; break;
        case ESR_RING     : getval.ext_dKickProbLen = dicKickLenESRExt[setval.ext_sid];   getval.ext_dKickProbLevel = dicKickLevelESRExt[setval.ext_sid];   break;
        case CRYRING_RING : getval.ext_dKickProbLen = dicKickLenYRExt[setval.ext_sid];    getval.ext_dKickProbLevel = dicKickLevelYRExt[setval.ext_sid];    break;
        default           : getval.ext_dKickProbLen = -1;                                 getval.ext_dKickProbLevel = -1;                                   break;
      } // switch setval ext_gid
      
      break;
    case tagKdi     :          // data types within parameter field are 'int' as this a. should be easy for the users b. granularity is 1ns only
      tmpu                     = (uint32_t)(param & 0x00000000ffffffff);
      if (tmpu == 0x7fffffff) getval.inj_dKickProb = NAN;
      else                    getval.inj_dKickProb = tmpu;
      tmpu                     = (uint32_t)((param & 0xffffffff00000000) >> 32);
      if (tmpu == 0x7fffffff) getval.inj_dKickMon  = NAN;
      else                    getval.inj_dKickMon  = tmpu;
      flagErr                  = ((evtId & B2B_ERRFLAG_KDINJ) != 0);
      getval.flagEvtErr       |= flagErr << tag;

      switch(setval.inj_gid) {
        case ESR_RING     : getval.inj_dKickProbLen = dicKickLenESRInj[setval.inj_sid];   getval.inj_dKickProbLevel = dicKickLevelESRInj[setval.inj_sid];   break;
        case CRYRING_RING : getval.inj_dKickProbLen = dicKickLenYRInj[setval.inj_sid];    getval.inj_dKickProbLevel = dicKickLevelYRInj[setval.inj_sid];    break;
        default           : getval.inj_dKickProbLen = -1;                                 getval.inj_dKickProbLevel = -1;                                   break;
      } // switch setval ext_gid
      
      break;
    case tagPde     :         // chk: consider changing 0x7fffffff to NAN in b2b-pm.c after beam time 2024
      tmp.data                 = ((param & 0x00000000ffffffff));
      if (tmp.data == 0x7fffffff) getval.ext_diagMatch = NAN;
      else                        getval.ext_diagMatch = (double)tmp.f;
      tmp.data                 = ((param & 0xffffffff00000000) >> 32);
      if (tmp.data == 0x7fffffff) getval.ext_diagPhase = NAN;
      else                        getval.ext_diagPhase = (double)tmp.f;
      break;
    case tagPdi     :         // chk: consider changing 0x7fffffff to NAN in b2b-pm.c after beam time 2024
      tmp.data                 = ((param & 0x00000000ffffffff));
      if (tmp.data == 0x7fffffff) getval.inj_diagMatch = NAN;
      else                        getval.inj_diagMatch = (double)tmp.f;
      tmp.data                 = ((param & 0xffffffff00000000) >> 32);
      if (tmp.data == 0x7fffffff) getval.inj_diagPhase = NAN;
      else                        getval.inj_diagPhase = (double)tmp.f;
      break;
    case tagPse     :
      tmpu                     = ((param & 0x00000000ffffffff));
      // revert endianess hack
      tmp.data                 = ((tmpu & 0x0000ffff) << 16);
      tmp.data                |= ((tmpu & 0xffff0000) >> 16);
      if (!isnan(tmp.f)) tmp.f = tmp.f / 360.0 * (setval.ext_T / (float)one_ns_as);
      getval.ext_phaseShift    = tmp.f;
      break;
    case tagPsi     :
      tmpu                     = ((param & 0x00000000ffffffff));
      // revert endianess hack
      tmp.data                 = ((tmpu & 0x0000ffff) << 16);
      tmp.data                |= ((tmpu & 0xffff0000) >> 16);
      if (!isnan(tmp.f)) tmp.f = tmp.f / 360.0 * (setval.inj_T / (float)one_ns_as);
      getval.inj_phaseShift    = tmp.f;
      break;
    default         :
      ;
  } // switch tag
  
  //printf("out tag %d, bpid %d\n", tag, bpid);
} // timingmessage


// this will be called when receiving ECA actions from software action queue
// informative: this routine is presently not used, as the softare action queue does not support the TEF field
/*static void recTimingMessage(uint64_t id, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags, uint32_t tag)
{
  int                 flagLate;

  flagLate    = flags & 0x1;

  timingMessage(tag, deadline, id, param, 0x0, flagLate, 0, 0, 0);
} // recTimingMessag*/


// call back for command
class RecvCommand : public DimCommand
{
  int  reset;
  void commandHandler() {disNTransfer = 0;}
public :
  RecvCommand(const char *name) : DimCommand(name,"C"){}
}; 


// add all dim services
void disAddServices(char *prefix)
{
  char name[DIMMAXSIZE];
  int  i;

  // 'generic' services
  sprintf(name, "%s-raw_version_fw", prefix);
  sprintf(disVersion, "%s",  b2b_version_text(B2B_SERV_RAW_VERSION));
  disVersionId   = dis_add_service(name, "C", disVersion, 8, 0 , 0);

  sprintf(name, "%s-raw_state", prefix);
  sprintf(disState, "%s", b2b_state_text(COMMON_STATE_OPREADY));
  disStateId      = dis_add_service(name, "C", disState, 10, 0 , 0);

  sprintf(name, "%s-raw_hostname", prefix);
  disHostnameId   = dis_add_service(name, "C", disHostname, DIMCHARSIZE, 0 , 0);

  sprintf(name, "%s-raw_status", prefix);
  disStatus       = 0x1;   
  disStatusId     = dis_add_service(name, "X", &disStatus, sizeof(disStatus), 0 , 0);

  sprintf(name, "%s-raw_ntransfer", prefix);
  disNTransferId  = dis_add_service(name, "I", &disNTransfer, sizeof(disNTransfer), 0 , 0);

  // set values
  for (i=0; i< B2B_NSID; i++) {
    sprintf(name, "%s-raw_sid%02d_setval", prefix, i);
    disSetvalId[i]  = dis_add_service(name, "I:1;X:1;I:1;F:1;I:2;X:1;I:1;F:1;I:2;F:1", &(disSetval[i]), sizeof(setval_t), 0, 0);
    dis_set_timestamp(disSetvalId[i], 1, 0);
  } // for i

  // set values
  for (i=0; i< B2B_NSID; i++) {
    sprintf(name, "%s-raw_sid%02d_getval", prefix, i);
    disGetvalId[i]  = dis_add_service(name, "X:1;F:10;X:1;F:10;I:3;X:1;F:6", &(disGetval[i]), sizeof(getval_t), 0, 0);
    dis_set_timestamp(disGetvalId[i], 1, 0);
  } // for i
} // disAddServices


// add all dim services
void dicSubscribeServices(char *prefix)
{
  char name[DIMMAXSIZE];
  int  i;

  for (i=0; i<B2B_NSID; i++) {
    sprintf(name, "%s_sis18-kdde_sid%02d_len",    prefix, i);
    //printf("prefix %s, name %s\n", prefix, name);
    dicKickLenSIS18ExtId[i] = dic_info_service_stamped(   name, MONITORED, 0, &(dicKickLenSIS18Ext[i]),    sizeof(double), 0 , 0, &no_link_dbl, sizeof(double));
    sprintf(name, "%s_esr-kdde_sid%02d_len",      prefix, i);
    dicKickLenESRExtId[i] = dic_info_service_stamped(     name, MONITORED, 0, &(dicKickLenESRExt[i]),      sizeof(double), 0 , 0, &no_link_dbl, sizeof(double));
    sprintf(name, "%s_esr-kddi_sid%02d_len",      prefix, i);
    dicKickLenESRInjId[i] = dic_info_service_stamped(     name, MONITORED, 0, &(dicKickLenESRInj[i]),      sizeof(double), 0 , 0, &no_link_dbl, sizeof(double));
    sprintf(name, "%s_yr-kdde_sid%02d_len",       prefix, i);
    dicKickLenYRExtId[i] = dic_info_service_stamped(      name, MONITORED, 0, &(dicKickLenYRExt[i]),       sizeof(double), 0 , 0, &no_link_dbl, sizeof(double));
    sprintf(name, "%s_yr-kddi_sid%02d_len",       prefix, i);
    dicKickLenYRInjId[i] = dic_info_service_stamped(      name, MONITORED, 0, &(dicKickLenYRInj[i]),       sizeof(double), 0 , 0, &no_link_dbl, sizeof(double));
    sprintf(name, "%s_sis100-kdde_sid%02d_len",   prefix, i);
    dicKickLenSIS100ExtId[i] = dic_info_service_stamped(  name, MONITORED, 0, &(dicKickLenSIS100Ext[i]),   sizeof(double), 0 , 0, &no_link_dbl, sizeof(double));
    sprintf(name, "%s_sis100-kddi_sid%02d_len",   prefix, i);
    dicKickLenSIS100InjId[i] = dic_info_service_stamped(  name, MONITORED, 0, &(dicKickLenSIS100Inj[i]),   sizeof(double), 0 , 0, &no_link_dbl, sizeof(double));

    sprintf(name, "%s_sis18-kdde_sid%02d_level",  prefix, i);
    dicKickLevelSIS18ExtId[i] = dic_info_service_stamped( name, MONITORED, 0, &(dicKickLevelSIS18Ext[i]),  sizeof(double), 0 , 0, &no_link_dbl, sizeof(double));
    sprintf(name, "%s_esr-kdde_sid%02d_level",    prefix, i);
    dicKickLevelESRExtId[i] = dic_info_service_stamped(   name, MONITORED, 0, &(dicKickLevelESRExt[i]),    sizeof(double), 0 , 0, &no_link_dbl, sizeof(double));
    sprintf(name, "%s_esr-kddi_sid%02d_level",    prefix, i);
    dicKickLevelESRInjId[i] = dic_info_service_stamped(   name, MONITORED, 0, &(dicKickLevelESRInj[i]),    sizeof(double), 0 , 0, &no_link_dbl, sizeof(double));
    sprintf(name, "%s_yr-kdde_sid%02d_level",     prefix, i);
    dicKickLevelYRExtId[i] = dic_info_service_stamped(    name, MONITORED, 0, &(dicKickLevelYRExt[i]),     sizeof(double), 0 , 0, &no_link_dbl, sizeof(double));
    sprintf(name, "%s_yr-kddi_sid%02d_level",     prefix, i);
    dicKickLevelYRInjId[i] = dic_info_service_stamped(    name, MONITORED, 0, &(dicKickLevelYRInj[i]),     sizeof(double), 0 , 0, &no_link_dbl, sizeof(double));
    sprintf(name, "%s_sis100-kdde_sid%02d_level", prefix, i);
    dicKickLevelSIS100ExtId[i] = dic_info_service_stamped(name, MONITORED, 0, &(dicKickLevelSIS100Ext[i]), sizeof(double), 0 , 0, &no_link_dbl, sizeof(double));
    sprintf(name, "%s_sis100-kddi_sid%02d_level", prefix, i);
    dicKickLevelSIS100InjId[i] = dic_info_service_stamped(name, MONITORED, 0, &(dicKickLevelSIS100Inj[i]), sizeof(double), 0 , 0, &no_link_dbl, sizeof(double));
  } // for i
} // dicSubscribeServices

                        
//using namespace saftlib;
//using namespace std;

// display help
static void help(void) {
  std::cerr << std::endl << "Usage: " << program << " <device name> [OPTIONS] <server name prefix>" << std::endl;
  std::cerr << std::endl;
  std::cerr << "  -e<index>            specify extraction ring (0: SIS18[default], 1: ESR, 2: CRYRING)" << std::endl;
  std::cerr << "  -h                   display this help and exit" << std::endl;
  std::cerr << "  -f                   use the first attached device (and ignore <device name>)" << std::endl;
  std::cerr << std::endl;
  std::cerr << std::endl;
  std::cerr << "This tool provides a server for raw b2b data." << std::endl;
  std::cerr << std::endl;
  std::cerr << "Important notice: This program uses the ECA action queue of an lm32(!). Only one instance of this" << std::endl;
  std::cerr << "programm shall be used. Other programs (from host or lm32) must not access that action  queue. " << std::endl;
  std::cerr << std::endl;
  std::cerr << "Example1: '" << program << " tr0 -e0 pro'" << std::endl;
  std::cerr << std::endl;

  std::cerr << "Report bugs to <d.beck@gsi.de> !!!" << std::endl;
  std::cerr << "Version " << b2b_version_text(B2B_SERV_RAW_VERSION) << ". Licensed under the GPL v3." << std::endl;
} // help

int main(int argc, char** argv)
{
  // variables and flags for command line parsing
  int  opt;
  bool useFirstDev    = false;
  char *tail;


  // variables snoop event
  uint64_t snoopID     = 0x0;
  int      nCondition  = 0;

  char tmp[752];
  int i;

  // variables attach, remove
  char    *deviceName = NULL;
  char    *envName    = NULL;

  char     ringName[NAMELEN];
  char     prefix[NAMELEN*2];
  char     kickerPrefix[NAMELEN*2];
  char     disName[DIMMAXSIZE];


  reqExtRing  = SIS18_RING;


  // parse for options
  program = argv[0];
  while ((opt = getopt(argc, argv, "e:hf")) != -1) {
    switch (opt) {
      case 'e' :
        switch (strtol(optarg, &tail, 0)) {
          case 0 : reqExtRing = SIS18_RING;   break;
          case 1 : reqExtRing = ESR_RING;     break;
          case 2 : reqExtRing = CRYRING_RING; break;
          default:
            std::cerr << "option -e: parameter out of range" << std::endl;
            return 1;
        } // switch optarg
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          return 1;
        } // if *tail
        break;
      case 'f' :
        useFirstDev = true;
        break;
      case 'h':
        help();
        return 0;
      default:
        std::cerr << program << ": bad getopt result" << std::endl;
        return 1;
    } // switch opt
  }   // while opt

  if (optind >= argc) {
    std::cerr << program << " expecting one non-optional arguments: <device name>" << std::endl;
    help();
    return 1;
  }

  // no parameters, no command: just display help and exit
  if ((optind == 1) && (argc == 1)) {
    help();
    return 0;
  } // if optind

  if (!(optind+1 < argc)) return 1;
  
  deviceName = argv[optind];
  envName    = argv[optind+1];
  gethostname(disHostname, 32);

  switch(reqExtRing) {
    case SIS18_RING :
      nCondition = 18;
      sprintf(ringName, "sis18");
      break;
    case ESR_RING :
      nCondition = 18;
      sprintf(ringName, "esr");
      break;
    case CRYRING_RING :
      nCondition = 7;
      sprintf(ringName, "yr");
      break;
    default :
        std::cerr << "Ring '"<< reqExtRing << "' does not exist" << std::endl;
        return -1;;
  } // switch extRing
  
  // init DIM service data
  for (i=0; i< B2B_NSID; i++ ) {
    initSetval(&(disSetval[i]));
    initGetval(&(disGetval[i]));
  } // for i
  
  // create service and start server
  sprintf(prefix, "b2b_%s_%s", envName, ringName);

  printf("%s: starting server using prefix %s\n", program, prefix);

  disAddServices(prefix);
  // uuuuhhhh, mixing c++ and c  
  sprintf(tmp, "%s-raw_cmd_cleardiag", prefix);
  RecvCommand cmdClearDiag(tmp);
  
  sprintf(disName, "%s-raw", prefix);
  dis_start_serving(disName);

  // subscribe to services at kicker diagnostic
  sprintf(kickerPrefix, "b2b_%s", envName);

  printf("%s: subscribing to services using prefix %s\n", program, kickerPrefix);  

  dicSubscribeServices(kickerPrefix);
  
  try {
    // basic saftd stuff
    std::shared_ptr<SAFTd_Proxy> saftd = SAFTd_Proxy::create();

    // connect to timing receiver
    map<std::string, std::string> devices = SAFTd_Proxy::create()->getDevices();
    std::shared_ptr<TimingReceiver_Proxy> receiver;
    if (useFirstDev) {
      receiver = TimingReceiver_Proxy::create(devices.begin()->second);
    } else {
      if (devices.find(deviceName) == devices.end()) {
        std::cerr << "Device '" << deviceName << "' does not exist" << std::endl;
        return -1;
      } // find device
      receiver = TimingReceiver_Proxy::create(devices[deviceName]);
    } //if(useFirstDevice);

    // create software action sink
    //std::shared_ptr<SoftwareActionSink_Proxy> sink = SoftwareActionSink_Proxy::create(receiver->NewSoftwareActionSink(""));
    //std::shared_ptr<SoftwareCondition_Proxy> condition[nCondition];

    // search for embedded CPU channel
     map<std::string, std::string> e_cpus = receiver->getInterfaces()["EmbeddedCPUActionSink"];
    if (e_cpus.size() != 1)
    {
      std::cerr << "Device '" << receiver->getName() << "' has no embedded CPU!" << std::endl;
      return (-1);
    }
    // connect to embedded CPU
    std::shared_ptr<EmbeddedCPUActionSink_Proxy> e_cpu = EmbeddedCPUActionSink_Proxy::create(e_cpus.begin()->second);

    // create action sink for ecpu
    std::shared_ptr<EmbeddedCPUCondition_Proxy> condition[nCondition];
    //uint32_t tag[nCondition];
    uint32_t tmpTag;

    // define conditions (ECA filter rules)
    switch (reqExtRing) {
      case SIS18_RING : 

        // SIS18, CMD_B2B_START, signals start of data collection
        tmpTag        = tagStart;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_RING << 48) | ((uint64_t)B2B_ECADO_B2B_START << 36);
        condition[0]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[0]        = tmpTag;
        
        // SIS18, CMD_B2B_START, +100ms (!), signals stop of data collection
        tmpTag        = tagStop;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_RING << 48) | ((uint64_t)B2B_ECADO_B2B_START << 36);
        condition[1]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 100000000, tmpTag));
        //tag[1]        = tmpTag;
        
        // SIS18 to extraction, PMEXT,
        tmpTag        = tagPme;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PMEXT << 36);
        condition[2]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[2]        = tmpTag;

        // SIS18 to extraction, PREXT
        tmpTag        = tagPre;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PREXT << 36);
        condition[3]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[3]        = tmpTag;

        // SIS18 to extraction, DIAGEXT
        tmpTag        = tagPde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGEXT << 36);
        condition[4]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[4]        = tmpTag;

        // SIS18 to extraction, continues further down ...

        // SIS18 to ESR, PMEXT
        tmpTag        = tagPme;       
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_PMEXT << 36);
        condition[5]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[5]        = tmpTag;

        // SIS18 to ESR, PMINJ
        tmpTag        = tagPmi;       
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_PMINJ << 36);
        condition[6]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[6]        = tmpTag;

        // SIS18 to ESR, PREXT
        tmpTag        = tagPre;       
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_PREXT << 36);
        condition[7]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[7]        = tmpTag;

        // SIS18 to ESR, PRINJ
        tmpTag        = tagPri;       
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_PRINJ << 36);
        condition[8]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[8]        = tmpTag;
   
        // SIS18 to ESR, DIAGEXT
        tmpTag        = tagPde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGEXT << 36);
        condition[9]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[9]        = tmpTag;

        // SIS18 to ESR, DIAGINJ
        tmpTag        = tagPdi;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGINJ << 36);
        condition[10] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[10]       = tmpTag;
        
        // SIS18 extraction kicker trigger
        tmpTag        = tagKte;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_RING << 48) | ((uint64_t)B2B_ECADO_B2B_TRIGGEREXT << 36);
        condition[11] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[11]       = tmpTag;
        
        // SIS18 extraction kicker diagnostic
        tmpTag        = tagKde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_RING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGKICKEXT << 36);
        condition[12] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[12]       = tmpTag;
        
        // ESR injection kicker trigger
        tmpTag        = tagKti;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_TRIGGERINJ << 36);
        condition[13] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[13]       = tmpTag;
        
        // ESR injection kicker diagnostic
        tmpTag        = tagKdi;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGKICKINJ << 36);
        condition[14] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[14]       = tmpTag;

        // SIS18 to extraction, phase shift extraction
        tmpTag        = tagPse;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PSHIFTEXT << 36);
        condition[15] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[15]        = tmpTag;

        // SIS18 to ESR, phase shift extraction
        tmpTag        = tagPse;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_PSHIFTEXT << 36);
        condition[16] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[16]        = tmpTag;

        // SIS18 to ESR, phase shift injection
        tmpTag        = tagPsi;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_PSHIFTINJ << 36);
        condition[17] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[17]        = tmpTag;


        break;
      case ESR_RING : 

        // ESR, CMD_B2B_START, signals start of data collection
        tmpTag        = tagStart;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_START << 36);
        condition[0]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[0]        = tmpTag;
        
        // ESR, CMD_B2B_START, +100ms (!), signals stop of data collection 
        tmpTag        = tagStop;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_START << 36);
        condition[1]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 100000000, tmpTag));
        //tag[1]        = tmpTag;
        
        // ESR to extraction, PMEXT, 
        tmpTag        = tagPme;       
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PMEXT << 36);
        condition[2]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[2]        = tmpTag;

        // ESR to extraction, PREXT
        tmpTag        = tagPre;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PREXT << 36);
        condition[3]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[3]        = tmpTag;

        // ESR to extraction, DIAGEXT
        tmpTag        = tagPde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGEXT << 36);
        condition[4]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[4]        = tmpTag;
       
        // ESR to CRYRING, PMEXT
        tmpTag        = tagPme;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_CRYRING << 48) | ((uint64_t)B2B_ECADO_B2B_PMEXT << 36);
        condition[5]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[5]        = tmpTag;

        // ESR to CRYRING, PMINJ
        tmpTag        = tagPmi;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_CRYRING << 48) | ((uint64_t)B2B_ECADO_B2B_PMINJ << 36);
        condition[6]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[6]        = tmpTag;

        // ESR to CRYRING, PREXT
        tmpTag        = tagPre;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_CRYRING << 48) | ((uint64_t)B2B_ECADO_B2B_PREXT << 36);
        condition[7]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[7]        = tmpTag;

        // ESR to CRYRING, PRINJ
        tmpTag        = tagPri;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_CRYRING << 48) | ((uint64_t)B2B_ECADO_B2B_PRINJ << 36);
        condition[8]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[8]        = tmpTag;
   
        // ESR to CRYRING, DIAGEXT
        tmpTag        = tagPde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_CRYRING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGEXT << 36);
        condition[9]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[9]        = tmpTag;

        // ESR to CRYRING, DIAGINJ
        tmpTag        = tagPdi;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_CRYRING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGINJ << 36);
        condition[10] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[10]       = tmpTag;

        // ESR extraction kicker trigger
        tmpTag        = tagKte;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_TRIGGEREXT << 36);
        condition[11] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[11]       = tmpTag;
        
        // ESR extraction kicker diagnostic
        tmpTag        = tagKde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGKICKEXT << 36);
        condition[12] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[12]       = tmpTag;

        // CRYRING injection kicker trigger
        tmpTag        = tagKti;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_RING << 48) | ((uint64_t)B2B_ECADO_B2B_TRIGGERINJ << 36);
        condition[13] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[13]       = tmpTag;
        
        // CRYRING injection kicker diagnostic
        tmpTag        = tagKdi;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_RING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGKICKINJ << 36);
        condition[14] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[14]       = tmpTag;

        // ESR to extraction, phase shift extraction
        tmpTag        = tagPse;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PSHIFTEXT << 36);
        condition[15] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[15]        = tmpTag;

        // ESR to CRYRING, phase shift extraction
        tmpTag        = tagPse;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_CRYRING << 48) | ((uint64_t)B2B_ECADO_B2B_PSHIFTEXT << 36);
        condition[16] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[16]        = tmpTag;

        // ESR to CRYRING, phase shift injection
        tmpTag        = tagPsi;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_CRYRING << 48) | ((uint64_t)B2B_ECADO_B2B_PSHIFTINJ << 36);
        condition[17] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[17]        = tmpTag;

        break;
      case CRYRING_RING : 

        // CRYRING, CMD_B2B_START, signals start of data collection
        tmpTag        = tagStart;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_RING << 48) | ((uint64_t)B2B_ECADO_B2B_START << 36);
        condition[0]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[0]        = tmpTag;
        
        // CRYRING, CMD_B2B_START, +100ms (!), signals stop of data collection 
        tmpTag        = tagStop;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_RING << 48) | ((uint64_t)B2B_ECADO_B2B_START << 36);
        condition[1]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 100000000, tmpTag));
        //tag[1]        = tmpTag;
        
        // CRYRING to extraction, PMEXT, 
        tmpTag        = tagPme;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PMEXT << 36);
        condition[2]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[2]        = tmpTag;

        // CRYRING to extraction, PREXT
        tmpTag        = tagPre;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PREXT << 36);
        condition[3]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[3]        = tmpTag;

        // CRYRING to extraction, DIAGEXT
        tmpTag        = tagPde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGEXT << 36);
        condition[4]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[4]        = tmpTag;
       
        // CRYRING extraction kicker trigger
        tmpTag        = tagKte;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_RING << 48) | ((uint64_t)B2B_ECADO_B2B_TRIGGEREXT << 36);
        condition[5]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[5]        = tmpTag;
        
        // CRYRING extraction kicker diagnostic
        tmpTag        = tagKde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_RING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGKICKEXT << 36);
        condition[6]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        //tag[6]        = tmpTag;

        break;
      default :
        std::cerr << "Extraction ring " << reqExtRing << " does not exit" << std::endl;
        exit(1);
    } // switch extRing

    // let's go!
    for (i=0; i<nCondition; i++) {
      condition[i]->setAcceptLate(true);
      condition[i]->setAcceptEarly(true);
      condition[i]->setAcceptConflict(true);
      condition[i]->setAcceptDelayed(true);
      //condition[i]->SigAction.connect(sigc::bind(sigc::ptr_fun(&recTimingMessage), tag[i]));
      condition[i]->setActive(true);    
    } // for i


    eb_device_t   device;
    eb_address_t  ecaq_base;
    char          ebPath[1024];
    uint32_t      recTag;
    uint64_t      deadline;
    uint64_t      evtId; 
    uint64_t      param;  
    uint32_t      tef;
    uint32_t      isLate;
    uint32_t      isEarly;
    uint32_t      isConflict;
    uint32_t      isDelayed;
    saftlib::Time deadline_t;
    uint32_t      ecaStatus;
    eb_status_t   ebStatus;
    uint32_t      qIdx = 0;
    uint64_t      t1, t2;
    uint32_t      tmp32;

    sprintf(ebPath, "%s", receiver->getEtherbonePath().c_str());
    if ((ebStatus = comlib_ecaq_open(ebPath, qIdx, &device, &ecaq_base)) != EB_OK) {
      std::cerr << program << ": can't open lm32 ECA queue" << std::endl;
      exit(1);
    } // if ebStatus
    
    while(true) {
      //      saftlib::wait_for_signal();
      t1 = comlib_getSysTime();
      ecaStatus = comlib_wait4ECAEvent(1, device, ecaq_base, &recTag, &deadline, &evtId, &param, &tef, &isLate, &isEarly, &isConflict, &isDelayed);
      t2 = comlib_getSysTime();
      tmp32 = t2 - t1; 
      if (tmp32 > 10000000) printf("%s: reading from ECA Q took %u [us]\n", program, tmp32 / 1000);
      if (ecaStatus == COMMON_STATUS_EB) { printf("eca EB error, device %x, address %x\n", device, (uint32_t)ecaq_base);}
      if (ecaStatus == COMMON_STATUS_OK) {
        deadline_t = saftlib::makeTimeTAI(deadline);
        //t2         = comlib_getSysTime(); printf("msg: tag %x, id %lx, tef %lx, dtu %lu\n", recTag, evtId, tef, (uint32_t)(t2 -t1));
        timingMessage(recTag, deadline_t, evtId, param, tef, isLate, isEarly, isConflict, isDelayed);
      }
    } // while true
    comlib_ecaq_close(device);
    
  } // try
  catch (const saftbus::Error& error) {
    std::cerr << "Failed to invoke method: " << error.what() << std::endl;
  }
  
  return 0;
} // main

