/******************************************************************************
 *  syncmonlib.c
 *
 *  created : 2025
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 18-Jun-2025
 *
 * library for sync monitoring
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
#include <time.h>

// etherbone
#include <etherbone.h>

// sync-mon
#include <common-defs.h>                 // common definitions
#include <common-lib.h>                  // common routines
#include <common-core.h>                 // common core
#include <syncmonlib.h>                  // x86 library


void smGetEvtString(uint32_t evtNo, char *name)
{
  switch (evtNo) {
    case EVT_BEAM_ON        : sprintf(name, "%s", "EVT_BEAM_ON")        ; break;
    case CMD_BEAM_ON        : sprintf(name, "%s", "CMD_BEAM_ON")        ; break;
    case EVT_MB_TRIGGER     : sprintf(name, "%s", "EVT_MB_TRIGGER")     ; break;
    case EVT_RAMP_START     : sprintf(name, "%s", "EVT_RAMP_START")     ; break;
    case EVT_MK_LOAD_1      : sprintf(name, "%s", "EVT_MK_LOAD_1")      ; break;
    case EVT_MK_LOAD_2      : sprintf(name, "%s", "EVT_MK_LOAD_2")      ; break;
    case CMD_SEPTUM_CHARGE  : sprintf(name, "%s", "CMD_SEPT_CHARGE")    ; break;
    case CMD_B2B_TRIGGERINJ : sprintf(name, "%s", "CMD_B2B_TRIGINJ")    ; break;
    case CMD_B2B_START      : sprintf(name, "%s", "CMD_B2B_START")      ; break;
    case CMD_BEAM_INJECTION : sprintf(name, "%s", "CMD_BEAM_INJECT")    ; break;
    default                 : sprintf(name, "%s", "undefined")          ; break;
  } // switch evtNo
} // smGetEvtString


monval_t smEmptyMonData()
{
  monval_t data;

  data.fid      = 0x0;
  data.gid      = 0x0;
  data.evtNo    = 0x0;
  data.flags    = 0x0;
  data.sid      = 0x0;
  data.bpid     = 0x0;
  data.eia      = 0x0;
  data.param    = 0x0;
  data.deadline = 0x0;
  data.dummy    = 0x0;
  data.counter  = 0;

  return data;
} // smEmpyMonData
