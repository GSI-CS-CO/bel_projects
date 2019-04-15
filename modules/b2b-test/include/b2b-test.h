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
#define  B2BTEST_QUERYTIMEOUT     1           // timeout for querying virt acc from MIL Piggy FIFO [ms] 
                                              // Ludwig: we have 10ms time; here: use 5 ms to be on the safe side
#define  B2BTEST_MILTIMEOUT       100         // timeout for querying MIL event
#define  B2BTEST_ECATIMEOUT       1           // timeout for querying ECA action 
#define  B2BTEST_MATCHWINDOW      200000      // used for comparing timestamps: 1 TS from TLU->ECA matches event from MIL FIFO, 2: synch EVT_MB_TRIGGER, ...
#define  B2BTEST_ECA_ADDRESS      0x7ffffff0  // address of ECA input
#define  B2BTEST_EB_HACKISH       0x12345678  // value for EB read handshake
#define  B2BTEST_UNILACFREQ       50          // frequency of UNILAC operation [Hz]
#define  B2BTEST_UNILACPERIOD     20000000    // length of one UNILAC cylce [ns]
#define  B2BTEST_MAXPREPOFFSET    2000        // max offset of a prep event within UNILAC cycle

// numbers for UNIPZ
#define  B2BTEST_NEVT             32          // # of events per virt acc
#define  B2BTEST_NVACC            16          // # vAcc
#define  B2BTEST_NCHN              2          // # of channels (each virtacc may have multiple channels, example: "verkuerzte" operation)
#define  B2BTEST_NPZ               7          // # of Pulszentralen
#define  B2BTEST_NFLAG             4          // # flags per virt acc 

// (error) status, each status will be represented by one bit, bits will be ORed into a 32bit word
#define  B2BTEST_STATUS_OK               0    // OK
#define  B2BTEST_STATUS_ERROR            1    // an error occured
#define  B2BTEST_STATUS_TIMEDOUT         2    // a timeout occured
#define  B2BTEST_STATUS_OUTOFRANGE       3    // some value is out of range
#define  B2BTEST_STATUS_LATE             4    // a timing messages is not dispatched in time
#define  B2BTEST_STATUS_EARLY            5    // a timing messages is dispatched unreasonably early (dt > UNILACPERIOD)
#define  B2BTEST_STATUS_TRANSACTION      6    // transaction failed
#define  B2BTEST_STATUS_EB               7    // an Etherbone error occured
#define  B2BTEST_STATUS_MIL              8    // an error on MIL hardware occured (MIL piggy etc...)
#define  B2BTEST_STATUS_NOMILEVENTS      9    // no MIL events from UNIPZ
#define  B2BTEST_STATUS_NOIP            10    // DHCP request via WR network failed                                
#define  B2BTEST_STATUS_EBREADTIMEDOUT  11    // EB read via WR network timed out
#define  B2BTEST_STATUS_WRONGVIRTACC    12    // received EVT_READY_TO_SIS with wrong virt acc number
#define  B2BTEST_STATUS_SAFETYMARGIN    13    // violation of safety margin for data master and timing network
#define  B2BTEST_STATUS_NOTIMESTAMP     14    // received EVT_READY_TO_SIS in MIL FIFO but not via TLU -> ECA
#define  B2BTEST_STATUS_BADTIMESTAMP    15    // TS from TLU->ECA does not coincide with MIL Event from FIFO
#define  B2BTEST_STATUS_ORDERTIMESTAMP  16    // TS from TLU->ECA and MIL Events are out of order
#define  B2BTEST_STATUS_WAIT4UNIEVENT   17    // timeout while waiting for EVT_READY_TO_SIS
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
#define  B2BTEST_CMD_CONFINIT     7           // init transaction of table data
#define  B2BTEST_CMD_CONFSUBMIT   8           // submit data written to DP RAM
#define  B2BTEST_CMD_CONFKILL     9           // this will kill an ongoing transaction
#define  B2BTEST_CMD_CONFCLEAR   10           // this will clear all event tables
#define  B2BTEST_CMD_MODESPZ     11           // mode -> B2BTEST_MODE_SPZ
#define  B2BTEST_CMD_MODETEST    12           // mode ->  B2BTEST_MODE_TEST

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
#define  B2BTEST_ECADO_UNKOWN     1           // unnkown activity requested (unexpected action by ECA)
#define  B2BTEST_ECADO_TEST       2           // test mode (internal 50 Hz trigger)
#define  B2BTEST_ECADO_MIL        3           // a MIL event was received

// define log levels for print statemens
#define  B2BTEST_LOGLEVEL_ALL     0           // info on every UNILAC cycles
#define  B2BTEST_LOGLEVEL_SECOND  1           // summary info once per second
#define  B2BTEST_LOGLEVEL_STATUS  2           // info on status changes, info on state changes
#define  B2BTEST_LOGLEVEL_STATE   3           // info on state changes

#define  B2BTEST_CONFSTAT_IDLE    0           // no transaction in progress
#define  B2BTEST_CONFSTAT_INIT    1           // transaction of config data has been initialized
#define  B2BTEST_CONFSTAT_SUBMIT  2           // config data for transaction has been submitted, waiting for commit event

#define B2BTEST_MODE_SPZ          0           // listen to events from Super-UNIPZ
#define B2BTEST_MODE_TEST         1           // test mode: 50 Hz clock generated internally

#define B2BTEST_EVT_PZ1           1           // next cycle PZ 1
#define B2BTEST_EVT_PZ2           2           // next cycle PZ 2
#define B2BTEST_EVT_PZ3           3           // next cycle PZ 3
#define B2BTEST_EVT_PZ4           4           // next cycle PZ 4
#define B2BTEST_EVT_PZ5           5           // next cycle PZ 5
#define B2BTEST_EVT_PZ6           6           // next cycle PZ 6
#define B2BTEST_EVT_PZ7           7           // next cycle PZ 7
#define B2BTEST_EVT_SYNCH_DATA   32           // commit event for transaction
#define B2BTEST_EVT_50HZ_SYNCH   33           // 50 Hz trigger, cycle start


typedef struct dataTable {                    // table with _one_ virtAcc for _one_ Pulszentrale
  uint32_t validFlags;                        // if bit 'n' is set, data[n] is valid
  uint32_t prepFlags;                         // if bit 'n' is set, data[n] is prep data
  uint32_t evtFlags;                          // if bit 'n' is set, data[n] is event data  
  uint32_t data[B2BTEST_NEVT];                // bits 0..7 'event code', bits 8..15 'data', bits 16..31 offset [us]
} dataTable;

// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// sizes
#define _32b_SIZE_                    4                                                 // size of 32bit value [bytes]
#define B2BTEST_DATA4EBSIZE          (_32b_SIZE_ * 20)                                  // size of shared memory used to receive EB return values [bytes]
#define B2BTEST_NCONFDATA            (B2BTEST_NEVT  * B2BTEST_NPZ * B2BTEST_NCHN)       // # of config data words for one virt acc
#define B2BTEST_NCONFFLAG            (B2BTEST_NFLAG * B2BTEST_NPZ * B2BTEST_NCHN)       // # of config flag words for one virt acc

// offsets
// simple values
#define B2BTEST_SHARED_BEGIN          0x0                                               // begin of used shared memory
#define B2BTEST_SHARED_SUMSTATUS      B2BTEST_SHARED_BEGIN                              // error sum status; all actual error bits are ORed into here                      
#define B2BTEST_SHARED_CMD            (B2BTEST_SHARED_SUMSTATUS  + _32b_SIZE_)          // input of 32bit command
#define B2BTEST_SHARED_STATE          (B2BTEST_SHARED_CMD        + _32b_SIZE_)          // state of state machine
#define B2BTEST_SHARED_TCYCLEAVG      (B2BTEST_SHARED_STATE      + _32b_SIZE_)          // period of UNILAC cycle [ns] (average over one second)
#define B2BTEST_SHARED_VERSION        (B2BTEST_SHARED_TCYCLEAVG  + _32b_SIZE_)          // version of firmware
#define B2BTEST_SHARED_MACHI          (B2BTEST_SHARED_VERSION    + _32b_SIZE_)          // WR MAC of wrunipz, bits 31..16 unused
#define B2BTEST_SHARED_MACLO          (B2BTEST_SHARED_MACHI      + _32b_SIZE_)          // WR MAC of wrunipz
#define B2BTEST_SHARED_IP             (B2BTEST_SHARED_MACLO      + _32b_SIZE_)          // IP of wrunipz
#define B2BTEST_SHARED_NBADSTATUS     (B2BTEST_SHARED_IP         + _32b_SIZE_)          // # of bad status (=error) incidents
#define B2BTEST_SHARED_NBADSTATE      (B2BTEST_SHARED_NBADSTATUS + _32b_SIZE_)          // # of bad state (=FATAL, ERROR, UNKNOWN) incidents
#define B2BTEST_SHARED_NCYCLE         (B2BTEST_SHARED_NBADSTATE  + _32b_SIZE_)          // # of UNILAC cycles
#define B2BTEST_SHARED_NMESSAGEHI     (B2BTEST_SHARED_NCYCLE     + _32b_SIZE_)          // # of messsages, high bits
#define B2BTEST_SHARED_NMESSAGELO     (B2BTEST_SHARED_NMESSAGEHI + _32b_SIZE_)          // # of messsages, low bits
#define B2BTEST_SHARED_MSGFREQAVG     (B2BTEST_SHARED_NMESSAGELO + _32b_SIZE_)          // message rate (average over one second)
#define B2BTEST_SHARED_DTMAX          (B2BTEST_SHARED_MSGFREQAVG + _32b_SIZE_)          // delta T max (actTime - deadline)
#define B2BTEST_SHARED_DTMIN          (B2BTEST_SHARED_DTMAX      + _32b_SIZE_)          // delta T min (actTime - deadline)
#define B2BTEST_SHARED_NLATE          (B2BTEST_SHARED_DTMIN      + _32b_SIZE_)          // # of late messages
#define B2BTEST_SHARED_VACCAVG        (B2BTEST_SHARED_NLATE      + _32b_SIZE_)          // virt accs used (past second) bits 0..15 (normal), 16-31 (verkuerzt)
#define B2BTEST_SHARED_PZAVG          (B2BTEST_SHARED_VACCAVG    + _32b_SIZE_)          // PZ used (past second) bits 0..6
#define B2BTEST_SHARED_MODE           (B2BTEST_SHARED_PZAVG      + _32b_SIZE_)          // mode (see B2BTEST_MODE_...)
#define B2BTEST_SHARED_TDIAGHI        (B2BTEST_SHARED_MODE       + _32b_SIZE_)          // time when diagnostics was cleared, high bits
#define B2BTEST_SHARED_TDIAGLO        (B2BTEST_SHARED_TDIAGHI    + _32b_SIZE_)          // time when diagnostics was cleared, low bits
#define B2BTEST_SHARED_TS0HI          (B2BTEST_SHARED_TDIAGLO    + _32b_SIZE_)          // time when FW was in S0 state (start of FW), high bits
#define B2BTEST_SHARED_TS0LO          (B2BTEST_SHARED_TS0HI      + _32b_SIZE_)          // time when FW was in S0 state (start of FW), low bits


// shared memory for EB return values
#define B2BTEST_SHARED_DATA_4EB       (B2BTEST_SHARED_TS0LO      + _32b_SIZE_)          // shared area for EB return values

// shared memory for submitting new 'event tables'                                      
#define B2BTEST_SHARED_CONF_VACC      (B2BTEST_SHARED_DATA_4EB   + B2BTEST_DATA4EBSIZE) // vAcc for config data
#define B2BTEST_SHARED_CONF_STAT      (B2BTEST_SHARED_CONF_VACC  + _32b_SIZE_)          // status of config transaction
// config PZ flag layout: least significant bit is PZ0; '1': PZ has new data uploaded
#define B2BTEST_SHARED_CONF_PZ        (B2BTEST_SHARED_CONF_STAT  + _32b_SIZE_)
// config data layout
// ==================
// note: all config data is valid for the SAME virtual accelerator defined in B2BTEST_SHARED_CONF_VACC
// (there are 32 words per virtual accelerator for each "Kanal"
// [data0 of PZ0-chn0]..[data31 of PZ0-chan0][data0 of PZ0-chn1]..[data31 of PZ0-chn1][data0 of PZ1-chn0].....[data31 of PZ6-chn1] 
#define B2BTEST_SHARED_CONF_DATA      (B2BTEST_SHARED_CONF_PZ    + _32b_SIZE_) 
// config flag layout
// ==================
// (there are 4 words per virtual accelerator for each channel
// [flag1 of PZ0-chn0]..[flag3 of PZ0-chn0][flag0 of PZ0-chn1]..[flag3 of PZ0-chn1][flag0 of PZ1-chn0].....[flag3 of PZ6-chn1] 
#define B2BTEST_SHARED_CONF_FLAG      (B2BTEST_SHARED_CONF_DATA  + (B2BTEST_NCONFDATA << 2))  

// diagnosis: end of used shared memory
#define B2BTEST_SHARED_END            (B2BTEST_SHARED_CONF_FLAG  + (B2BTEST_NCONFFLAG << 2)) //

#endif
