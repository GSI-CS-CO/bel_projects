#ifndef _COMMON_FWLIB_
#define _COMMON_FWLIB_

// typedef for treating sub-ns timestamps within the b2b firmware
// the picosecond part may exceed +-1000, but it is recommended to
// use fwlib_cleanB2bt for alignment 
typedef struct{                                      
  uint64_t ns;                                        // full nanoseconds of time
  int32_t  ps;                                        // ps fraction of time, should be positive
  uint32_t dps;                                       // uncertainty [ps]
} b2bt_t;

// adjusts ns such, that ps part remains small 
b2bt_t fwlib_cleanB2bt(b2bt_t t_ps                    // time [ps]
                       );


// project time t1 [ns] to approximately t2 [ns] in multiples of period T [as]; returns projected time [ns]
uint64_t fwlib_advanceTime(uint64_t t1,               // time 1 [ns]
                           uint64_t t2,               // time 2 [ns], where t2 > t1
                           uint64_t T_as              // period T [as]
                           );

// project time t1 [ps] to approximately t2 [ps] in multiples of period T [as]; returns projected time [ps]
b2bt_t fwlib_advanceTimePs(b2bt_t    t1_ps,           // time 1 [ps]
                           b2bt_t    t2_ps,           // time 2 [ps], where t2 > t1
                           uint64_t T_as              // period T [as]
                           );

// convert [ns, float] to [ps], returns t [ps]
b2bt_t fwlib_tfns2tps(float t_ns                      // time [ns]
                      );

// convert [ps] to [ns, float], returns t [ns, float]
float fwlib_tps2tfns(b2bt_t t_ps                      // time [ps]
                     );

// convert [ns] to [ps], returns t [ps]
b2bt_t fwlib_tns2tps(uint64_t t_ns                    // time [ns]
                     );

// convert [ps] to [ns], returns t [ns]
uint64_t fwlib_tps2tns(b2bt_t t_ps                    // time [ps]
                       );

// get my own MAC, returns MAC
uint64_t fwlib_wrGetMac();

//check status of White Rabbit, returns (error) status
uint32_t fwlib_wrCheckSyncState();

//find WB address of WR Endpoint
//uint32_t findWREp();

// 1. query ECA for actions, 2. trigger activity, returns ECA action (=tag, a value of '0' is reserved for signaling a timeout
uint32_t fwlib_wait4ECAEvent(uint32_t usTimeout,      // timeout [us]
                             uint64_t *deadline,      // deadline of action
                             uint64_t *evtId,         // event ID
                             uint64_t *param,         // parameter field
                             uint32_t *tef,           // TEF field
                             uint32_t *isLate,        // flag 'late'
                             uint32_t *isEarly,       // flag 'early'
                             uint32_t *isConflict,    // flag 'conflict'
                             uint32_t *isDelayed      // flag 'delayed'
                             );

// 1. query ECA for actions, 2. trigger activity, returns ECA action (=tag, a value of '0' is reserved for signaling a timeout
uint32_t fwlib_wait4ECAEvent2(uint32_t usTimeout,      // timeout [us]
                              uint64_t *deadline,      // deadline of action
                              uint64_t *evtId,         // event ID
                              uint64_t *param,         // parameter field
                              uint32_t *tef,           // TEF field
                              uint32_t *isLate,        // eca flag 'late'
                              uint32_t *isEarly,       // eca flag 'early'
                              uint32_t *isConflict,    // eca flag 'conflict'
                              uint32_t *isDelayed,     // eca flag 'delayed
                              uint32_t *isMissed,      // flag deadline < start wait4eca
                              uint32_t *offsMissed,    // if 'missed': offset deadline to start wait4eca; else '0'
                              uint32_t *comLatency     // if 'missed': offset start to stop wait4eca; else deadline to stop wait4eca
                              );

// wait for MIL event or timeout, returns (error) status
uint32_t fwlib_wait4MILEvent(uint32_t usTimeout,      // timeout [us]
                             uint32_t *evtData,       // data received with event (4 relevant bits)
                             uint32_t *evtCode,       // event code (8 relevant bits)
                             uint32_t *virtAcc,       // virtual accelerator (4 relevant bits)
                             uint32_t *validEvtCodes, // array of valid event codes the routine will listen to
                             uint32_t nValidEvtCodes  // # of valid event codes; in case n==0, the routine will return with the first event received
                             );

// pulse lemo for debugging with scope
void fwlib_milPulseLemo(uint32_t nLemo                // # of lemo output (starting at '1')
                        );

// initialize fwlib stuff and WB slave addresses
// IMPORTANT: *cpuRamExternal must be set prior calling this routine; typically this is done in routine initShared() of the main program
void fwlib_init(uint32_t *startShared,                // start of shared section (external view)
                uint32_t *cpuRamExternal,             // lm32 RAM (external view)
                uint32_t sharedOffs,                  // offset for shared section
                uint32_t sharedSize,                  // total size of DP RAM 
                char*    name,                        // firmware name
                uint32_t fwVersion                    // firmware version
                );

// publish (used) size of shared memory
void fwlib_publishSharedSize(uint32_t size            // used size of shared memory
                             );

// init stuff for handling commands, trivial for now, will be extended
void fwlib_initCmds();

// clears all statistics
void fwlib_clearDiag();

// do action S0 state, returns (error) status
uint32_t fwlib_doActionS0();

// get address of MIL piggy
volatile uint32_t * fwlib_getMilPiggy();

// get address of OLED
volatile uint32_t * fwlib_getOLED();

// get WB address of SCU bus master
volatile uint32_t * fwlib_getSbMaster();

// acquire and publish NIC data
void fwlib_publishNICData();

// publish state
void fwlib_publishState(uint32_t state                 // state
                        );

// publish status array
void fwlib_publishStatusArray(uint64_t statusArray     // status array (each bit represents one status information)
                              );

// publish status of ongoing transfer
void fwlib_publishTransferStatus(uint32_t nTransfer,   // # of transfers;                          PSM: # of phase shifts @ SIS18
                                 uint32_t nInject,     // # of injections within current transfer; PSM: # of phase shifts @ ESR or CRYRING
                                 uint32_t transStat,   // status of ongoing transfer;              PSM: # of phase shifts @ SIS100
                                 uint32_t nLate,       // number of messages that could not be delivered in time
                                 uint32_t offsDone,    // offset event deadline to time when we are done [ns]
                                 uint32_t comLatency   // latency for messages received from via ECA (tDeadline - tNow)) [ns]
                                 );
                              
// publish status of ongoing transfer, extended version
void fwlib_publishTransferStatus2(uint32_t nTransfer,   // # of transfers;                          PSM: # of phase shifts @ SIS18
                                  uint32_t nInject,     // # of injections within current transfer; PSM: # of phase shifts @ ESR or CRYRING
                                  uint32_t transStat,   // status of ongoing transfer;              PSM: # of phase shifts @ SIS100
                                  uint32_t nLate,       // number of ECA 'late' incidents                                            
                                  uint32_t nEarly,      // number of ECA 'early' incidents                                           
                                  uint32_t nConflict,   // number of ECA 'conflict' incidents                                        
                                  uint32_t nDelayed,    // number of ECA 'delayed' incidents                                         
                                  uint32_t nMissed,     // number of incidents, when 'wait4eca' was called after the deadline        
                                  uint32_t offsMissed,  // if 'missed': offset deadline to start wait4eca; else '0'                  
                                  uint32_t comLatency,  // if 'missed': offset start to stop wait4eca; else deadline to stop wait4eca
                                  uint32_t offsDone     // offset event deadline to time when we are done [ns]
                                  );

// publish number of bad status incidents
void fwlib_incBadStatusCnt();

// publish number of bad state incidents
void fwlib_incBadStateCnt();

// handle commands from the outside world
void fwlib_cmdHandler(uint32_t *reqState,              // requested state (just in case) 
                      uint32_t *cmd                    // requested command
                      ); 

// performs state specific action, returns (error) status
uint32_t fwlib_doActionState(uint32_t *reqState,       // requested state (just in case)
                             uint32_t actState,        // actual state
                             uint32_t status           // actual status
                             );

// set gate of LVDS input, returns (error) status
uint32_t fwlib_ioCtrlSetGate(uint32_t enable,          // enable ('1') or disable ('0');
                             uint32_t io);             // # of IO (starting at '0')

// state machine - change state, returns (error) status
uint32_t fwlib_changeState(uint32_t *actState,         // actual statue
                           uint32_t *reqState,         // requested state
                           uint32_t actStatus          // actual status
                           );

// do autorecovery from error state
void fwlib_doAutoRecovery(uint32_t actState,           // handles autorecovery from ERROR state
                          uint32_t *reqState);         // requested state

// intialize Etherbone master, returns (error) status
uint32_t fwlib_ebmInit(uint32_t msTimeout,             // timeout [ms]
                       uint64_t dstMac,                // MAC of destination
                       uint32_t dstIp,                 // IP of destination
                       uint32_t eb_ops                 // see ebm 
                       );


// builds 64 bit EvtId in format v1, see https://www-acc.gsi.de/wiki/Timing/TimingSystemEvent
uint64_t fwlib_buildEvtidV1(uint32_t gid,              // group ID
                            uint32_t evtno,            // event number
                            uint32_t flags,            // flags
                            uint32_t sid,              // sequence ID
                            uint32_t bpid,             // beam process ID
                            uint32_t reserved          // reserved
                            );

// write timing message to input of ECA
// deadline must be a least (COMMON_LATELIMIT) in the future
// if not, the  message will be rescheduled using COMMON_AHEADT
uint32_t fwlib_ecaWriteTM(uint64_t deadline,           // deadline (when action shall be performed)
                          uint64_t evtId,              // event ID
                          uint64_t param,              // parameter field
                          uint32_t tef,                // TEF field
                          uint32_t flagForceLate       // disable rescheduling in case of 'late' deadline
                          );


// write timing message via Etherbone, returns (error) status
// deadline must be a least (COMMON_LATELIMIT) in the future
// if not, the  message will be rescheduled using COMMON_AHEADT
uint32_t fwlib_ebmWriteTM(uint64_t deadline,           // deadline (when action shall be performed)
                          uint64_t evtId,              // event ID
                          uint64_t param,              // parameter field
                          uint32_t tef,                // TEF field
                          uint32_t flagForceLate       // disable rescheduling in case of 'late' deadline
                          );

// write N words using Etherbone, returns (error) status
uint32_t fwlib_ebmWriteN(uint32_t address,             // Wishbone address @ destination
                         uint32_t *data,               // array with data
                         uint32_t n32BitWords          // array size
                         );

// read N words using Etherbone, returns (error) status
uint32_t fwlib_ebmReadN(uint32_t msTimeout,            // timeout [ms]
                        uint32_t address,              // Wishbone address @ destination
                        uint32_t *data,                // array with data
                        uint32_t n32BitWords           // array size
                        );

// print stuff to OLED
void fwlib_printOLED(char *chars                       // text to print
                     );

// clear OLED
void fwlib_clearOLED();

// non-optimed routine for converting single precision to half precision float, IEEE 754
uint16_t fwlib_float2half(float f                      // single precision number
                          );

// non-optimed routine for converting half precision to single precision float, IEEE 754
float fwlib_half2float(uint16_t h                      // half precision number
                       );

#endif 
