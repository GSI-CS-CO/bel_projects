/******************************************************************************
 *  wrunipzlib.h
 *
 *  created : 2020
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 20-March-2020
 *
 * library for wrunipz
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
#ifndef _WR_UNIPZ_LIB_H_
#define _WR_UNIPZ_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define WRUNIPZLIB_VERSION "0.01.01"

  // convert status code to status text
  const char* wrunipz_status_text(uint32_t code                // status code
                                  );

  // convert state code to state text
  const char* wrunipz_state_text(uint32_t code                 // state code
                                 );
  
  // open connection to firmware
  uint32_t wrunipz_firmware_open(uint64_t       *ebDevice,     // EB device
                                 const char*    device,        // EB device such as 'dev/wbm0'
                                 uint32_t       cpu,           // # of CPU, 0..0xff
                                 uint32_t       *wbAddr        // WB address of firmware
                                 );
  
  // close connection to firmware
  uint32_t wrunipz_firmware_close(uint64_t ebDevice            // EB device
                                  );
  
  // get version of firmare
  const char* wrunipz_version_firmware(uint64_t ebDevice       // EB device
                                       );

  // get version of library
  const char* wrunipz_version_library();
  
  // get info from firmware
  uint32_t wrunipz_info_read(uint64_t ebDevice,                // EB device
                             uint32_t *ncycles,                // # of cycles executed
                             uint32_t *tCycleAvg,              // average time per cycle [ns]
                             uint32_t *msgFreqAvg,             // average message rate [Hz]
                             uint32_t *nLate,                  // # of late messages
                             uint32_t *vaccAvg,                // virt accs used, bits 0..15 (normal), 16-31 (verkuerzt)
                             uint32_t *pzAvg,                  // PZ used (past second) bits 0..6
                             uint64_t *nMessages,              // # of messages sent
                             int32_t  *dtMax,                  // delta T max (actTime - deadline)
                             int32_t  *dtMin,                  // delta T min (actTime - deadline)
                             int32_t  *cycJmpMax,              // delta T max (expected and actual start of UNILAC cycle)
                             int32_t  *cycJmpMin               // delta T min (expected and actual start of UNILAC cycle)
                             );
  
  // get common properties from firmware
  uint32_t wrunipz_common_read(uint64_t ebDevice,              // EB device
                               uint64_t *statusArray,          // array with status bits
                               uint32_t *state,                // state
                               uint32_t *nBadStatus,           // # of bad status incidents
                               uint32_t *nBadState,            // # of bad state incidents
                               uint32_t *version,              // FW version
                               uint32_t printDiag              // prints info on common firmware properties to stdout
                               );
  
  // uploads (parts of) an event table to the firmware
  uint32_t wrunipz_table_upload(uint64_t ebDevice,             // EB device
                                uint32_t pz,                   // # of PZ;
                                uint32_t vacc,                 // # of vacc;
                                uint32_t chn,                  // # of 'Kanal'; there are max two channels
                                uint32_t *data,                // event data;
                                uint32_t nData                 // # of events in data
                                );

  // commands requesting state transitions
  void wrunipz_cmd_configure(uint64_t ebDevice);               // to state 'configured'
  void wrunipz_cmd_startop(uint64_t ebDevice);                 // to state 'opready'
  void wrunipz_cmd_stopop(uint64_t ebDevice);                  // back to state 'configured'
  void wrunipz_cmd_recover(uint64_t ebDevice);                 // try error recovery
  void wrunipz_cmd_idle(uint64_t ebDevice);                    // to state idle
  
  // commands for normal operation
  void wrunipz_cmd_cleardiag(uint64_t ebDevice);               // clear diagnostic data
  void wrunipz_cmd_submit(uint64_t ebDevice);                  // submit all pending event tables; useful for testing
  void wrunipz_cmd_clearTables(uint64_t ebDevice);             // clears all event tables; useful for testing

  
#ifdef __cplusplus
}
#endif 


#endif
