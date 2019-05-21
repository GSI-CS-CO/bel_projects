#ifndef _B2B_COMMON_
#define _B2B_COMMON_

// !!!!!
// experimental: let's try to use common defines used by many lm32 projects
// !!!!!

// this file is structured in two parts
// 1st: definitions of general things like error messages
// 2nd: definitions for data exchange via DP RAM

// ****************************************************************************************
// general things
// ****************************************************************************************
#define  COMMON_US_ASMNOP        31           // # of asm("nop") operations per microsecond
#define  COMMON_MS_ASMNOP        31 * 1000    // # of asm("nop") operations per microsecond
#define  COMMON_DEFAULT_TIMEOUT  100          // default timeout used by main loop [ms]
#define  COMMON_ECATIMEOUT       1            // timeout for querying ECA action
#define  COMMON_MILTIMEOUT       100          // timeout for querying MIL events
#define  COMMON_AHEADT           500000       // ahead interval for sending timing messages [ns]
#define  COMMON_ECA_ADDRESS      0x7ffffff0   // address of ECA input
#define  COMMON_EB_HACKISH       0x12345678   // value for EB read handshake

// (error) status, each status will be represented by one bit, bits will be ORed into a 32bit word
#define  COMMON_STATUS_OK                 0    // OK
#define  COMMON_STATUS_ERROR              1    // an error occured
#define  COMMON_STATUS_TIMEDOUT           2    // a timeout occured
#define  COMMON_STATUS_OUTOFRANGE         3    // some value is out of range
#define  COMMON_STATUS_EB                 4    // an Etherbone error occured
#define  COMMON_STATUS_NOIP               5    // DHCP request via WR network failed                                
#define  COMMON_STATUS_EBREADTIMEDOUT     6    // EB read via WR network timed out
#define  COMMON_STATUS_WRBADSYNC          7    // White Rabbit: not in 'TRACK_PHASE'
#define  COMMON_STATUS_AUTORECOVERY       8    // trying auto-recovery from state ERROR
#define  COMMON_STATUS_RESERVEDTILHERE   15    // reserved for common error codes

// commands from the outside
#define  COMMON_CMD_NOCMD                 0    // no command ...
#define  COMMON_CMD_CONFIGURE             1    // configures the gateway
#define  COMMON_CMD_STARTOP               2    // starts operation
#define  COMMON_CMD_STOPOP                3    // stops operation
#define  COMMON_CMD_IDLE                  4    // requests gateway to enter idle state
#define  COMMON_CMD_RECOVER               5    // recovery from error state
#define  COMMON_CMD_CLEARDIAG             6    // reset statistics information
#define  COMMON_CMD_RESERVEDTILHERE      10    // reserved for commmon commands

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

// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// sizes
#define _32b_SIZE_                    4                                                 // size of 32bit value [bytes]
#define COMMON_DATA4EBSIZE           (_32b_SIZE_ * 20)                                  // size of shared memory used to receive EB return values [bytes]

// offsets
// simple values
#define COMMON_SHARED_BEGIN           0x0                                               // begin of used shared memory
#define COMMON_SHARED_SUMSTATUS       COMMON_SHARED_BEGIN                               // error sum status; all actual error bits are ORed into here                      
#define COMMON_SHARED_CMD             (COMMON_SHARED_SUMSTATUS  + _32b_SIZE_)           // input of 32bit command
#define COMMON_SHARED_STATE           (COMMON_SHARED_CMD        + _32b_SIZE_)           // state of state machine
#define COMMON_SHARED_VERSION         (COMMON_SHARED_STATE      + _32b_SIZE_)           // version of firmware
#define COMMON_SHARED_MACHI           (COMMON_SHARED_VERSION    + _32b_SIZE_)           // WR MAC of wrunipz, bits 31..16 unused
#define COMMON_SHARED_MACLO           (COMMON_SHARED_MACHI      + _32b_SIZE_)           // WR MAC of wrunipz
#define COMMON_SHARED_IP              (COMMON_SHARED_MACLO      + _32b_SIZE_)           // IP of wrunipz
#define COMMON_SHARED_NBADSTATUS      (COMMON_SHARED_IP         + _32b_SIZE_)           // # of bad status (=error) incidents
#define COMMON_SHARED_NBADSTATE       (COMMON_SHARED_NBADSTATUS + _32b_SIZE_)           // # of bad state (=FATAL, ERROR, UNKNOWN) incidents
#define COMMON_SHARED_TDIAGHI         (COMMON_SHARED_NBADSTATE  + _32b_SIZE_)           // time when diagnostics was cleared, high bits
#define COMMON_SHARED_TDIAGLO         (COMMON_SHARED_TDIAGHI    + _32b_SIZE_)           // time when diagnostics was cleared, low bits
#define COMMON_SHARED_TS0HI           (COMMON_SHARED_TDIAGLO    + _32b_SIZE_)           // time when FW was in S0 state (start of FW), high bits
#define COMMON_SHARED_TS0LO           (COMMON_SHARED_TS0HI      + _32b_SIZE_)           // time when FW was in S0 state (start of FW), low bits

// shared memory for EB return values
#define COMMON_SHARED_DATA_4EB        (COMMON_SHARED_TS0LO      + _32b_SIZE_)           // shared area for EB return values
#define COMMON_SHARED_END             (COMMON_SHARED_DATA_4EB   + (COMMON_DATA4EBSIZE << 2)) // here the common part of the shared memory ends


// ****************************************************************************************
// public routines
// ****************************************************************************************

// get my own MAC
uint64_t common_wrGetMac();

// intialize Etherbone master
// uint32_t common_ebmInit(uint32_t msTimeout, uint64_t dstMac, uint32_t dstIp, uint32_t eb_ops);

// write timing message
uint32_t common_ebmWriteTM(uint64_t deadline, uint64_t evtId, uint64_t param);

//find WB address of WR Endpoint
//uint32_t findWREp();

// 1. query ECA for actions, 2. trigger activity
uint32_t common_wait4ECAEvent(uint32_t msTimeout, uint64_t *deadline, uint64_t *param, uint32_t *isLate);

// wait for MIL event or timeout
uint32_t common_wait4MILEvent(uint32_t *evtData, uint32_t *evtCode, uint32_t *virtAcc, uint32_t *validEvtCodes, uint32_t nValidEvtCodes, uint32_t msTimeout);

// pulse lemo for debugging with scope
void common_milPulseLemo(uint32_t nLemo);

// initialize common stuff and WB slave addresses
void common_init(uint32_t *startShared, uint32_t fwVersion);

// init stuff for handling commands, trivial for now, will be extended
void common_initCmds();

// clears all statistics
void common_clearDiag();

// do action S0 state
uint32_t common_doActionS0();

// get address of MIL piggy
volatile uint32_t * common_getMilPiggy();

// acquire and publish NIC data
void common_publishNICData();

// publish state
void common_publishState(uint32_t state);

// publish sum status
void common_publishSumStatus(uint32_t sumStatus);

// publish number of bad status incidents
void common_incBadStatusCnt();

// publish number of bad state incidents
void common_incBadStateCnt();

// handle commands from the outside world
void common_cmdHandler(uint32_t *reqState, uint32_t *cmd);

// do state specific action
uint32_t common_doActionState(uint32_t *reqState, uint32_t actState, uint32_t status);

// set gate of LVDS input
uint32_t common_ioCtrlSetGate(uint32_t enable, uint32_t io);

// state machine
uint32_t common_changeState(uint32_t *actState, uint32_t *reqState, uint32_t actStatus);

// do autorecovery from error state
void common_doAutoRecovery(uint32_t actState, uint32_t *reqState);

// intialize Etherbone master
uint32_t common_ebmInit(uint32_t msTimeout, uint64_t dstMac, uint32_t dstIp, uint32_t eb_ops);

#endif
