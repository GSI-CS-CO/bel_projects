#ifndef _DM_UNIPZ_H_
#define _DM_UNIPZ_H_

#include <common-defs.h>
#include "../../ftm/include/ftm_common.h"     // defs and regs for data master


// dm-unipz specific things
#define  DMUNIPZ_UNITIMEOUT          2000     // timeout used when requesting things from UNILAC [ms]
#define  DMUNIPZ_TKTIMEOUT            210     // timeout used when requesting TK from UNILAC [ms]
#define  DMUNIPZ_QUERYTIMEOUT           1     // timeout for querying virt acc from MIL Piggy FIFO [ms] 
                                              // Ludwig: we have 10ms time;
#define  DMUNIPZ_DMTIMEOUT           9000     // after receiving ReqBeam from DM, this is the amount of time available within we must reply to the DM [us]
#define  DMUNIPZ_MATCHWINDOW       200000     // used for comparing timestamps: 1 TS from TLU->ECA matches event from MIL FIFO, 2: synch EVT_MB_TRIGGER, ...
#define  DMUNIPZ_OFFSETTHRD       1500000     // offset added to obtain TS for the start of the 'injection-thread' at DM [ns]
#define  DMUNIPZ_OFFSETINJECT     9980000     // offset added to obtain expected time of injection [ns], used for diagnostic only
#define  DMUNIPZ_EVT_READY2SIS       0x1e     // event number EVT_READY_TO_SIS (HEX)

// dm-unipz specific commands from the outside
#define  DMUNIPZ_CMD_RELEASETK    6           // release TK request at UNILAC
#define  DMUNIPZ_CMD_RELEASEBEAM  7           // release beam request at UNILAC
#define  DMUNIPZ_CMD_DEBUGON      8           // enables debug mode (writes data to be sent to DM to my own ECA input, experimental!)
#define  DMUNIPZ_CMD_DEBUGOFF     9           // disables debug mode

// (error) status
#define  DMUNIPZ_STATUS_REQTKFAILED     16    // UNILAC refuses TK request
#define  DMUNIPZ_STATUS_REQTKTIMEOUT    17    // UNILAC TK request timed out
#define  DMUNIPZ_STATUS_REQBEAMFAILED   18    // UNILAC refuses beam request
#define  DMUNIPZ_STATUS_RELTKFAILED     19    // UNILAC refuses to release TK request
#define  DMUNIPZ_STATUS_RELBEAMFAILED   20    // UNILAC refuses to release beam request
#define  DMUNIPZ_STATUS_DEVBUSERROR     21    // something went wrong with write/read on the MIL devicebus
#define  DMUNIPZ_STATUS_REQNOTOK        22    // UNILAC signals 'request not ok'                          
#define  DMUNIPZ_STATUS_REQBEAMTIMEDOUT 23    // UNILAC beam request timed out                                
#define  DMUNIPZ_STATUS_WRONGVIRTACC    24    // received EVT_READY_TO_SIS with wrong virt acc number
#define  DMUNIPZ_STATUS_SAFETYMARGIN    25    // violation of safety margin for data master and timing network
#define  DMUNIPZ_STATUS_NOTIMESTAMP     26    // received EVT_READY_TO_SIS in MIL FIFO but not via TLU -> ECA
#define  DMUNIPZ_STATUS_BADTIMESTAMP    27    // TS from TLU->ECA does not coincide with MIL Event from FIFO
#define  DMUNIPZ_STATUS_DMQNOTEMPTY     28    // Data Master: Q not empty
#define  DMUNIPZ_STATUS_LATEEVENT       29    // received 'late event' from Data Master
#define  DMUNIPZ_STATUS_TKNOTRESERVED   30    // TK is not reserved
#define  DMUNIPZ_STATUS_DMTIMEOUT       31    // beam request did not succeed within 10s timeout at DM
#define  DMUNIPZ_STATUS_BADSYNC         32    // t(EVT_MB_TRIGGER) - t(EVT_READY_TO_SIS) != 10ms
#define  DMUNIPZ_STATUS_WAIT4UNIEVENT   33    // timeout while waiting for EVT_READY_TO_SIS
#define  DMUNIPZ_STATUS_BADSCHEDULEA    34    // t(EVT_MB_TRIGGER) - t(CMD_UNI_BREQ) < 10ms
#define  DMUNIPZ_STATUS_BADSCHEDULEB    35    // unexpected event
#define  DMUNIPZ_STATUS_INVALIDBLKADDR  36    // invalid address of block for Data Master
#define  DMUNIPZ_STATUS_NODM            37    // Data Master unreachable
#define  DMUNIPZ_STATUS_INVALIDTHRADDR  38    // invalid address of thread handling area for Data Master
#define  DMUNIPZ_STATUS_DELAYEDEVENT    39    // received 'delayed event'

// MASP 
#define  DMUNIPZ_MASP_NOMEN    "U_DM_UNIPZ"   // nomen for gateway
#define  DMUNIPZ_MASP_CUSTOMSIG  "TRANSFER"   // custom signal for MASP

// activity requested by ECA Handler, the relevant codes are also used as "tags"
#define  DMUNIPZ_ECADO_TIMEOUT    COMMON_ECADO_TIMEOUT
#define  DMUNIPZ_ECADO_UNKOWN     0x1         // unnkown activity requested (unexpected action by ECA)
#define  DMUNIPZ_ECADO_REQTK      0x2         // request the transfer channel (TK), carries info on DM wait after beam request
#define  DMUNIPZ_ECADO_REQBEAM    0x3         // request beam at UNIPZ, terminate waiting block and start thread at DM
#define  DMUNIPZ_ECADO_RELTK      0x4         // release the transfer channel (TK)
#define  DMUNIPZ_ECADO_PREPDM     0x5         // dedicated message from DM, carries info on DM wait after TK request (deprecated)
#define  DMUNIPZ_ECADO_READY2SIS  0x6         // received EVT_READY_TO_SIS via TLU
#define  DMUNIPZ_ECADO_MBTRIGGER  0x7         // received EVT_MB_TRIGGER via TLU
#define  DMUNIPZ_ECADO_PREPBEAM   0x8         // prepare beam at UNIPZ (preceedes 'REQBEAM')
#define  DMUNIPZ_ECADO_REQBEAMNW  0x9         // request beam at UNIPZ, start thread at DM
#define  DMUNIPZ_ECADO_READY2SIS2 0xa         // received EVT_READY_TO_SIS via timing message (intended for testing, don't ask)

// status of transfer (status bits)
#define DMUNIPZ_TRANS_REQTK       0           // TK requested
#define DMUNIPZ_TRANS_REQTKOK     1           // TK request succeeded
#define DMUNIPZ_TRANS_RELTK       2           // TK released
#define DMUNIPZ_TRANS_REQBEAM     3           // beam requested
#define DMUNIPZ_TRANS_REQBEAMOK   4           // beam request succeeded
#define DMUNIPZ_TRANS_RELBEAM     5           // beam released
#define DMUNIPZ_TRANS_PREPBEAM    6           // beam preparation requested
#define DMUNIPZ_TRANS_UNPREPBEAM  7           // beam preparation released


typedef struct {                              // group together all information required for modifying blocks within the data master
  uint32_t dynpar;                            // received from DM: 32 bit of param field
  uint32_t cmdAddr;                           // write to DM: external address of a command
  uint32_t cmdData[_T_CMD_SIZE_];             // write to DM: data of a command
  uint32_t blockWrIdxsAddr;                   // write to DM: external address ofs wrIdxs within block
  uint32_t blockWrIdxs;                       // write to DM: updated value of wrIdxs
} dmComm;


typedef struct {                              // group together all information required for modifying threads with the data master 
  uint32_t dynpar;                            // received from DM: 32bit of param field
  uint32_t cpuIdx;                            // received from  DM: idx of cpu (redundant information)
  uint32_t thrIdx;                            // received from  DM: idx of thread (required to start the thread) 
  uint32_t TSAddr;                            // write to DM: 32bit address of thread start timestamp in the 'thread staging' area
  uint32_t TSData[2];                         // write to DM: start timestamp low and high words
  uint32_t StartAddr;                         // write to DM: 32bit address of thread start bits in the 'Global Thread Control and Status' area
  uint32_t StartData;                         // write to DM: 32bit thread start bits
} dmThrd;


// part below provided by Ludwig Hechler 
#define IFB_ADDRESS_SIS     0x20        /* Adresse der Interfacekarte               */
#define IFB_ADDRESS_UNI     0x10        /* Adresse der Interfacekarte               */

#define IO_MODULE_1            1        /* Adresse I/O-Modul 1                      */
#define IO_MODULE_2            2        /* Adresse I/O-Modul 2                      */
#define IO_MODULE_3            3        /* Adresse I/O-Modul 3                      */

#define IFB_ADR_BUS_W       0x11        /* Funktionscode Adressbus schreiben        */
#define IFB_DATA_BUS_W      0x10        /* Funktionscode Datenbus schreiben         */
#define IFB_DATA_BUS_R      0x90        /* Funktionscode Datenbus lesen             */

#define C_IO32_KANAL_0      0x00        /* Subadresse I/O-Modul fuer I/O-Bits 0..15 */
#define C_IO32_KANAL_1      0x02        /* Subadresse I/O-Modul fuer I/O-Bits 16..31*/


// part below provided by Ludwig Hechler
typedef struct TPZSInfo1 {
  /* Bit 11..15 */  uint16_t free1         : 5;
  /* Bit 10     */  uint16_t Med_MachID_ok : 1;   /* MachineID is ok */
  /* Bit  9     */  uint16_t DTAVal_Ack    : 1;   /* Data valid acknowledge */
  /* Bit  8     */  uint16_t Med_Lock_Sts  : 1;   /* Lock operation status */
  /* Bit  7     */  uint16_t Med_Req_enabl : 1;   /* therapy requests enabled for Unilac */ 
  /* Bit  6     */  uint16_t Req_not_ok    : 1;   /* Unilac PZ signals error for beam request */
  /* Bit  5     */  uint16_t SIS_Req_Ack   : 1;   /* Acknowledge for SIS beam request */
  /* Bit  4     */  uint16_t TK_Req_Ack    : 1;   /* Acknowledge for TK preparation */ 
  /* Bit  0..3  */  uint16_t free2         : 4;
} TPZSInfo1;


typedef struct TPZSInfo5 {
  /* Bit  9..15 */  uint16_t freeIn1       : 7;
  /* Bit  8     */  uint16_t SIS_PrepReq   : 1;   /* beam Request will happen in 100ms */
  /* Bit  7     */  uint16_t ReqNoBeam     : 1;   /* Request accelerator but without beam (timing only) */
  /* Bit  6     */  uint16_t Req_not_ok_Ack: 1;   /* Acknowledge for beam request error */
  /* Bit  5     */  uint16_t SIS_Request   : 1;   /* Request beam for SIS */
  /* Bit  4     */  uint16_t TK_Request    : 1;   /* Request TK preparation */
  /* Bit  0..3  */  uint16_t SIS_Acc_Select: 4;   /* Requested accelerator */
} TPZSInfo5;


// part below provided by Ludwig Hechler
typedef union {
  TPZSInfo5 bits;
  uint16_t uword;
} WriteToPZU_Type;

typedef union {
  TPZSInfo1 bits;
  uint16_t uword;
} ReadFromPZU_Type;

// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// offsets
#define DMUNIPZ_SHARED_SUMSTATUSHI    (COMMON_SHARED_END            + _32b_SIZE_)       // error sum status (ORed bits HI word)
#define DMUNIPZ_SHARED_NITERMAIN      (DMUNIPZ_SHARED_SUMSTATUSHI   + _32b_SIZE_)       // # of iterations of main loop
#define DMUNIPZ_SHARED_DSTMACHI       (DMUNIPZ_SHARED_NITERMAIN     + _32b_SIZE_)       // WR MAC of data master, bits 31..16 unused
#define DMUNIPZ_SHARED_DSTMACLO       (DMUNIPZ_SHARED_DSTMACHI      + _32b_SIZE_)       // WR MAC of data master
#define DMUNIPZ_SHARED_DSTIP          (DMUNIPZ_SHARED_DSTMACLO      + _32b_SIZE_)       // IP of data master
#define DMUNIPZ_SHARED_OFFSETTHRD     (DMUNIPZ_SHARED_DSTIP         + _32b_SIZE_)       // TS_THREADSTART = OFFSETTHRD + TS_EVT_READY_TO_SIS; value in ns
#define DMUNIPZ_SHARED_UNITIMEOUT     (DMUNIPZ_SHARED_OFFSETTHRD    + _32b_SIZE_)       // timeout for UNILAC
#define DMUNIPZ_SHARED_TKTIMEOUT      (DMUNIPZ_SHARED_UNITIMEOUT    + _32b_SIZE_)       // timeout for TK (via UNILAC)
#define DMUNIPZ_SHARED_TRANSVIRTACC   (DMUNIPZ_SHARED_TKTIMEOUT     + _32b_SIZE_)       // # requested virtual accelerator 0..F
#define DMUNIPZ_SHARED_TRANSNOBEAM    (DMUNIPZ_SHARED_TRANSVIRTACC  + _32b_SIZE_)       // # UNILAC requested without beam
#define DMUNIPZ_SHARED_RECVIRTACC     (DMUNIPZ_SHARED_TRANSNOBEAM   + _32b_SIZE_)       // # last 2 digits: received virtual accelerator 0..F from UNIPZ, leading digits: number of received MIL events
#define DMUNIPZ_SHARED_DTSTART        (DMUNIPZ_SHARED_RECVIRTACC    + _32b_SIZE_)       // difference between actual time and start of injection-thread @ DM
#define DMUNIPZ_SHARED_DTSYNC1        (DMUNIPZ_SHARED_DTSTART       + _32b_SIZE_)       // time difference between EVT_READY_TO_SIS and EVT_MB_TRIGGER; value in us
#define DMUNIPZ_SHARED_DTINJECT       (DMUNIPZ_SHARED_DTSYNC1       + _32b_SIZE_)       // time difference between CMD_UNI_BREQ and EVT_MB_TRIGGER; value in us
#define DMUNIPZ_SHARED_DTTRANSFER     (DMUNIPZ_SHARED_DTINJECT      + _32b_SIZE_)       // time difference between CMD_UNI_TKREQ and EVT_MB_TRIGGER; value in us
#define DMUNIPZ_SHARED_DTTKREQ        (DMUNIPZ_SHARED_DTTRANSFER    + _32b_SIZE_)       // time difference between CMD_UNI_TKREQ and reply from UNIPZ; value in us
#define DMUNIPZ_SHARED_DTBREQ         (DMUNIPZ_SHARED_DTTKREQ       + _32b_SIZE_)       // time difference between CMD_UNI_BREQ and reply from UNIPZ; value in us
#define DMUNIPZ_SHARED_DTREADY2SIS    (DMUNIPZ_SHARED_DTBREQ        + _32b_SIZE_)       // time difference between CMD_UNI_BREQ and EVT_READY_TO_SIS; value in us
#define DMUNIPZ_SHARED_NR2STRANSFER   (DMUNIPZ_SHARED_DTREADY2SIS   + _32b_SIZE_)       // # of EVT_READY_TO_SIS events in between CMD_UNI_TKREQ and CMD_UNI_TKREL
#define DMUNIPZ_SHARED_NR2SCYCLE      (DMUNIPZ_SHARED_NR2STRANSFER  + _32b_SIZE_)       // # of EVT_READY_TO_SIS events in between CMD_UNI_TKREL and the following CMD_UNI_TKREL
#define DMUNIPZ_SHARED_DTBPREP        (DMUNIPZ_SHARED_NR2SCYCLE     + _32b_SIZE_)       // time difference between CMD_UNI_BREQ and start of request at UNIPZ; value in us
#define DMUNIPZ_SHARED_NBOOSTER       (DMUNIPZ_SHARED_DTBPREP       + _32b_SIZE_)       // # of booster injections
#define DMUNIPZ_SHARED_DTSYNC2        (DMUNIPZ_SHARED_NBOOSTER      + _32b_SIZE_)       // time difference between EVT_READY_TO_SIS and CMD_UNI_TCREL; value in us

// diagnosis: end of used shared memory
#define DMUNIPZ_SHARED_END            (DMUNIPZ_SHARED_DTSYNC2       + _32b_SIZE_)       // end of shared memory

#endif
