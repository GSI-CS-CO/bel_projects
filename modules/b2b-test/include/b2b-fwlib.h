#ifndef _B2B_FWLIB_
#define _B2B_FWLIB_

// !!!!!
// experimental: let's try to use common routines by many lm32 projects
// !!!!!

// get my own MAC, returns MAC
uint64_t fwlib_wrGetMac();

//check status of White Rabbit, returns status
uint32_t fwlib_wrCheckSyncState();

// intialize Etherbone master
uint32_t fwlib_ebmInit(uint32_t msTimeout, uint64_t dstMac, uint32_t dstIp, uint32_t eb_ops);

// write timing message via Etherbone
uint32_t fwlib_ebmWriteTM(uint64_t deadline, uint64_t evtId, uint64_t param);

//find WB address of WR Endpoint
//uint32_t findWREp();

// 1. query ECA for actions, 2. trigger activity
uint32_t fwlib_wait4ECAEvent(uint32_t msTimeout, uint64_t *deadline, uint64_t *evtId, uint64_t *param, uint32_t *tef, uint32_t *isLate);

// wait for MIL event or timeout
uint32_t fwlib_wait4MILEvent(uint32_t msTimeout, uint32_t *evtData, uint32_t *evtCode, uint32_t *virtAcc, uint32_t *validEvtCodes, uint32_t nValidEvtCodes);

// pulse lemo for debugging with scope
void fwlib_milPulseLemo(uint32_t nLemo);

// initialize fwlib stuff and WB slave addresses
void fwlib_init(uint32_t *startShared, uint32_t *cpuRamExternal, uint32_t sharedOffs, char* name,  uint32_t fwVersion);

// init stuff for handling commands, trivial for now, will be extended
void fwlib_initCmds();

// clears all statistics
void fwlib_clearDiag();

// do action S0 state
uint32_t fwlib_doActionS0();

// get address of MIL piggy
volatile uint32_t * fwlib_getMilPiggy();

volatile uint32_t * fwlib_getOLED();

// acquire and publish NIC data
void fwlib_publishNICData();

// publish state
void fwlib_publishState(uint32_t state);

// publish status array
void fwlib_publishStatusArray(uint64_t statusArray);

// publish status of ongoing transfer
void fwlib_publishTransferStatus(uint32_t nTransfer, uint32_t nInject, uint32_t transStat);

// publish number of bad status incidents
void fwlib_incBadStatusCnt();

// publish number of bad state incidents
void fwlib_incBadStateCnt();

// handle commands from the outside world
void fwlib_cmdHandler(uint32_t *reqState, uint32_t *cmd);

// do state specific action
uint32_t fwlib_doActionState(uint32_t *reqState, uint32_t actState, uint32_t status);

// set gate of LVDS input
uint32_t fwlib_ioCtrlSetGate(uint32_t enable, uint32_t io);

// state machine
uint32_t fwlib_changeState(uint32_t *actState, uint32_t *reqState, uint32_t actStatus);

// do autorecovery from error state
void fwlib_doAutoRecovery(uint32_t actState, uint32_t *reqState);

// intialize Etherbone master
uint32_t fwlib_ebmInit(uint32_t msTimeout, uint64_t dstMac, uint32_t dstIp, uint32_t eb_ops);

// write N words using Etherbone
uint32_t fwlib_ebmWriteN(uint32_t address, uint32_t *data, uint32_t n32BitWords);

// read N words using Etherbone
uint32_t fwlib_ebmReadN(uint32_t msTimeout, uint32_t address, uint32_t *data, uint32_t n32BitWords);

// print stuff to OLED
void fwlib_printOLED(char *chars);

// clear OLED
void fwlib_clearOLED();

#endif 
