#ifndef _B2B_FWLIB_
#define _B2B_FWLIB_

// !!!!!
// experimental: let's try to use common routines by my (DB) lm32 projects
// !!!!!

// get my own MAC, returns MAC
uint64_t fwlib_wrGetMac();

//check status of White Rabbit, returns (error) status
uint32_t fwlib_wrCheckSyncState();

//find WB address of WR Endpoint
//uint32_t findWREp();

// 1. query ECA for actions, 2. trigger activity, returns (error) status
uint32_t fwlib_wait4ECAEvent(uint32_t msTimeout,      // timeout [ms]
                             uint64_t *deadline,      // deadline of action
                             uint64_t *evtId,         // event ID
                             uint64_t *param,         // parameter field
                             uint32_t *tef,           // TEF filed
                             uint32_t *isLate         // flag 'is late'
                             );

// wait for MIL event or timeout, returns (error) status
uint32_t fwlib_wait4MILEvent(uint32_t msTimeout,      // timeout [ms]
                             uint32_t *evtData,       // data received with event (4 relevant bits)
                             uint32_t *evtCode,       // event code (8 relevant bits)
                             uint32_t *virtAcc,       // virtual accelerator (4 relevant bits)
                             uint32_t *validEvtCodes, // array of valid event codes the routine will listen to
                             uint32_t nValidEvtCodes  // # of valid event codes
                             );

// pulse lemo for debugging with scope
void fwlib_milPulseLemo(uint32_t nLemo                // # of lemo output (starting at '1')
                        );

// initialize fwlib stuff and WB slave addresses
void fwlib_init(uint32_t *startShared,                // start of shared section (external view)
                uint32_t *cpuRamExternal,             // lm32 RAM (external view)
                uint32_t sharedOffs,                  // offset for shared section
                char* name,                           // firmware name
                uint32_t fwVersion                    // firmware version
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

// acquire and publish NIC data
void fwlib_publishNICData();

// publish state
void fwlib_publishState(uint32_t state                 // state
                        );

// publish status array
void fwlib_publishStatusArray(uint64_t statusArray     // status array (each bit represents one status information)
                              );

// publish status of ongoing transfer
void fwlib_publishTransferStatus(uint32_t nTransfer,   // # of transfers
                                 uint32_t nInject,     // # of injections within current transferr
                                 uint32_t transStat    // status of ongoing transfer
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
                             uint32_t io);             // # of IO (starting at '1')

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

// write timing message via Etherbone, returns (error) status
uint32_t fwlib_ebmWriteTM(uint64_t deadline,           // deadline (when action shall be performed)
                          uint64_t evtId,              // event ID
                          uint64_t param               // parameter field
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

#endif 
