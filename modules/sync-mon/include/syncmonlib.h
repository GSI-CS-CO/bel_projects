/******************************************************************************
 *  unisissynclib.h
 *
 *  created : 2025
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 10-Jul-2024
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

#define SYNCMON_VERSION 0x000001
#define SYNCMON_STRLEN  64

#define GIDUNILACEXT       0x290        // reference group TK, UNILAC, 'extraction'
#define GIDSIS18INJ        0x12c        // reference group SIS18, 'injection'

#define EVT_BEAM_ON        0x006        // valid beam
#define CMD_BEAM_ON        0x206        // begin of beam passage
#define EVT_MB_TRIGGER     0x028        // start bumper in SIS18 (injection thread)
#define EVT_RAMP_START     0x02b        // start acc/dec ramp in magnets

#define DTLIMIT            2000000000   // limit for time difference [ns]

  enum ringMachine{NORING, SIS18, ESR, CRYRING};
  typedef enum ringMachine ring_t;

  enum evtTag{tagSis18i};
  typedef enum evtTag evtTag_t;


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
