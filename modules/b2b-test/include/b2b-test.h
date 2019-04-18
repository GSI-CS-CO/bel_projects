#ifndef _B2B_TEST_
#define _B2B_TEST_

// this file is structured in two parts
// 1st: definitions of general things like error messages
// 2nd: definitions for data exchange via DP RAM

// ****************************************************************************************
// general things
// ****************************************************************************************
#define  B2BTEST_US_ASMNOP        31          // # of asm("nop") operations per microsecond
#define  B2BTEST_MS_ASMNOP        31 * 1000   // # of asm("nop") operations per microsecond
#define  B2BTEST_DEFAULT_TIMEOUT  100         // default timeout used by main loop [ms]
#define  B2BTEST_ECATIMEOUT       1           // timeout for querying ECA action 
#define  B2BTEST_ECA_ADDRESS      0x7ffffff0  // address of ECA input
#define  B2BTEST_EB_HACKISH       0x12345678  // value for EB read handshake

// (error) status, each status will be represented by one bit, bits will be ORed into a 32bit word
#define  B2BTEST_STATUS_OK               0    // OK
#define  B2BTEST_STATUS_ERROR            1    // an error occured
#define  B2BTEST_STATUS_TIMEDOUT         2    // a timeout occured
#define  B2BTEST_STATUS_OUTOFRANGE       3    // some value is out of range
#define  B2BTEST_STATUS_PHASEFAILED      4    // phase measurement failed
#define  B2BTEST_STATUS_TRANSACTION      6    // transaction failed
#define  B2BTEST_STATUS_EB               7    // an Etherbone error occured
#define  B2BTEST_STATUS_NOIP            10    // DHCP request via WR network failed                                
#define  B2BTEST_STATUS_EBREADTIMEDOUT  11    // EB read via WR network timed out
#define  B2BTEST_STATUS_SAFETYMARGIN    13    // violation of safety margin for data master and timing network
#define  B2BTEST_STATUS_WRBADSYNC       18    // White Rabbit: not in 'TRACK_PHASE'
#define  B2BTEST_STATUS_AUTORECOVERY    19    // trying auto-recovery from state ERROR
                              
// commands from the outside
#define  B2BTEST_CMD_NOCMD        0           // no command ...
#define  B2BTEST_CMD_CONFIGURE    1           // configures the gateway
#define  B2BTEST_CMD_STARTOP      2           // starts operation
#define  B2BTEST_CMD_STOPOP       3           // stops operation
#define  B2BTEST_CMD_IDLE         4           // requests gateway to enter idle state
#define  B2BTEST_CMD_RECOVER      5           // recovery from error state
#define  B2BTEST_CMD_CLEARDIAG    6           // reset statistics information

// states; implicitely, all states may transit to the ERROR or FATAL state
#define  B2BTEST_STATE_UNKNOWN    0           // unknown state
#define  B2BTEST_STATE_S0         1           // initial state -> IDLE (automatic)
#define  B2BTEST_STATE_IDLE       2           // idle state -> CONFIGURED (by command "configure")
#define  B2BTEST_STATE_CONFIGURED 3           // configured state -> IDLE ("idle"), CONFIGURED ("configure"), OPREADY ("startop")
#define  B2BTEST_STATE_OPREADY    4           // gateway in operation -> STOPPING ("stopop")
#define  B2BTEST_STATE_STOPPING   5           // gateway in operation -> CONFIGURED (automatic)
#define  B2BTEST_STATE_ERROR      6           // gateway in error -> IDLE ("recover")
#define  B2BTEST_STATE_FATAL      7           // gateway in fatal error; RIP                                                             

// activity requested by ECA Handler, the relevant codes are also used as "tags".
#define  B2BTEST_ECADO_TIMEOUT    0           // timeout: no activity requested
#define  B2BTEST_ECADO_UNKOWN     1           // unkown activity requested (unexpected action by ECA)
#define  B2BTEST_ECADO_PHASE      2           // command: perform phase measurement
#define  B2BTEST_ECADO_INPUT      3           // event from input (TLU)

// define log levels for print statements
#define  B2BTEST_LOGLEVEL_ALL     0           // info on every incident
#define  B2BTEST_LOGLEVEL_ONCE    1           // info on every completed transfer
#define  B2BTEST_LOGLEVEL_STATUS  2           // info on status changes, info on state changes
#define  B2BTEST_LOGLEVEL_STATE   3           // info on state changes

// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// sizes
#define _32b_SIZE_                    4                                                 // size of 32bit value [bytes]
#define B2BTEST_DATA4EBSIZE          (_32b_SIZE_ * 20)                                  // size of shared memory used to receive EB return values [bytes]

// offsets
// simple values
#define B2BTEST_SHARED_BEGIN          0x0                                               // begin of used shared memory
#define B2BTEST_SHARED_SUMSTATUS      B2BTEST_SHARED_BEGIN                              // error sum status; all actual error bits are ORed into here                      
#define B2BTEST_SHARED_CMD            (B2BTEST_SHARED_SUMSTATUS  + _32b_SIZE_)          // input of 32bit command
#define B2BTEST_SHARED_STATE          (B2BTEST_SHARED_CMD        + _32b_SIZE_)          // state of state machine
#define B2BTEST_SHARED_VERSION        (B2BTEST_SHARED_STATE      + _32b_SIZE_)          // version of firmware
#define B2BTEST_SHARED_MACHI          (B2BTEST_SHARED_VERSION    + _32b_SIZE_)          // WR MAC of wrunipz, bits 31..16 unused
#define B2BTEST_SHARED_MACLO          (B2BTEST_SHARED_MACHI      + _32b_SIZE_)          // WR MAC of wrunipz
#define B2BTEST_SHARED_IP             (B2BTEST_SHARED_MACLO      + _32b_SIZE_)          // IP of wrunipz
#define B2BTEST_SHARED_NBADSTATUS     (B2BTEST_SHARED_IP         + _32b_SIZE_)          // # of bad status (=error) incidents
#define B2BTEST_SHARED_NBADSTATE      (B2BTEST_SHARED_NBADSTATUS + _32b_SIZE_)          // # of bad state (=FATAL, ERROR, UNKNOWN) incidents
#define B2BTEST_SHARED_NTRANSFER      (B2BTEST_SHARED_NBADSTATE  + _32b_SIZE_)          // # of UNILAC cycles
#define B2BTEST_SHARED_TDIAGHI        (B2BTEST_SHARED_NTRANSFER  + _32b_SIZE_)          // time when diagnostics was cleared, high bits
#define B2BTEST_SHARED_TDIAGLO        (B2BTEST_SHARED_TDIAGHI    + _32b_SIZE_)          // time when diagnostics was cleared, low bits
#define B2BTEST_SHARED_TS0HI          (B2BTEST_SHARED_TDIAGLO    + _32b_SIZE_)          // time when FW was in S0 state (start of FW), high bits
#define B2BTEST_SHARED_TS0LO          (B2BTEST_SHARED_TS0HI      + _32b_SIZE_)          // time when FW was in S0 state (start of FW), low bits


// shared memory for EB return values
#define B2BTEST_SHARED_DATA_4EB       (B2BTEST_SHARED_TS0LO      + _32b_SIZE_)          // shared area for EB return values

// diagnosis: end of used shared memory
#define B2BTEST_SHARED_END            (B2BTEST_SHARED_DATA_4EB   + (B2BTEST_DATA4EBSIZE<< 2)) 

#endif
