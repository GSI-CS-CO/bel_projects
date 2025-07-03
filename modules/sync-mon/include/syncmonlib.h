/******************************************************************************
 *  unisissynclib.h
 *
 *  created : 2025
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 18-Jul-2024
 *
 * library for checking UNILAC SIS18 synchronization
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
#ifndef _SYNCMON_LIB_H_
#define _SYNCMON_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SYNCMON_VERSION 0x000005
#define SYNCMON_STRLEN  64

#define GID_UNILAC         0x290        // reference group TK, UNILAC
#define GID_SIS18          0x12c        // reference group SIS18
#define GID_ESR            0x154        // reference group ESR
#define GID_YR             0x0d2        // reference group CRYRING

#define EVT_BEAM_ON        0x006        // valid beam
#define CMD_BEAM_ON        0x206        // begin of beam passage
#define EVT_MB_TRIGGER     0x028        // start bumper in SIS18 (injection thread)
#define EVT_RAMP_START     0x02b        // start acc/dec ramp in magnets
#define EVT_MK_LOAD_1      0x02f        // magnet kicker load (SIS18: extraction; ESR: injection)
#define EVT_MK_LOAD_2      0x030        // magnet kicker load (ESR: extraction)
#define CMD_SEPTUM_CHARGE  0x209        // start septum ramp up
#define CMD_B2B_TRIGGERINJ 0x805        // B2B: trigger kicker electronics (injection) 
#define CMD_B2B_START      0x81f        // start B2B procedure
#define CMD_BEAM_INJECTION 0x11b        // injection into a ring machine; played shortly prior to each injection within a sequence
#define DTLIMIT            100000000    // limit for time difference [ns]

  enum machine{NOMACHINE, UNILAC, SIS18, ESR, CRYRING};
  typedef enum machine machine_t;

  enum actionType{unused, uniExt, sis18Inj, sis18Ext, esrInj, esrExt, yrInj};
  typedef enum actionType action_t;


  // data type monitoring values; data are in 'native units' used by the ECA
  typedef struct{
    uint32_t  fid;                                       // FID
    uint32_t  gid;                                       // GID
    uint32_t  evtNo;                                     // EvtNo
    uint32_t  flags;                                     // flags
    uint32_t  sid;                                       // SID
    uint32_t  bpid;                                      // BPID
    uint32_t  eia;                                       // event ID attributes
    uint64_t  param;                                     // parameter field
    uint64_t  deadline;                                  // TAI deadline
    uint32_t  dummy;                                     // to be defined
    uint32_t  counter;                                   // counter of occurences
    char      domainName[SYNCMON_STRLEN];                // name of domain
} monval_t;

  // routines ...

  // get name of event
  void smGetEvtString(uint32_t evtNo,                      // event number
                    char     *name                       // event name
                    );

  // returns empty monitoring data set
  monval_t smEmptyMonData();

  
#ifdef __cplusplus
}
#endif 

#endif
