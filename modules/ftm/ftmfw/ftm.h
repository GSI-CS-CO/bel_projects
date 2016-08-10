#ifndef _FTM_FW_H_
#define _FTM_FW_H_
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "dbg.h"
#include "mini_sdb.h"
#include "irq.h"
#include "aux.h"
#include "../ftm_common.h"
#include "prio_regs.h"

extern uint32_t*       _startshared[];
extern uint32_t*       _endshared[];

#define PRIO_BIT_ENABLE     (1<<0)
#define PRIO_BIT_MSG_LIMIT  (1<<1)
#define PRIO_BIT_TIME_LIMIT (1<<2)

#define PRIO_DAT_STD     0x00
#define PRIO_DAT_TS_HI   0x04    
#define PRIO_DAT_TS_LO   0x08 
#define PRIO_DRP_TS_HI   0x14    
#define PRIO_DRP_TS_LO   0x18  

#define DIAG_PQ_MSG_CNT  0x0FA62F9000000000
   

uint32_t cmdCnt;

#define MSI_SIG             0
#define TLU_CH_TEST         0x60



uint32_t prioQcapacity;

//FIXME No more structs, structs are BAD when used between platforms.
//intermediate solution is to use the base ptr from the LBT plus an offset

//TODO blocks are to be live code, not just data. this will get rid of all the offset business if we use PIC
//TODO TODO get rid of instruction cache
//TODO TODO TODO tie RAM port directly to LM32 for 1 cycle access


/*

typedef struct {
   uint8_t sig;
   uint8_t cond;
} t_semaphore;  

typedef struct {
   uint8_t reserved [ FTM_PAGEDATA ];
} t_pageSpace;  




typedef struct {
   uint32_t value; 
   t_time   time;
} t_shared;
*/

typedef uint64_t t_time ;

typedef struct {
   uint64_t id;
   uint64_t par;
   uint32_t tef;
   uint32_t res;
   t_time   ts;
   t_time   offs;
} t_ftmMsg;

typedef struct {
   
   t_time               tStart;  //desired start time of this chain
   t_time               tPeriod; //chain period
   t_time               tExec;   //chain execution time. if repQty > 0 or -1, this will be tStart + n*tPeriod 
   uint32_t             flags; 
   uint32_t*            condSrc; //condition source adr
   uint32_t             condVal; //pattern to compare
   uint32_t             condMsk; //mask for comparison in condition
   uint32_t*            sigDst;  //dst adr for signalling
   uint32_t             sigVal;  //value for signalling
   uint32_t             repQty;  //number of desired repetitions. -1 -> infinite, 0 -> none
   uint32_t             repCnt;  //running count of repetitions
   uint32_t             msgQty;  //Number of messages
   uint32_t             msgIdx;  //idx of the currently processed msg
     
   uint32_t             msgOffset; //offset to messages
   uint32_t             nextOffset;   //offset to next chain


} t_ftmChain;


/*
//a plan is a linked list of chains
typedef struct {

   uint32_t       planQty;
   t_ftmChain*    plans[FTM_PLAN_MAX];
   t_ftmChain*    pBp;
   t_ftmChain*    pStart;
   t_pageSpace    pagedummy;
   
} t_ftmPage;

*/
volatile uint32_t* p;

volatile void* pV;

t_ftmChain* pCurrentChain;





volatile uint64_t* pMsgCntPQ;

void prioQueueInit();
void ftmInit(void);
void processFtm();

void cmdEval();
//void showFtmPage(t_ftmPage* pPage);
void showStatus();

extern uint32_t * pEcaAdr;
extern uint32_t * pEbmAdr;
extern uint32_t * pFPQctrl;


inline uint16_t getIdFID(uint64_t id)     {return ((uint16_t)(id >> ID_FID_POS))     & (ID_MSK_B16 >> (16 - ID_FID_LEN));}
inline uint16_t getIdGID(uint64_t id)     {return ((uint16_t)(id >> ID_GID_POS))     & (ID_MSK_B16 >> (16 - ID_GID_LEN));}
inline uint16_t getIdEVTNO(uint64_t id)   {return ((uint16_t)(id >> ID_EVTNO_POS))   & (ID_MSK_B16 >> (16 - ID_EVTNO_LEN));}
inline uint16_t getIdSID(uint64_t id)     {return ((uint16_t)(id >> ID_SID_POS))     & (ID_MSK_B16 >> (16 - ID_SID_LEN));}
inline uint16_t getIdBPID(uint64_t id)    {return ((uint16_t)(id >> ID_BPID_POS))    & (ID_MSK_B16 >> (16 - ID_BPID_LEN));}
inline uint16_t getIdSCTR(uint64_t id)    {return ((uint16_t)(id >> ID_SCTR_POS))    & (ID_MSK_B16 >> (16 - ID_SCTR_LEN));}
inline void incIdSCTR(uint64_t* id, volatile uint16_t* sctr)   {*id = ( *id & 0xfffffffffffffc00) | *sctr; *sctr = (*sctr+1) & ~0xfc00; DBPRINT3("id: %x sctr: %x\n", *id, *sctr);}
inline uint32_t hiW(uint64_t dword) {return (uint32_t)(dword >> 32);}
inline uint32_t loW(uint64_t dword) {return (uint32_t)dword;}

#endif
