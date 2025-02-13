#ifndef _COMMON_DEFS_
#define _COMMON_DEFS_

// !!!!!
// experimental: let's try to use common defines used by many lm32 projects
// !!!!!

// this file is structured in two parts
// 1st: definitions of general things like error messages
// 2nd: definitions for data exchange via DP RAM

// ****************************************************************************************
// general things
// ****************************************************************************************
typedef union {                                // easier copying of bytes float from/to int data types
  uint32_t data;
  float    f;
} fdat_t;

#define  COMMON_US_ASMNOP        31           // # of asm("nop") operations per microsecond
#define  COMMON_MS_ASMNOP        31 * 1000    // # of asm("nop") operations per microsecond
#define  COMMON_DEFAULT_TIMEOUT  100          // default timeout used by main loop [ms]
#define  COMMON_ECATIMEOUT       1            // timeout for querying ECA action [ms]
#define  COMMON_MILTIMEOUT       100          // timeout for querying MIL events [ms]
#define  COMMON_AHEADT           500000       // ahead interval for sending timing messages [ns]
#define  COMMON_LATELIMIT        250000       // if below this limit [ns], messages are considered to be delivered 'late'
#define  COMMON_ECA_ADDRESS      0x7ffffff0   // address of ECA input
#define  COMMON_EB_HACKISH       0x12345678   // value for EB read handshake

// (error) status
#define  COMMON_STATUS_OK                 0    // OK
#define  COMMON_STATUS_ERROR              1    // an error occured
#define  COMMON_STATUS_TIMEDOUT           2    // a timeout occured
#define  COMMON_STATUS_OUTOFRANGE         3    // some value is out of range
#define  COMMON_STATUS_EB                 4    // an Etherbone error occured
#define  COMMON_STATUS_NOIP               5    // DHCP request via WR network failed
#define  COMMON_STATUS_WRONGIP            6    // IP received via DHCP does not match local config
#define  COMMON_STATUS_EBREADTIMEDOUT     7    // EB read via WR network timed out
#define  COMMON_STATUS_WRBADSYNC          8    // White Rabbit: not in 'TRACK_PHASE'
#define  COMMON_STATUS_AUTORECOVERY       9    // trying auto-recovery from state ERROR
#define  COMMON_STATUS_LATEMESSAGE       10    // late timing message received
#define  COMMON_STATUS_BADSETTING        11    // bad setting data
#define  COMMON_STATUS_RESERVEDTILHERE   15    // 00..15 reserved for common error codes

// commands from the outside
#define  COMMON_CMD_NOCMD                 0    // no command ...
#define  COMMON_CMD_CONFIGURE             1    // configures the gateway
#define  COMMON_CMD_STARTOP               2    // starts operation
#define  COMMON_CMD_STOPOP                3    // stops operation
#define  COMMON_CMD_IDLE                  4    // requests gateway to enter idle state
#define  COMMON_CMD_RECOVER               5    // recovery from error state
#define  COMMON_CMD_CLEARDIAG             6    // reset statistics information
#define  COMMON_CMD_RESERVEDTILHERE      10    // 0..10 reserved for commmon commands

// states; implicitely, all states may transit to the ERROR or FATAL state
#define  COMMON_STATE_UNKNOWN             0    // unknown state
#define  COMMON_STATE_S0                  1    // initial state -> IDLE (automatic)
#define  COMMON_STATE_IDLE                2    // idle state -> CONFIGURED (by command "configure")
#define  COMMON_STATE_CONFIGURED          3    // configured state -> IDLE ("idle"), CONFIGURED ("configure"), OPREADY ("startop")
#define  COMMON_STATE_OPREADY             4    // gateway in operation -> STOPPING ("stopop")
#define  COMMON_STATE_STOPPING            5    // gateway in operation -> CONFIGURED (automatic)
#define  COMMON_STATE_ERROR               6    // gateway in error -> IDLE ("recover")
#define  COMMON_STATE_FATAL               7    // gateway in fatal error; RIP

// activity requested by ECA Handler, the relevant codes are also used as "tags"
#define  COMMON_ECADO_TIMEOUT             0    // no ECA event within timeout

// define log levels for print statements
#define  COMMON_LOGLEVEL_ALL              0    // info on every incident
#define  COMMON_LOGLEVEL_ONCE             1    // info on every completed transfer
#define  COMMON_LOGLEVEL_STATUS           2    // info on status changes, info on state changes
#define  COMMON_LOGLEVEL_STATE            3    // info on state changes

// other
#define COMMON_TIMMSG_LEN                 8    // length of timing message [32bit words]

// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// sizes
#define _32b_SIZE_                    4                                                 // size of 32bit value [bytes]
#define COMMON_DATA4EBSIZE           (_32b_SIZE_ * 20)                                  // size of shared memory used to receive EB return values [bytes]

// offsets
// simple values
#define COMMON_SHARED_BEGIN            0x0                                              // begin of used shared memory
#define COMMON_SHARED_STATUSLO         COMMON_SHARED_BEGIN                              // status array, LO word; all actual error bits are ORed into here
#define COMMON_SHARED_STATUSHI        (COMMON_SHARED_STATUSLO     + _32b_SIZE_)         // status array, HI word
#define COMMON_SHARED_CMD             (COMMON_SHARED_STATUSHI     + _32b_SIZE_)         // input of 32bit command
#define COMMON_SHARED_STATE           (COMMON_SHARED_CMD          + _32b_SIZE_)         // state of state machine
#define COMMON_SHARED_VERSION         (COMMON_SHARED_STATE        + _32b_SIZE_)         // version of firmware
#define COMMON_SHARED_MACHI           (COMMON_SHARED_VERSION      + _32b_SIZE_)         // WR MAC of wrunipz, bits 31..16 unused
#define COMMON_SHARED_MACLO           (COMMON_SHARED_MACHI        + _32b_SIZE_)         // WR MAC of wrunipz
#define COMMON_SHARED_IP              (COMMON_SHARED_MACLO        + _32b_SIZE_)         // IP of wrunipz
#define COMMON_SHARED_NBADSTATUS      (COMMON_SHARED_IP           + _32b_SIZE_)         // # of bad status (=error) incidents
#define COMMON_SHARED_NBADSTATE       (COMMON_SHARED_NBADSTATUS   + _32b_SIZE_)         // # of bad state (=FATAL, ERROR, UNKNOWN) incidents
#define COMMON_SHARED_TDIAGHI         (COMMON_SHARED_NBADSTATE    + _32b_SIZE_)         // time when diagnostics was cleared, high bits
#define COMMON_SHARED_TDIAGLO         (COMMON_SHARED_TDIAGHI      + _32b_SIZE_)         // time when diagnostics was cleared, low bits
#define COMMON_SHARED_TS0HI           (COMMON_SHARED_TDIAGLO      + _32b_SIZE_)         // time when FW was in S0 state (start of FW), high bits
#define COMMON_SHARED_TS0LO           (COMMON_SHARED_TS0HI        + _32b_SIZE_)         // time when FW was in S0 state (start of FW), low bits
#define COMMON_SHARED_NTRANSFER       (COMMON_SHARED_TS0LO        + _32b_SIZE_)         // # of transfers
#define COMMON_SHARED_NINJECT         (COMMON_SHARED_NTRANSFER    + _32b_SIZE_)         // # of injections (within current transfer)
#define COMMON_SHARED_TRANSSTAT       (COMMON_SHARED_NINJECT      + _32b_SIZE_)         // bitwise state of ongoing transfer
#define COMMON_SHARED_NLATE           (COMMON_SHARED_TRANSSTAT    + _32b_SIZE_)         // number of messages that could not be delivered in time
#define COMMON_SHARED_OFFSDONE        (COMMON_SHARED_NLATE        + _32b_SIZE_)         // offset event deadline to time when we are done [ns]
#define COMMON_SHARED_COMLATENCY      (COMMON_SHARED_OFFSDONE     + _32b_SIZE_)         // latency for messages received from via ECA (tDeadline - tNow)) [ns]
#define COMMON_SHARED_DATA_4EB        (COMMON_SHARED_COMLATENCY   + _32b_SIZE_)         // shared area for EB return values
#define COMMON_SHARED_USEDSIZE        (COMMON_SHARED_DATA_4EB     + (COMMON_DATA4EBSIZE << 2))  // used size of shared memory [bytes] /* chk */
#define COMMON_SHARED_END             (COMMON_SHARED_USEDSIZE     + _32b_SIZE_)         // here the common part of the shared memory ends

#endif 
