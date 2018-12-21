#ifndef _WR_UNIPZ_H_
#define _WR_UNIPZ_H_

// this file is structured in two parts
// 1st: definitions of general things like error messages
// 2nd: definitions for data exchange via DP RAM

// ****************************************************************************************
// general things
// ****************************************************************************************
#define  WRUNIPZ_US_ASMNOP        31          // # of asm("nop") operations per microsecond
#define  WRUNIPZ_MS_ASMNOP        31 * 1000   // # of asm("nop") operations per microsecond
#define  WRUNIPZ_DEFAULT_TIMEOUT  100         // default timeout used by main loop [ms]
#define  WRUNIPZ_QUERYTIMEOUT     1           // timeout for querying virt acc from MIL Piggy FIFO [ms] 
                                              // Ludwig: we have 10ms time; here: use 5 ms to be on the safe side
#define  WRUNIPZ_MILTIMEOUT       5           // timeout for querying MIL event
#define  WRUNIPZ_MATCHWINDOW      200000      // used for comparing timestamps: 1 TS from TLU->ECA matches event from MIL FIFO, 2: synch EVT_MB_TRIGGER, ...
#define  WRUNIPZ_ECA_ADDRESS      0x7ffffff0  // address of ECA input
#define  WRUNIPZ_EB_HACKISH       0x12345678  // value for EB read handshake
#define  WRUNIPZ_UNILACFREQ       50          // frequency of UNILAC operation [Hz]
#define  WRUNIPZ_UNILACPERIOD     20000000    // length of one UNILAC cylce [ns]

// numbers for UNIPZ
#define  WRUNIPZ_NEVT             32          // # of events per virt acc
#define  WRUNIPZ_NVACC            16          // # vAcc
#define  WRUNIPZ_NSET              2          // # of sets for each virt acc ("normal", "verkuerzt")
#define  WRUNIPZ_NPZ               7          // # of Pulszentralen
#define  WRUNIPZ_NFLAG             4          // # flags per virt acc 

// (error) status
#define  WRUNIPZ_STATUS_UNKNOWN          0    // unknown status
#define  WRUNIPZ_STATUS_OK               1    // OK
#define  WRUNIPZ_STATUS_ERROR            2    // an error occured
#define  WRUNIPZ_STATUS_TIMEDOUT         3    // a timeout occured
#define  WRUNIPZ_STATUS_OUTOFRANGE       4    // some value is out of range
#define  WRUNIPZ_STATUS_LATE             5    // a timing messages is not dispatched in time
#define  WRUNIPZ_STATUS_EARLY            6    // a timing messages is dispatched unreasonably early (dt > UNILACPERIOD)
#define  WRUNIPZ_STATUS_TRANSACTION      7    // transaction failed
#define  WRUNIPZ_STATUS_EB               8    // an Etherbone error occured
#define  WRUNIPZ_STATUS_MIL              9    // an error on MIL hardware occured (MIL piggy etc...)
#define  WRUNIPZ_STATUS_NOIP            13    // DHCP request via WR network failed                                
#define  WRUNIPZ_STATUS_EBREADTIMEDOUT  16    // EB read via WR network timed out
#define  WRUNIPZ_STATUS_WRONGVIRTACC    17    // received EVT_READY_TO_SIS with wrong virt acc number
#define  WRUNIPZ_STATUS_SAFETYMARGIN    18    // violation of safety margin for data master and timing network
#define  WRUNIPZ_STATUS_NOTIMESTAMP     19    // received EVT_READY_TO_SIS in MIL FIFO but not via TLU -> ECA
#define  WRUNIPZ_STATUS_BADTIMESTAMP    20    // TS from TLU->ECA does not coincide with MIL Event from FIFO
#define  WRUNIPZ_STATUS_WAIT4UNIEVENT   26    // timeout while waiting for EVT_READY_TO_SIS
#define  WRUNIPZ_STATUS_WRBADSYNC       30    // White Rabbit: not in 'TRACK_PHASE'
#define  WRUNIPZ_STATUS_AUTORECOVERY    31    // trying auto-recovery from state ERROR
                              
// commands from the outside
#define  WRUNIPZ_CMD_NOCMD        0           // no command ...
#define  WRUNIPZ_CMD_CONFIGURE    1           // configures the gateway
#define  WRUNIPZ_CMD_STARTOP      2           // starts operation
#define  WRUNIPZ_CMD_STOPOP       3           // stops operation
#define  WRUNIPZ_CMD_IDLE         4           // requests gateway to enter idle state
#define  WRUNIPZ_CMD_RECOVER      5           // recovery from error state
#define  WRUNIPZ_CMD_CLEARDIAG    6           // reset statistics information
#define  WRUNIPZ_CMD_CONFINIT     7           // init transaction of table data
#define  WRUNIPZ_CMD_CONFSUBMIT   8           // submit data written to DP RAM
#define  WRUNIPZ_CMD_CONFKILL     9           // this will kill an ongoing transaction
#define  WRUNIPZ_CMD_CONFCLEAR   10           // this will clear all event tables
#define  WRUNIPZ_CMD_MODESPZ     11           // mode -> WRUNIPZ_MODE_SPZ
#define  WRUNIPZ_CMD_MODETEST    12           // mode ->  WRUNIPZ_MODE_TEST

// states; implicitely, all states may transit to the ERROR or FATAL state
#define  WRUNIPZ_STATE_UNKNOWN    0           // unknown state
#define  WRUNIPZ_STATE_S0         1           // initial state -> IDLE (automatic)
#define  WRUNIPZ_STATE_IDLE       2           // idle state -> CONFIGURED (by command "configure")
#define  WRUNIPZ_STATE_CONFIGURED 3           // configured state -> IDLE ("idle"), CONFIGURED ("configure"), OPREADY ("startop")
#define  WRUNIPZ_STATE_OPREADY    4           // gateway in operation -> STOPPING ("stopop")
#define  WRUNIPZ_STATE_STOPPING   5           // gateway in operation -> CONFIGURED (automatic)
#define  WRUNIPZ_STATE_ERROR      6           // gateway in error -> IDLE ("recover")
#define  WRUNIPZ_STATE_FATAL      7           // gateway in fatal error; RIP                                                             

// activity requested by ECA Handler, the relevant codes are also used as "tags".
#define  WRUNIPZ_ECADO_TIMEOUT    0           // timeout: no activity requested
#define  WRUNIPZ_ECADO_UNKOWN     1           // unnkown activity requested (unexpected action by ECA)
#define  WRUNIPZ_ECADO_TEST       2           // test mode (internal 50 Hz trigger)
#define  WRUNIPZ_ECADO_MIL        3           // a MIL event was received

// define log levels for print statemens
#define  WRUNIPZ_LOGLEVEL_ALL     0           // info on every UNILAC cycles
#define  WRUNIPZ_LOGLEVEL_SECOND  1           // summary info once per second
#define  WRUNIPZ_LOGLEVEL_STATUS  2           // info on status changes, info on state changes
#define  WRUNIPZ_LOGLEVEL_STATE   3           // info on state changes

#define  WRUNIPZ_CONFSTAT_IDLE    0           // no transaction in progress
#define  WRUNIPZ_CONFSTAT_INIT    1           // transaction of config data has been initialized
#define  WRUNIPZ_CONFSTAT_SUBMIT  2           // config data for transaction has been submitted, waiting for commit event

#define WRUNIPZ_MODE_SPZ          0           // listen to events from Super-UNIPZ
#define WRUNIPZ_MODE_TEST         1           // test mode: 50 Hz clock generated internally

#define WRUNIPZ_EVT_PZ1            1          // next cycle PZ 1
#define WRUNIPZ_EVT_PZ2            2          // next cycle PZ 2
#define WRUNIPZ_EVT_PZ3            3          // next cycle PZ 3
#define WRUNIPZ_EVT_PZ4            4          // next cycle PZ 4
#define WRUNIPZ_EVT_PZ5            5          // next cycle PZ 5
#define WRUNIPZ_EVT_PZ6            6          // next cycle PZ 6
#define WRUNIPZ_EVT_PZ7            7          // next cycle PZ 7
#define WRUNIPZ_EVT_SYNCH_DATA    32          // commit event for transaction
#define WRUNIPZ_EVT_50HZ_SYNCH    33          // 50 Hz trigger, cycle start


typedef struct dataTable {                    // table with _one_ virtAcc for _one_ Pulszentrale
  uint32_t validFlags;                        // if bit 'n' is set, data[n] is valid
  uint32_t prepFlags;                         // if bit 'n' is set, data[n] is prep data
  uint32_t evtFlags;                          // if bit 'n' is set, data[n] is event data  
  uint32_t data[WRUNIPZ_NEVT];                // bits 0..7 'event code', bits 8..15 'data', bits 16..31 offset [us]
} dataTable;

// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// sizes
#define _32b_SIZE_                    4                                                 // size of 32bit value [bytes]
#define WRUNIPZ_DATA4EBSIZE          (_32b_SIZE_ * 20)                                  // size of shared memory used to receive EB return values [bytes]
#define WRUNIPZ_NCONFDATA            (WRUNIPZ_NEVT  * WRUNIPZ_NPZ * WRUNIPZ_NSET)       // # of config data words for one virt acc
#define WRUNIPZ_NCONFFLAG            (WRUNIPZ_NFLAG * WRUNIPZ_NPZ * WRUNIPZ_NSET)       // # of config flag words for one virt acc

// offsets
// simple values
#define WRUNIPZ_SHARED_STATUS         0x0                                               // error status                       
#define WRUNIPZ_SHARED_CMD            (WRUNIPZ_SHARED_STATUS     + _32b_SIZE_)          // input of 32bit command
#define WRUNIPZ_SHARED_STATE          (WRUNIPZ_SHARED_CMD        + _32b_SIZE_)          // state of state machine
#define WRUNIPZ_SHARED_TCYCLEAVG      (WRUNIPZ_SHARED_STATE      + _32b_SIZE_)          // period of UNILAC cycleft [us] (average over one second)
#define WRUNIPZ_SHARED_VERSION        (WRUNIPZ_SHARED_TCYCLEAVG  + _32b_SIZE_)          // version of firmware
#define WRUNIPZ_SHARED_MACHI          (WRUNIPZ_SHARED_VERSION    + _32b_SIZE_)          // WR MAC of wrunipz, bits 31..16 unused
#define WRUNIPZ_SHARED_MACLO          (WRUNIPZ_SHARED_MACHI      + _32b_SIZE_)          // WR MAC of wrunipz
#define WRUNIPZ_SHARED_IP             (WRUNIPZ_SHARED_MACLO      + _32b_SIZE_)          // IP of wrunipz
#define WRUNIPZ_SHARED_NBADSTATUS     (WRUNIPZ_SHARED_IP         + _32b_SIZE_)          // # of bad status (=error) incidents
#define WRUNIPZ_SHARED_NBADSTATE      (WRUNIPZ_SHARED_NBADSTATUS + _32b_SIZE_)          // # of bad state (=FATAL, ERROR, UNKNOWN) incidents
#define WRUNIPZ_SHARED_NCYCLE         (WRUNIPZ_SHARED_NBADSTATE  + _32b_SIZE_)          // # of UNILAC cycles
#define WRUNIPZ_SHARED_NMESSAGEHI     (WRUNIPZ_SHARED_NCYCLE     + _32b_SIZE_)          // # of messsages, high bits
#define WRUNIPZ_SHARED_NMESSAGELO     (WRUNIPZ_SHARED_NMESSAGEHI + _32b_SIZE_)          // # of messsages, low bits
#define WRUNIPZ_SHARED_MSGFREQAVG     (WRUNIPZ_SHARED_NMESSAGELO + _32b_SIZE_)          // message rate (average over one second)
#define WRUNIPZ_SHARED_DTMAX          (WRUNIPZ_SHARED_MSGFREQAVG + _32b_SIZE_)          // delta T max (actTime - deadline)
#define WRUNIPZ_SHARED_DTMIN          (WRUNIPZ_SHARED_DTMAX      + _32b_SIZE_)          // delta T min (actTime - deadline)
#define WRUNIPZ_SHARED_NLATE          (WRUNIPZ_SHARED_DTMIN      + _32b_SIZE_)          // # of late messages
#define WRUNIPZ_SHARED_VACCAVG        (WRUNIPZ_SHARED_NLATE      + _32b_SIZE_)          // virt accs used (past second) bits 0..15 (normal), 16-31 (verkuerzt)
#define WRUNIPZ_SHARED_PZAVG          (WRUNIPZ_SHARED_VACCAVG    + _32b_SIZE_)          // PZ used (past second) bits 0..6
#define WRUNIPZ_SHARED_MODE           (WRUNIPZ_SHARED_PZAVG      + _32b_SIZE_)          // mode (see WRUNIPZ_MODE_...)

// shared memory for EB return values
#define WRUNIPZ_SHARED_DATA_4EB       (WRUNIPZ_SHARED_MODE       + _32b_SIZE_)          // shared area for EB return values

// shared memory for submitting new 'event tables'                                      
#define WRUNIPZ_SHARED_CONF_VACC      (WRUNIPZ_SHARED_DATA_4EB   + WRUNIPZ_DATA4EBSIZE) // vACC for config data
#define WRUNIPZ_SHARED_CONF_STAT      (WRUNIPZ_SHARED_CONF_VACC  + _32b_SIZE_)          // status of config transaction
// config PZ flag layout: least significant bit is PZ0; '1': PZ has new data uploaded
#define WRUNIPZ_SHARED_CONF_PZ        (WRUNIPZ_SHARED_CONF_STAT  + _32b_SIZE_)
// config data layout
// ==================
// note: all config data is valid for the SAME virtual accelerator defined in WRUNIPZ_SHARED_CONF_VACC
// (there are 32 words per virtual accelerator for "normal" and another 32 words for "verkuerzt" operation)
// [norm0 of PZ0]..[norm31 of PZ0][kurz0 of PZ0]..[kurz31 of PZ0][norm0 of PZ1].....[kurz31 of PZ6] 
#define WRUNIPZ_SHARED_CONF_DATA      (WRUNIPZ_SHARED_CONF_PZ    + _32b_SIZE_) 
// config flag layout
// ==================
// (there are 4 words per virtual accelerator for "normal" and another 4 words for "verkuerzt" operation)
// [norm0 of PZ0]..[norm3 of PZ0][kurz0 of PZ0]..[kurz3 of PZ0][norm0 of PZ1].....[kurz32 of PZ6] 
#define WRUNIPZ_SHARED_CONF_FLAG      (WRUNIPZ_SHARED_CONF_DATA  + (WRUNIPZ_NCONFDATA << 2))  

// diagnosis
#define WRUNIPZ_SHARED_SIZEUSED       (WRUNIPZ_SHARED_CONF_FLAG  + (WRUNIPZ_NCONFFLAG << 2)) // used size of shared area

#endif
