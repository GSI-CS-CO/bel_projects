#ifndef _DMUNIPZ__REGS_
#define _DMUNIPZ_REGS_

#include "dmunipz_shared_mmap.h"

#define DMUNIPZ_SHARED_DATA_4EB_SIZE  68     // size of shared memory used to receive EB return values; size is in bytes

/* the following values must be added to the offset of the shared memory */
#define DMUNIPZ_SHARED_STATUS         0x00   // error status                       
#define DMUNIPZ_SHARED_CMD            0x04   // input of 32bit command
#define DMUNIPZ_SHARED_STATE          0x08   // state of state machine
#define DMUNIPZ_SHARED_NITERMAIN      0x0C   // # of iterations of main loop
#define DMUNIPZ_SHARED_VERSION        0x10   // version of firmware
#define DMUNIPZ_SHARED_SRCMACHI       0x14   // WR MAC of dmunipz, bits 31..16 unused
#define DMUNIPZ_SHARED_SRCMACLO       0x18   // WR MAC of dmunipz
#define DMUNIPZ_SHARED_SRCIP          0x1C   // IP of dmunipz
#define DMUNIPZ_SHARED_DSTMACHI       0x20   // WR MAC of data master, bits 31..16 unused
#define DMUNIPZ_SHARED_DSTMACLO       0x24   // WR MAC of data master
#define DMUNIPZ_SHARED_DSTIP          0x28   // IP of data master
#define DMUNIPZ_SHARED_OFFSETFLEX     0x2C   // TS_FLEXWAIT = OFFSETFLEX + TS_EVT_READY_TO_SIS; value in ns
#define DMUNIPZ_SHARED_UNITIMEOUT     0x30   // timeout for UNILAC
#define DMUNIPZ_SHARED_TKTIMEOUT      0x34   // timeout for TK (via UNILAC)
#define DMUNIPZ_SHARED_NBADSTATUS     0x38   // # of bad status (=error) incidents
#define DMUNIPZ_SHARED_NBADSTATE      0x3C   // # of bad state (=FATAL, ERROR, UNKNOWN) incidents
#define DMUNIPZ_SHARED_TRANSN         0x40   // # N of transfers
#define DMUNIPZ_SHARED_INJECTN        0x44   // # N of injections (of current transfer)
#define DMUNIPZ_SHARED_TRANSVIRTACC   0x48   // # requested virtual accelerator 0..F
#define DMUNIPZ_SHARED_TRANSSTATUS    0x4C   // # status of transfer
#define DMUNIPZ_SHARED_TRANSNOBEAM    0x50   // # UNILAC requested without beam
#define DMUNIPZ_SHARED_RECVIRTACC     0x54   // # last 2 digits: received virtual accelerator 0..F from UNIPZ, leading digits: number of received MIL events
#define DMUNIPZ_SHARED_DTSTART        0x58   // difference between actual time and flextime @ DM
#define DMUNIPZ_SHARED_DTSYNC         0x5C   // time difference between EVT_READY_TO_SIS and EVT_MB_TRIGGER; value in ns
#define DMUNIPZ_SHARED_DTINJECT       0x60   // time difference between CMD_UNI_BREQ and EVT_MB_TRIGGER; value in ns
#define DMUNIPZ_SHARED_DTTRANSFER     0x64   // time difference between CMD_UNI_TKREQ and EVT_MB_TRIGGER; value in ns
#define DMUNIPZ_SHARED_DTTKREQ        0x68   // time difference between CMD_UNI_TKREQ and reply from UNIPZ
#define DMUNIPZ_SHARED_DTBREQ         0x6c   // time difference between CMD_UNI_BREQ and reply from UNIPZ
#define DMUNIPZ_SHARED_DTREADY2SIS    0x70   // time difference between CMD_UNI_BREQ and EVT_READY_TO_SIS
#define DMUNIPZ_SHARED_NR2STRANSFER   0x74   // # of EVT_READY_TO_SIS events in between CMD_UNI_TKREQ and CMD_UNI_TKREL
#define DMUNIPZ_SHARED_NR2SCYCLE      0x78   // # of EVT_READY_TO_SIS events in between CMD_UNI_TKREL and the following CMD_UNI_TKREL


// 0x7C-0x7F: reserved
#define DMUNIPZ_SHARED_DATA_4EB_START 0x80    // start of shared memory for EB return values
#define DMUNIPZ_SHARED_DATA_4EB_END   DMUNIPZ_SHARED_DATA_4EB_START + DMUNIPZ_SHARED_DATA_4EB_SIZE // end of shared memory area for EB return values

#endif
