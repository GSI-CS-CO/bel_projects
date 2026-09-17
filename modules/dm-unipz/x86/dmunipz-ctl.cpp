/******************************************************************************
 *  dmunipz-ctl.cpp
 *
 *  created : 2017
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 11-Nov-2024
 *
 * Command-line interface for dmunipz
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
#define DMUNIPZ_X86_VERSION "00.09.02"

// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

// includes for bel_projects
#include <etherbone.h>

// includes for this project
#include "../../ftm/include/ftm_common.h"  // defs and regs for data master
#include <dm-unipz.h>
#include <common-lib.h>                    // this is C code, >> always compile using g++ <<
#include <dmunipz_shared_mmap.h>

// USE MASP
#ifdef USEMASP
// includes for MASP
#include "MASP/Emitter/End_of_scope_status_emitter.h"
#include "MASP/StatusDefinition/DeviceStatus.h"
#include "MASP/Util/Logger.h"
#include "MASP/Common/StatusNames.h"
#include <boost/thread/thread.hpp> // (sleep)
#include <iostream>
#include <string>

// includes for unipz
#include <limits.h>

std::string   maspNomen = std::string(DMUNIPZ_MASP_NOMEN);
std::string   maspSourceId; 
bool          maspProductive;     // send to pro/dev masp
bool          maspSigOpReady;     // value for MASP signal OP_READY
bool          maspSigTransfer;    // value for MASP signal TRANSFER (custom emitter)

MASP::StatusEmitterConfig get_config() {
  char   hostname[HOST_NAME_MAX];

  gethostname(hostname, HOST_NAME_MAX);
  maspSourceId   = maspNomen + "." + std::string(hostname);

#ifdef PRODUCTIVE
  maspProductive = true;
#else
  maspProductive = false;
#endif

  MASP::StatusEmitterConfig config = MASP::StatusEmitterConfig(MASP::StatusEmitterConfig::CUSTOM_EMITTER_DEFAULT(), maspSourceId, maspProductive);
  return config;
}
#endif

const char* program;
static int getInfo    = 0;
static int getConfig  = 0;
static int getVersion = 0;
static int snoop      = 0;
static int logLevel   = 0;

eb_device_t  device;               // keep this and below global
eb_address_t lm32_base;            // base address of lm32
eb_address_t dmunipz_iterations;   // # of iterations of main loop, read
eb_address_t dmunipz_virtAccReq;   // # of requested virtual accelerator of ongoing or last transfer, read
eb_address_t dmunipz_virtAccRec;   // # of received virtual accelerator of ongoing or last transfer, read
eb_address_t dmunipz_noBeam;       // requested 'noBeam' flag, read
eb_address_t dmunipz_dtStart;      // difference between actual time and flextime @ DM
eb_address_t dmunipz_dtSync1;      // time difference between EVT_READY_TO_SIS and EVT_MB_TRIGGER
eb_address_t dmunipz_dtSync2;      // time difference between EVT_READY_TO_SIS and CMD_UNI_TCREL
eb_address_t dmunipz_dtInject;     // time difference between CM_UNI_BREQ and EVT_MB_LOAD
eb_address_t dmunipz_dtTransfer;   // time difference between CM_UNI_TKREQ and EVT_MB_LOAD
eb_address_t dmunipz_dtTkreq;      // time difference between CMD_UNI_TKREQ and reply from UNIPZ
eb_address_t dmunipz_dtBreq;       // time difference between CMD_UNI_BREQ and reply from UNIPZ
eb_address_t dmunipz_dtBprep;      // time difference between CMD_UNI_BREQ and begin of request to UNIPZ
eb_address_t dmunipz_dtReady2Sis;  // time difference between CMD_UNI_BREQ and EVT_READY_TO_SIS
eb_address_t dmunipz_nR2sTransfer; // # of EVT_READY_TO_SIS events in between CMD_UNI_TKREQ and CMD_UNI_TKREL
eb_address_t dmunipz_nR2sCycle;    // # of EVT_READY_TO_SIS events in between CMD_UNI_TKRELand the following CMD_UNI_TKREL
eb_address_t dmunipz_nBooster;     // # of booster injections
eb_address_t dmunipz_cmd;          // commands
eb_address_t dmunipz_dstMacHi;     // ebm dst mac, write
eb_address_t dmunipz_dstMacLo;     // ebm dst mac, write
eb_address_t dmunipz_dstIp;        // ebm dst ip, write
eb_address_t dmunipz_flexOffset;   // offset added to timestamp of MIL event for schedule continuation
eb_address_t dmunipz_uniTimeout;   // timeout value for UNILAC
eb_address_t dmunipz_tkTimeout;    // timeout value for TK (via UNILAC)

eb_data_t   data1;

 
static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


const char* dmunipz_statusText(uint32_t bit) {  
  static char message[256];

  switch (bit) {
    case DMUNIPZ_STATUS_REQTKFAILED      : sprintf(message, "error %d, %s",    bit, "UNILAC refuses TK request"); break;
    case DMUNIPZ_STATUS_REQTKTIMEOUT     : sprintf(message, "error %d, %s",    bit, "UNILAC TK request timed out"); break;
    case DMUNIPZ_STATUS_REQBEAMFAILED    : sprintf(message, "error %d, %s",    bit, "UNILAC refuses beam request"); break;
    case DMUNIPZ_STATUS_RELTKFAILED      : sprintf(message, "error %d, %s",    bit, "UNILAC refuses to release TK request"); break;
    case DMUNIPZ_STATUS_RELBEAMFAILED    : sprintf(message, "error %d, %s",    bit, "UNILAC refuses to release beam request"); break;
    case DMUNIPZ_STATUS_DEVBUSERROR      : sprintf(message, "error %d, %s",    bit, "something went wrong with write/read on the MIL devicebus"); break;
    case DMUNIPZ_STATUS_REQNOTOK         : sprintf(message, "error %d, %s",    bit, "UNILAC signals 'request not ok'"); break;
    case DMUNIPZ_STATUS_REQBEAMTIMEDOUT  : sprintf(message, "error %d, %s",    bit, "UNILAC beam request timed out"); break;
    case DMUNIPZ_STATUS_NODM             : sprintf(message, "error %d, %s",    bit, "Data Master unreachable"); break;                     
    case DMUNIPZ_STATUS_WRONGVIRTACC     : sprintf(message, "error %d, %s",    bit, "mismatching virtual accelerator for EVT_READY_TO_SIS from UNIPZ"); break;
    case DMUNIPZ_STATUS_SAFETYMARGIN     : sprintf(message, "error %d, %s",    bit, "violation of safety margin for data master and timing network"); break;
    case DMUNIPZ_STATUS_NOTIMESTAMP      : sprintf(message, "error %d, %s",    bit, "received EVT_READY_TO_SIS in MIL FIFO but no TS via TLU -> ECA"); break;
    case DMUNIPZ_STATUS_BADTIMESTAMP     : sprintf(message, "error %d, %s",    bit, "TS from TLU->ECA does not coincide with MIL Event from FIFO"); break;
    case DMUNIPZ_STATUS_DMQNOTEMPTY      : sprintf(message, "error %d, %s",    bit, "Data Master: Q not empty"); break;
    case DMUNIPZ_STATUS_LATEEVENT        : sprintf(message, "error %d, %s",    bit, "received 'late event' from Data Master"); break;
    case DMUNIPZ_STATUS_TKNOTRESERVED    : sprintf(message, "error %d, %s",    bit, "TK is not reserved"); break;
    case DMUNIPZ_STATUS_DMTIMEOUT        : sprintf(message, "error %d, %s",    bit, "beam request did not succeed within 10s timeout at DM"); break;
    case DMUNIPZ_STATUS_BADSYNC          : sprintf(message, "error %d, %s",    bit, "t(EVT_MB_TRIGGER) - t(EVT_READY_TO_SIS) != 10ms"); break;
    case DMUNIPZ_STATUS_WAIT4UNIEVENT    : sprintf(message, "error %d, %s",    bit, "timeout while waiting for EVT_READY_TO_SIS"); break;
    case DMUNIPZ_STATUS_BADSCHEDULEA     : sprintf(message, "error %d, %s",    bit, "t(EVT_MB_TRIGGER) - t(CMD_UNI_BREQ) < 10ms"); break;
    case DMUNIPZ_STATUS_BADSCHEDULEB     : sprintf(message, "error %d, %s",    bit, "unexpected event"); break;
    case DMUNIPZ_STATUS_INVALIDBLKADDR   : sprintf(message, "error %d, %s",    bit, "invalid address of block for Data Master"); break;
    case DMUNIPZ_STATUS_INVALIDTHRADDR   : sprintf(message, "error %d, %s",    bit, "invalid address of thread handling area for Data Master"); break;
    case DMUNIPZ_STATUS_DELAYEDEVENT     : sprintf(message, "error %d, %s",    bit, "received 'delayed event'");break;
    default                              : return comlib_statusText(bit); break;
  }

  return message;
} // dmunipz_statusText


const char* dmunipz_transferStatusText(uint32_t bit) {
  switch (bit) {
    case DMUNIPZ_TRANS_REQTK      : return "TK requested";
    case DMUNIPZ_TRANS_REQTKOK    : return "TK request succeeded";
    case DMUNIPZ_TRANS_RELTK      : return "TK released";
    case DMUNIPZ_TRANS_REQBEAM    : return "beam requested";
    case DMUNIPZ_TRANS_REQBEAMOK  : return "beam request succeeded";
    case DMUNIPZ_TRANS_RELBEAM    : return "beam released";
    case DMUNIPZ_TRANS_PREPBEAM   : return "beam preparation requested";
    case DMUNIPZ_TRANS_UNPREPBEAM : return "beam preparation released";
  default                      : return "undefined transfer status";
  }
} // dmunipz_transferStatusText


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> [COMMAND]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -c                  display configuration of gateway\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "  -i                  display information on gateway\n");
  fprintf(stderr, "  -s<n>               snoop gateway for information continuously\n");
  fprintf(stderr, "                      0: print all messages (default)\n");
  fprintf(stderr, "                      1: as 0, but without info on ongoing transfers\n");
  fprintf(stderr, "                      2: as 1, but without info on transfers\n");
  fprintf(stderr, "                      3: as 2, but without info on status\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  ebmdm    <mac> <ip> command sets DM WR MAC and IP for EB master (values in hex)\n");
  fprintf(stderr, "  flex     <offset>   command sets offset [ns] added to timestamp of READY_TO_SIS event (default 1500000 ns)\n");
  fprintf(stderr, "  uni      <timeout>  command sets timeout [ms] value for UNILAC (default 2000ms)\n");
  fprintf(stderr, "  tk       <timeout>  command sets timeout [ms] value for TK (via UNILAC, default 210 ms)\n");
  fprintf(stderr, "  IMPORTANT: values set by the above commands get applied by the 'configure' command\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  configure           command requests state change from IDLE or CONFIGURED to CONFIGURED\n");
  fprintf(stderr, "  startop             command requests state change from CONFIGURED to OPREADY\n");
  fprintf(stderr, "  stopop              command requests state change from OPREADY to STOPPING -> CONFIGURED\n");
  fprintf(stderr, "  recover             command tries to recover from state ERROR and transit to state IDLE\n");
  fprintf(stderr, "  idle                command requests state change to IDLE\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  reltk               command forces release of TK request at UNILAC\n");
  fprintf(stderr, "  relprep             command forces release of preparation request at UNILAC\n");
  fprintf(stderr, "  relbeam             command forces release of beam request at UNILAC\n");  
  fprintf(stderr, "\n");
  fprintf(stderr, "  diag                shows statistics and detailed information\n");
  fprintf(stderr, "  cleardiag           command clears FW statistics\n");
  fprintf(stderr, "  debugon             command enables debugging\n");
  fprintf(stderr, "  debugoff            command disables debuggging\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to control the DM-UNIPZ gateway from the command line\n");
  fprintf(stderr, "Example1: '%s dev/wbm0 ebmdm 0x00267b000401 0xc0a80a01' set MAC and IP of Data Master\n", program);
  fprintf(stderr, "Example2: '%s -i dev/wbm0' display typical information\n", program);
  fprintf(stderr, "Example3: '%s -s1 dev/wbm0 | logger -t TIMING -sp local0.info' monitor firmware and print to screen and to diagnostic logging", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "When using option '-s<n>', the following information is displayed\n");
  fprintf(stderr, "dm-unipz:                  TRANSFERS                |                        INJECTION                                | DIAGNOSIS  |                    INFO   \n");
  fprintf(stderr, "dm-unipz:              n    sum(tkr)  set(get)/noBm | multi/boost(r2s/sumr2s)  sum( init/bmrq/r2sis->mbtrig)          | DIAG margn | status         state      nchng stat   nchng\n");
  fprintf(stderr, "dm-unipz: TRANS 00057399,  5967( 13)ms, va 10(10)/0 | INJ 00006/00000(06/06),  964(0.146/   0/ 954 -> 9.979/17.512)ms | DG 1.453ms | 1 1 1 1 1 1 1 1, OpReady    (     0), OK (     4)\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '   | ' ' ' ' ' ' ' '        '          '    '       ' \n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '   | ' ' ' ' ' ' ' '        '          '    '       ' - # of 'bad status' incidents\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '   | ' ' ' ' ' ' ' '        '          '    '- status\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '   | ' ' ' ' ' ' ' '        '          ' - # of '!OpReady' incidents\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '   | ' ' ' ' ' ' ' '        '- state\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '   | ' ' ' ' ' ' ' '- beam (request) released\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '   | ' ' ' ' ' ' '- beam request succeeded\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '   | ' ' ' ' ' '- beam requested\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '   | ' ' ' ' '- beam preparation (request) released\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '   | ' ' ' '- beam preparation requested\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '   | ' ' '- TK (request) released -> transfer completed\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '   | ' '- TK request succeeded\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '   | '- TK requested\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '    - STATUS info ...\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |        '- remaining budget for data master and network  [ms] (> 1ms)\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '    |- DIAGNOSTIC info ...\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '      '- offset CMD_UNI_TCREL -> EVT_MB_TRIGGER [ms] (~16ms) \n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '        '- offset EVT_READY_TO_SIS -> EVT_MB_TRIGGER [ms] (~10ms) \n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '    '- offset beam request <-> EVT_READY_TO_SIS [ms] (< 2000ms)\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '    '- period required for acknowledgement of beam request at UNILAC [ms] (~20ms)\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '     '- period required for init of beam request ('DM Schnitzeljagd', ~0.2ms)\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '      '- total period required for injection [ms]\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '  '- # of received EVT_READY_TO_SIS since last sequence (should equal number of injections; if larger, another virtAcc to TK at UNILAC is present)\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '  '- # of received EVT_READY_TO_SIS during transfer (should equal the number of injections)\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '     '- # of booster injections within transfer\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |         '- # of multi-multi injections within transfer\n");
  fprintf(stderr, "          |            '      '   '         '  '  '  - INJECTION info ...\n");
  fprintf(stderr, "          |            '      '   '         '  '  '- 'no beam' flag\n");
  fprintf(stderr, "          |            '      '   '         '  ' # of virtAcc delivered by UNILAC (get value)\n");
  fprintf(stderr, "          |            '      '   '         '- # of virtAcc requested from UNILAC (set value)\n");
  fprintf(stderr, "          |            '      '   '- period required for acknowledgement of TK request at UNILAC [ms] (< 210ms)\n");
  fprintf(stderr, "          |            '      '- period required for transfer [ms] (including all injections)\n");
  fprintf(stderr, "          |            '- # of transfers\n");
  fprintf(stderr, "           - TRANSFER info ...\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "When using command 'debug_on', the following data is injected into the local ECA\n");
  fprintf(stderr, "FID: 0xc GID: 0x0afe EVTNO: 0x0fa0 - 'deadline': requested execution time stamp @DM, 'Other': address of TS @ DM\n");
  fprintf(stderr, "FID: 0xc GID: 0x0afe EVTNO: 0x0fa1 - 'Other': address of control register @ DM     , 'Param': data written to CR\n");
  fprintf(stderr, "FID: 0xc GID: 0x0afe EVTNO: 0x0fa2 - 'deadline': valid time of command    @ DM     , 'Other': address of command @ DM, 'Param': command data (first part)\n");
  fprintf(stderr, "FID: 0xc GID: 0x0afe EVTNO: 0x0fa3 - 'Other': address of write index @ DM          , 'Param': write index\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", DMUNIPZ_X86_VERSION);
} //help


int readInfo(uint32_t *iterations, uint32_t *virtAccReq, uint32_t *virtAccRec, uint32_t *noBeam, uint32_t *dtStart, uint32_t *dtSync1, uint32_t *dtSync2, uint32_t *dtInject, uint32_t *dtTransfer, uint32_t *dtTkreq, uint32_t *dtBreq, uint32_t *dtBprep, uint32_t *dtReady2Sis, uint32_t *nR2sTransfer, uint32_t *nR2sCycle, uint32_t *nBooster)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[30];
  
  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("dm-unipz: eb_cycle_open", eb_status);
  eb_cycle_read(cycle, dmunipz_iterations,    EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, dmunipz_virtAccReq,    EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, dmunipz_virtAccRec,    EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, dmunipz_noBeam,        EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, dmunipz_dtStart,       EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(cycle, dmunipz_dtSync1,       EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(cycle, dmunipz_dtInject,      EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(cycle, dmunipz_dtTransfer,    EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(cycle, dmunipz_dtTkreq,       EB_BIG_ENDIAN|EB_DATA32, &(data[8]));
  eb_cycle_read(cycle, dmunipz_dtBreq,        EB_BIG_ENDIAN|EB_DATA32, &(data[9]));  
  eb_cycle_read(cycle, dmunipz_dtReady2Sis,   EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(cycle, dmunipz_nR2sTransfer,  EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  eb_cycle_read(cycle, dmunipz_nR2sCycle,     EB_BIG_ENDIAN|EB_DATA32, &(data[12]));
  eb_cycle_read(cycle, dmunipz_dtBprep,       EB_BIG_ENDIAN|EB_DATA32, &(data[13]));
  eb_cycle_read(cycle, dmunipz_nBooster,      EB_BIG_ENDIAN|EB_DATA32, &(data[14]));
  eb_cycle_read(cycle, dmunipz_dtSync2,       EB_BIG_ENDIAN|EB_DATA32, &(data[15]));

  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("dm-unipz: eb_cycle_close", eb_status);

  *iterations    = data[0];
  *virtAccReq    = data[1];
  *virtAccRec    = data[2];
  *noBeam        = data[3];
  *dtStart       = data[4];
  *dtSync1       = data[5];
  *dtInject      = data[6];
  *dtTransfer    = data[7];
  *dtTkreq       = data[8];
  *dtBreq        = data[9];
  *dtReady2Sis   = data[10];
  *nR2sTransfer  = data[11];
  *nR2sCycle     = data[12];
  *dtBprep       = data[13];
  *nBooster      = data[14];
  *dtSync2       = data[15];

  return eb_status;
} // readInfo


int readConfig(uint32_t *flexOffset, uint32_t *uniTimeout, uint32_t *tkTimeout, uint64_t *dstMac, uint32_t *dstIp)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[10];

  uint32_t    dstMacHi, dstMacLo;

  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("dm-unipz: eb_cycle_open", eb_status);

  eb_cycle_read(cycle, dmunipz_dstMacHi,   EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, dmunipz_dstMacLo,   EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, dmunipz_dstIp,      EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, dmunipz_flexOffset, EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, dmunipz_uniTimeout, EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(cycle, dmunipz_tkTimeout,  EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("dm-unipz: eb_cycle_close", eb_status);

  dstMacHi      = data[0];
  dstMacLo      = data[1];
  *dstIp        = data[2];
  *flexOffset   = data[3];
  *uniTimeout   = data[4];
  *tkTimeout    = data[5];
  
  *dstMac = dstMacHi;
  *dstMac = (*dstMac << 32);
  *dstMac = *dstMac + dstMacLo;

  return eb_status;
} //readConfig


void printTransferHeader()
{
  printf("dm-unipz:                  TRANSFERS                |                              INJECTION                            | DIAGNOSIS  |                        INFO                       \n");
  printf("dm-unipz:              n    sum(tkr)  set(get)/noBm | multi/boost(r2s/sumr2s)   sum( init/bmrq/r2sis->mbtrig/tcrel)  | DIAG margn | status             state      nchng   stat   nchng\n");
} // printTransferHeader


void printTransfer(uint32_t transfers,
                   uint32_t injections,
                   uint32_t virtAccReq,
                   uint32_t virtAccRec,
                   uint32_t noBeam,
                   uint32_t dtStart,
                   uint32_t dtSync1,
                   uint32_t dtSync2,
                   uint32_t dtInject,
                   uint32_t dtTransfer,
                   uint32_t dtTkreq,
                   uint32_t dtBreq,
                   uint32_t dtBprep,
                   uint32_t dtReady2Sis,
                   uint32_t nR2sTransfer,
                   uint32_t nR2sCycle,
                   uint32_t nBooster,
                   uint32_t statTrans)
{

  char temp1[64];
  char temp2[64];
  char temp3[64];
  char temp4[64];
  char temp5[64];
  char temp6[64];

  // transfer
  if (virtAccReq  == 42)         sprintf(temp1, "--");
  else                           sprintf(temp1, "%2d", virtAccReq);
  if (virtAccRec  == 42)         sprintf(temp2, "--");
  else                           sprintf(temp2, "%2d", virtAccRec - (uint32_t)(100 * floor(virtAccRec / 100)));
  if (dtTransfer  == 0xffffffff) sprintf(temp3, "-----");
  else                           sprintf(temp3, "%5d", (uint32_t)((double)dtTransfer / 1000.0));
  if (dtTkreq     == 0xffffffff) sprintf(temp5, "---");
  else                           sprintf(temp5, "%3d", (uint32_t)((double)dtTkreq / 1000.0));
  if (noBeam      == 0xffffffff) sprintf(temp4, "-");
  else                           sprintf(temp4, "%1d", noBeam);
  printf("dm-unipz: TRANS %08d, %s(%s)ms, va %s(%s)/%s | ", transfers, temp3, temp5, temp1, temp2, temp4);

  // injection
  if (dtInject    == 0xffffffff) sprintf(temp1, "----");
  else                           sprintf(temp1, "%4d", (uint32_t)((double)dtInject / 1000.0));
  if (dtBprep     == 0xffffffff) sprintf(temp2, "-----");
  else                           sprintf(temp2, "%5.3f", (double)dtBprep / 1000.0);
  if (dtBreq      == 0xffffffff) sprintf(temp3, "----");
  else                           sprintf(temp3, "%4d", (uint32_t)((double)dtBreq / 1000.0));
  if (dtReady2Sis == 0xffffffff) sprintf(temp4, "----");
  else                           sprintf(temp4, "%4d", (uint32_t)((double)dtReady2Sis / 1000.0));
  if (dtSync1     == 0xffffffff) sprintf(temp5, "------");
  else                           sprintf(temp5, "%6.3f", (double)dtSync1 / 1000.0);
  if (dtSync2     == 0xffffffff) sprintf(temp6, "------");
  else                           sprintf(temp6, "%6.3f", (double)dtSync2 / 1000.0);

  printf("INJ %05d/%05d(%02d/%02d), %s(%s/%s/%s ->%s/%s)ms | ", injections, nBooster, nR2sTransfer, nR2sCycle, temp1, temp2, temp3, temp4, temp5, temp6);

  // diag
  if (dtStart     == 0xffffffff) sprintf(temp1, "-----");
  else                           sprintf(temp1, "%5.3f", (double)dtStart / 1000.0);
  printf("DG %sms | ", temp1);

  // status
  printf("%d %d %d %d %d %d %d %d", 
         ((statTrans & (0x1 << DMUNIPZ_TRANS_REQTK)     ) > 0),  
         ((statTrans & (0x1 << DMUNIPZ_TRANS_REQTKOK)   ) > 0), 
         ((statTrans & (0x1 << DMUNIPZ_TRANS_RELTK)     ) > 0),
         ((statTrans & (0x1 << DMUNIPZ_TRANS_PREPBEAM)  ) > 0),
         ((statTrans & (0x1 << DMUNIPZ_TRANS_UNPREPBEAM)) > 0),
         ((statTrans & (0x1 << DMUNIPZ_TRANS_REQBEAM)   ) > 0),
         ((statTrans & (0x1 << DMUNIPZ_TRANS_REQBEAMOK) ) > 0),
         ((statTrans & (0x1 << DMUNIPZ_TRANS_RELBEAM)   ) > 0)
         );
} // printTransfer


int main(int argc, char** argv) {
#define GSI           0x00000651
#define LM32_RAM_USER 0x54111351

  
  eb_status_t         eb_status;
  eb_socket_t         socket;

  struct sdb_device   sdbDevice;          // instantiated lm32 core
  int                 nDevices;           // number of instantiated cores
  
  const char* devName;
  const char* command;

  int opt, error = 0;
  int exitCode   = 0;
  char *tail;

  uint64_t statusArray;    
  uint32_t state;
  uint32_t nBadStatus;
  uint32_t nBadState;
  uint32_t iterations;
  uint32_t nTransfer; 
  uint32_t nInjection;
  uint32_t nLate;
  uint32_t offsDone;
  uint32_t comLatency;

  uint32_t virtAccReq;   
  uint32_t virtAccRec;   
  uint32_t noBeam;
  uint32_t dtStart;
  uint32_t dtSync1;
  uint32_t dtSync2;
  uint32_t dtInject;  
  uint32_t dtTransfer;  
  uint32_t dtTkreq;  
  uint32_t dtBreq;  
  uint32_t dtBprep;  
  uint32_t dtReady2Sis;  
  uint32_t nR2sTransfer;  
  uint32_t nR2sCycle;
  uint32_t nBooster;
  uint32_t statTrans;
  uint32_t usedSize;
  uint32_t version;

  uint32_t actTransfer;                        // actual # of transfer
  uint32_t actInjection;                       // actual # of injection within current transfer
  uint32_t actState = COMMON_STATE_UNKNOWN;    // actual state of gateway
  uint64_t actStatusArray;                     // actual status of gateway
  uint32_t sleepTime;                          // time to sleep [us]
  uint32_t printFlag;                          // flag for printing

  uint64_t dstMac;                             // mac for config of EB master (Data Master)
  uint32_t dstMacHi;                           // mac, high word
  uint32_t dstMacLo;                           // mac, low word
  uint32_t dstIp;                              // ip for config of EB master (Data Master)
  uint64_t mac;                                // mac of WR interface (local)
  uint32_t ip;                                 // ip of WR interface (local)
  uint64_t tDiag;                              // time, when diag data was reset
  uint64_t tS0;                                // time, when entering S0 state (firmware boot)
  uint32_t flexOffset;                         // offset value added to MIL EVent timestamp
  uint32_t uniTimeout;                         // timeout value for UNILAC
  uint32_t tkTimeout;                          // timeout value for TK (via UNILAC)

  int      i;
  
    
  program = argv[0];    

  while ((opt = getopt(argc, argv, "s:ceih")) != -1) {
    switch (opt) {
    case 'c':
      getConfig = 1;
      break;
    case 'e':
      getVersion = 1;
      break;
    case 'i':
      getInfo = 1;
      break;
    case 's':
      snoop = 1;
      logLevel = strtol(optarg, &tail, 0);
      if (*tail != 0) {
        fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
        exit(1);
      } /* if *tail */
      if ((logLevel < COMMON_LOGLEVEL_ALL) || (logLevel > COMMON_LOGLEVEL_STATE)) fprintf(stderr, "log level out of range\n");
      break;
    case 'h':
      help();
      return 0;
      case ':':
      case '?':
        error = 1;
      break;
    default:
      fprintf(stderr, "%s: bad getopt result\n", program);
      return 1;
    } /* switch opt */
  } /* while opt */

  if (error) {
    help();
    return 1;
  }
  
  if (optind >= argc) {
    fprintf(stderr, "%s: expecting one non-optional argument: <etherbone-device>\n", program);
    fprintf(stderr, "\n");
    help();
    return 1;
  }

  devName = argv[optind];

  if (optind+1 < argc)  command = argv[++optind];
  else command = NULL;

  /* open Etherbone device and socket */
  if ((eb_status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32|EB_DATA32, &socket)) != EB_OK) die("eb_socket_open", eb_status);
  if ((eb_status = eb_device_open(socket, devName, EB_ADDR32|EB_DATA32, 3, &device)) != EB_OK) die("eb_device_open", eb_status);

  /* get device Wishbone address of lm32 */
  nDevices = 1; // quick and dirty, use only first core
  if ((eb_status = eb_sdb_find_by_identity(device, GSI, LM32_RAM_USER, &sdbDevice, &nDevices)) != EB_OK) die("find lm32", eb_status);
  lm32_base =  sdbDevice.sdb_component.addr_first;

  comlib_initShared(lm32_base, SHARED_OFFS);
  dmunipz_iterations   = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_NITERMAIN;
  dmunipz_virtAccReq   = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_TRANSVIRTACC;
  dmunipz_virtAccRec   = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_RECVIRTACC;
  dmunipz_noBeam       = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_TRANSNOBEAM;
  dmunipz_dtStart      = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_DTSTART;
  dmunipz_dtSync1      = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_DTSYNC1;
  dmunipz_dtSync2      = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_DTSYNC2;
  dmunipz_dtInject     = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_DTINJECT;
  dmunipz_dtTransfer   = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_DTTRANSFER;
  dmunipz_dtTkreq      = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_DTTKREQ;
  dmunipz_dtBreq       = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_DTBREQ;
  dmunipz_dtBprep      = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_DTBPREP;
  dmunipz_dtReady2Sis  = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_DTREADY2SIS;
  dmunipz_nR2sTransfer = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_NR2STRANSFER;
  dmunipz_nR2sCycle    = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_NR2SCYCLE;
  dmunipz_nBooster     = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_NBOOSTER;
  dmunipz_dstMacHi     = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_DSTMACHI;
  dmunipz_dstMacLo     = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_DSTMACLO;
  dmunipz_dstIp        = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_DSTIP;
  dmunipz_flexOffset   = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_OFFSETTHRD;
  dmunipz_uniTimeout   = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_UNITIMEOUT;
  dmunipz_tkTimeout    = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_TKTIMEOUT;
  dmunipz_cmd          = lm32_base + SHARED_OFFS + COMMON_SHARED_CMD;

  if (getConfig) {
    readConfig(&flexOffset, &uniTimeout, &tkTimeout, &dstMac, &dstIp);
    printf("dm-unipz: the values below are applied if the gateway becomes 'CONFIGURED'\n");
    printf( " dm-unipz: flexOffset %" PRIu32 " ns, uniTimeout %" PRIu32 " ms, tkTimeout %" PRIu32 " ms\n " , flexOffset, uniTimeout, tkTimeout);
    printf("dm-unipz: EB Master (DM   ): mac 0x%012" PRIx64 ", ip %03d.%03d.%03d.%03d\n", dstMac, (dstIp & 0xff000000) >> 24, (dstIp & 0x00ff0000) >> 16, (dstIp & 0x0000ff00) >> 8, (dstIp & 0x000000ff));
  } // if getConfig

  if (getVersion) {
    comlib_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, &nLate, &offsDone, &comLatency, &usedSize, 0);
    printf("dm-unipz: software (firmware) version %s (%06x)\n",  DMUNIPZ_X86_VERSION, version);     
  } // if getEBVersion

  if (getInfo) {
    // status
    comlib_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, &nLate, &offsDone, &comLatency, &usedSize, 1);
    readInfo(&iterations, &virtAccReq, &virtAccRec, &noBeam, &dtStart, &dtSync1, &dtSync2, &dtInject, &dtTransfer, &dtTkreq, &dtBreq, &dtBprep, &dtReady2Sis, &nR2sTransfer, &nR2sCycle, &nBooster);
    printTransferHeader();
    printTransfer(nTransfer, nInjection, virtAccReq, virtAccRec, noBeam, dtStart, dtSync1, dtSync2, dtInject, dtTransfer, dtTkreq, dtBreq, dtBprep, dtReady2Sis, nR2sTransfer, nR2sCycle, nBooster, statTrans); 
    printf(", %s (%6u), ",  comlib_stateText(state), nBadState);
    if ((statusArray >> COMMON_STATUS_OK) & 0x1) printf("OK   (%6u)\n", nBadStatus);
    else                                         printf("NOTOK(%6u)\n", nBadStatus);
  } // if getInfo

  if (command) {
    // state required to give proper warnings
    comlib_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, &nLate, &offsDone, &comLatency, &usedSize, 0);

    if (!strcasecmp(command, "configure")) {
      eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CONFIGURE, 0, eb_block);
      if ((state != COMMON_STATE_CONFIGURED) && (state != COMMON_STATE_IDLE)) printf("dm-unipz: WARNING command has not effect (not in state CONFIGURED or IDLE)\n");
    } // "configure"
    if (!strcasecmp(command, "startop")) {
      eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STARTOP  , 0, eb_block);
      if (state != COMMON_STATE_CONFIGURED) printf("dm-unipz: WARNING command has not effect (not in state CONFIGURED)\n");
    } // "startop"
    if (!strcasecmp(command, "stopop")) {
      eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STOPOP   , 0, eb_block);
      if (state != COMMON_STATE_OPREADY) printf("dm-unipz: WARNING command has not effect (not in state OPREADY)\n");
    } // "startop"
    if (!strcasecmp(command, "recover")) {
      eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_RECOVER  , 0, eb_block);
      if (state != COMMON_STATE_ERROR) printf("dm-unipz: WARNING command has not effect (not in state ERROR)\n");
    } // "recover"
    if (!strcasecmp(command, "idle")) {
      eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_IDLE     , 0, eb_block);
      if (state != COMMON_STATE_CONFIGURED) printf("dm-unipz: WARNING command has not effect (not in state CONFIGURED)\n");
    } // "idle"
    if (!strcasecmp(command, "cleardiag")) {
      eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CLEARDIAG, 0, eb_block);
    } // "cleardiag"    
    if (!strcasecmp(command, "debugon")) {
      eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)DMUNIPZ_CMD_DEBUGON  , 0, eb_block);
    } // "debugon"
    if (!strcasecmp(command, "debugoff")) {
      eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)DMUNIPZ_CMD_DEBUGOFF , 0, eb_block);
    } // "debugoff"

    if (!strcasecmp(command, "diag")) {
      comlib_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, &nLate, &offsDone, &comLatency, &usedSize, 1);
      readConfig(&flexOffset, &uniTimeout, &tkTimeout, &dstMac, &dstIp);
      printf("\ndm-unipz: the values below are applied if the gateway becomes 'CONFIGURED'\n");
      printf("flexOffset                          : %" PRIu32 " ns\n", flexOffset);
      printf("uniTimeout                          : %" PRIu32 " ms\n", uniTimeout);
      printf("tkTimeout                           : %" PRIu32 " ms\n", tkTimeout);
      printf("DM MAC                              : 0x%012" PRIx64 "\n", dstMac);
      printf("DM IP                               : %03d.%03d.%03d.%03d\n", (dstIp & 0xff000000) >> 24, (dstIp & 0x00ff0000) >> 16, (dstIp & 0x0000ff00) >> 8, (dstIp & 0x000000ff));
    } // "diag"
    
    if (!strcasecmp(command, "ebmdm")) {
      if (optind+3  != argc) {printf("dm-unipz: expecting exactly two arguments: ebmdm <mac> <ip>\n"); return 1;} 

      dstMac = strtoull(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("dm-unipz: invalid mac -- %s\n", argv[optind+2]); return 1;} 

      ip = strtoull(argv[optind+2], &tail, 0);
      if (*tail != 0)        {printf("dm-unipz: invalid ip -- %sn", argv[optind+3]); return 1;}       

      dstMacHi = (uint32_t)(dstMac >> 32);
      dstMacLo = (uint32_t)(dstMac & 0xffffffff);

      eb_device_write(device, dmunipz_dstMacHi, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)dstMacHi, 0, eb_block); // todo: all writes in one cycle
      eb_device_write(device, dmunipz_dstMacLo, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)dstMacLo, 0, eb_block);
      eb_device_write(device, dmunipz_dstIp,    EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)ip , 0, eb_block);
      printf("dm-unipz: setting will become active upon the next 'configure' command\n");
    } // "ebmdm"
    if (!strcasecmp(command, "flex")) {
      if (optind+2  != argc) {printf("dm-unipz: expecting exactly one argument: flex <offset>\n"); return 1;} 

      flexOffset = strtoul(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("dm-unipz: invalid offset -- %s\n", argv[optind+2]); return 1;} 

      eb_device_write(device, dmunipz_flexOffset, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)flexOffset, 0, eb_block);
      printf("dm-unipz: setting will become active upon the next 'configure' command\n");
    } // "flex"
    if (!strcasecmp(command, "uni")) {
      if (optind+2  != argc) {printf("dm-unipz: expecting exactly one argument: uni <timeout>\n"); return 1;} 

      uniTimeout = strtoul(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("dm-unipz: invalid timeout -- %s\n", argv[optind+2]); return 1;} 

      eb_device_write(device, dmunipz_uniTimeout, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)uniTimeout, 0, eb_block);
      printf("dm-unipz: setting will become active upon the next 'configure' command\n");
    } // "uni"
    if (!strcasecmp(command, "tk")) {
      if (optind+2  != argc) {printf("dm-unipz: expecting exactly one argument: tk <timeout>\n"); return 1;} 

      tkTimeout = strtoul(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("dm-unipz: invalid timeout -- %s\n", argv[optind+2]); return 1;} 

      eb_device_write(device, dmunipz_tkTimeout, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)tkTimeout, 0, eb_block);
      printf("dm-unipz: setting will become active upon the next 'configure' command\n");
    } // "tk"
    if (!strcasecmp(command, "reltk"))   eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)DMUNIPZ_CMD_RELEASETK,   0, eb_block);
    if (!strcasecmp(command, "relbeam")) eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)DMUNIPZ_CMD_RELEASEBEAM, 0, eb_block);
  } //if command
  

  if (snoop) {
    printf("dm-unipz: continous monitoring of gateway, loglevel = %d\n", logLevel);
    
    actTransfer    = 0;
    actInjection   = 0;
    actState       = COMMON_STATE_UNKNOWN;
    actStatusArray = 0x1 << COMMON_STATUS_OK;

#ifdef USEMASP 
    // optional: disable masp logging (default: log to stdout, can be customized)
    MASP::no_logger no_log;
    MASP::Logger::middleware_logger = &no_log;

    MASP::StatusEmitter emitter(get_config());
    std::cout << "dm-unipz: emmitting to MASP as sourceId: " << maspSourceId << ", using nomen: " << maspNomen << ", environment pro: " << maspProductive << std::endl;
#endif // USEMASP
    
    printTransferHeader();

    while (1) {
      comlib_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, &nLate, &offsDone, &comLatency, &usedSize, 0);
      readInfo(&iterations, &virtAccReq, &virtAccRec, &noBeam, &dtStart, &dtSync1, &dtSync2, &dtInject, &dtTransfer, &dtTkreq, &dtBreq, &dtBprep, &dtReady2Sis, &nR2sTransfer, &nR2sCycle, &nBooster);

      switch(state) {
      case COMMON_STATE_OPREADY :
        if (actTransfer != nTransfer) sleepTime = COMMON_DEFAULT_TIMEOUT * 1000 * 2;        // ongoing transfer: reduce polling rate ...
        else                          sleepTime = COMMON_DEFAULT_TIMEOUT * 1000;            // sleep for default timeout to catch next REQ_TK
        break;
      default:
        sleepTime = COMMON_DEFAULT_TIMEOUT * 1000;                          
      } // switch actState
      
      // if required, print status change
      if  ((actState != state) && (logLevel <= COMMON_LOGLEVEL_STATE)) printFlag = 1;

      // determine when to print info
      printFlag = 0;

      if ((actState != state)  && (logLevel <= COMMON_LOGLEVEL_STATE)) {
        printFlag = 1; actState = state;
      } // if actstate
      if ((actStatusArray != statusArray) && (logLevel <= COMMON_LOGLEVEL_STATUS)) {
        printFlag      = 1;
        actStatusArray = statusArray;
      } // if actstatus
      if ((actTransfer  != nTransfer) && (logLevel <= COMMON_LOGLEVEL_ONCE) && (statTrans & (0x1 << DMUNIPZ_TRANS_RELTK))) {
        printFlag    = 1;
        actTransfer  = nTransfer;
      } // if acttransfer
      if (((actTransfer != nTransfer) || ((actInjection != nInjection) && (statTrans & (0x1 << DMUNIPZ_TRANS_RELBEAM)))) && (logLevel <= COMMON_LOGLEVEL_ALL)) {
        printFlag    = 1;
        actTransfer  = nTransfer;
        actInjection = nInjection;
      } // if ....

      if (printFlag) {
        printTransfer(nTransfer, nInjection, virtAccReq, virtAccRec, noBeam, dtStart, dtSync1, dtSync2, dtInject, dtTransfer, dtTkreq, dtBreq, dtBprep, dtReady2Sis, nR2sTransfer, nR2sCycle, nBooster, statTrans); 
        printf(", %s (%6u), ",  comlib_stateText(state), nBadState);
        if ((statusArray >> COMMON_STATUS_OK) & 0x1) printf("OK   (%6u)\n", nBadStatus);
        else printf("NOTOK(%6u)\n", nBadStatus);
        // print set status bits (except OK)
        for (i= COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
          if ((statusArray >> i) & 0x1)  printf("  ------ status bit is set : %s\n", dmunipz_statusText(i));
        } // for i
      } // if printFlag

      fflush(stdout);                                                                         // required for immediate writing (if stdout is piped to syslog)

#ifdef USEMASP
      if (actState  == COMMON_STATE_OPREADY) maspSigOpReady  = true;
      else                                   maspSigOpReady  = false;

      maspSigTransfer = true;   // ok, this is dummy for now, e.g. in case of MIL troubles or so
      
      // use masp end of scope emitter
      {  
        MASP::End_of_scope_status_emitter scoped_emitter(maspNomen, emitter);
        scoped_emitter.set_OP_READY(maspSigOpReady);
        // scoped_emitter.set_custom_status(DMUNIPZ_MASP_CUSTOMSIG, maspSigTransfer); disabled as our boss did not like it
      } // <--- status is send when the End_of_scope_emitter goes out of scope  
#endif

      //sleep 
      usleep(sleepTime);
    } // while
  } // if snoop

  // close Etherbone device and socket
  if ((eb_status = eb_device_close(device)) != EB_OK) die("eb_device_close", eb_status);
  if ((eb_status = eb_socket_close(socket)) != EB_OK) die("eb_socket_close", eb_status);

  return exitCode;
}
