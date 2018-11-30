#ifndef __WR_UNIPZ_H_
#define __WR_UNIPZ_H_

#define  WRUNIPZ_US_ASMNOP        31          // # of asm("nop") operations per microsecond
#define  WRUNIPZ_MS_ASMNOP        31 * 1000   // # of asm("nop") operations per microsecond
#define  WRUNIPZ_DEFAULT_TIMEOUT  100         // default timeout used by main loop [ms]
#define  WRUNIPZ_QUERYTIMEOUT     1           // timeout for querying virt acc from MIL Piggy FIFO [ms] 
                                              // Ludwig: we have 10ms time; here: use 5 ms to be on the safe side
#define  WRUNIPZ_MATCHWINDOW       200000     // used for comparing timestamps: 1 TS from TLU->ECA matches event from MIL FIFO, 2: synch EVT_MB_TRIGGER, ...
#define  WRUNIPZ_EVT_DUMMY        0x43        // event number chk !!!
#define  WRUNIPZ_ECA_ADDRESS      0x7ffffff0  // address of ECA input
#define  WRUNIPZ_EB_HACKISH       0x12345678  // value for EB read handshake

// (error) status
#define  WRUNIPZ_STATUS_UNKNOWN          0    // unknown status
#define  WRUNIPZ_STATUS_OK               1    // OK
#define  WRUNIPZ_STATUS_ERROR            2    // an error occured
#define  WRUNIPZ_STATUS_TIMEDOUT         3    // a timeout occured
#define  WRUNIPZ_STATUS_OUTOFRANGE       4    // some value is out of range
#define  WRUNIPZ_STATUS_NOIP            13    // DHCP request via WR network failed                                
#define  WRUNIPZ_STATUS_EBREADTIMEDOUT  16    // EB read via WR network timed out
#define  WRUNIPZ_STATUS_WRONGVIRTACC    17    // received EVT_READY_TO_SIS with wrong virt acc number
#define  WRUNIPZ_STATUS_SAFETYMARGIN    18    // violation of safety margin for data master and timing network
#define  WRUNIPZ_STATUS_NOTIMESTAMP     19    // received EVT_READY_TO_SIS in MIL FIFO but not via TLU -> ECA
#define  WRUNIPZ_STATUS_BADTIMESTAMP    20    // TS from TLU->ECA does not coincide with MIL Event from FIFO
#define  WRUNIPZ_STATUS_WAIT4UNIEVENT   26    // timeout while waiting for EVT_READY_TO_SIS
#define  WRUNIPZ_STATUS_WRBADSYNC       30    // White Rabbit: not in 'TRACK_PHASE'
#define  WRUNIPZ_STATUS_AUTORECOVERY    31    // trying auto-recovery from state ERROR



// MASP chk, we need this later probably
// #define  WRUNIPZ_MASP_NOMEN      "U_DM_UNIPZ" // nomen for gateway      
// #define  WRUNIPZ_MASP_CUSTOMSIG  "TRANSFER"   // custom signal for MASP

                                
// commands from the outside
#define  WRUNIPZ_CMD_NOCMD        0           // no command ...
#define  WRUNIPZ_CMD_CONFIGURE    1           // configures the gateway
#define  WRUNIPZ_CMD_STARTOP      2           // starts operation
#define  WRUNIPZ_CMD_STOPOP       3           // stops operation
#define  WRUNIPZ_CMD_IDLE         4           // requests gateway to enter idle state
#define  WRUNIPZ_CMD_RECOVER      5           // recovery from error state

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
#define  WRUNIPZ_ECADO_DUMMY      2           // chk some activity requested ...

// define log levels for print statemens
#define WRUNIPZ_LOGLEVEL_ALL      0           // info on every UNILAC cycles
#define WRUNIPZ_LOGLEVEL_SECOND   1           // summary info once per second
#define WRUNIPZ_LOGLEVEL_STATUS   2           // info on status changes, info on state changes
#define WRUNIPZ_LOGLEVEL_STATE    3           // info on state changes

#define WRUNIPZ_NEVTMAX          32           // maximum number of events per virtAcc and cycle
#define WRUNIPZ_NVIRTACC         16           // maximum number of virtual Accelerators
#define WRUNIPZ_NPZ               7           // maximum number of Pulszentralen
uint32_t gid[] =                 {1000, 1001, 1002, 1003, 1004, 1005, 1006};

typedef struct dataTable {                    // table with _one_ virtAcc for _one_ Pulszentrale
  uint32_t validFlags;                        // if bit 'n' is set, data of element 'n' is valid
  uint32_t prepFlags;                         // if bit 'n' is set, data of element 'n' are prep data
  uint32_t evtFlags;                          // if bit 'n' is set, data of element 'n' are event data  
  uint32_t data[WRUNIPZ_NEVTMAX];             // bits 0..7 'event code', bits 8..15 'data', bits 16..31 offset [us]
} dataTable;

#endif
