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

uint32_t cmdCnt;

#define MSI_SIG             0
#define TLU_CH_TEST         0x60

struct t_FPQ {
   uint32_t rst;
   uint32_t force;
   uint32_t dbgSet;
   uint32_t dbgGet;
   uint32_t clear;
   uint32_t cfgGet;
   uint32_t cfgSet;
   uint32_t cfgClr;
   uint32_t dstAdr;
   uint32_t heapCnt;
   uint32_t msgCntO;
   uint32_t msgCntI;
   uint32_t tTrnHi;
   uint32_t tTrnLo;
   uint32_t tDueHi;
   uint32_t tDueLo;
   uint32_t capacity;
   uint32_t msgMax;
   uint32_t ebmAdr;
   uint32_t tsAdr;
   uint32_t tsCh;
   uint32_t cfg_ENA;
   uint32_t cfg_FIFO;    
   uint32_t cfg_IRQ;
   uint32_t cfg_AUTOPOP;
   uint32_t cfg_AUTOFLUSH_TIME;
   uint32_t cfg_AUTOFLUSH_MSGS;
   uint32_t cfg_MSG_ARR_TS;
   uint32_t force_POP;
   uint32_t force_FLUSH;
};

extern const struct t_FPQ r_FPQ;

uint32_t prioQcapacity;


typedef struct {
   uint8_t sig;
   uint8_t cond;
} t_semaphore;  

typedef struct {
   uint8_t reserved [ FTM_PAGEDATA ];
} t_pageSpace;  

typedef uint64_t t_time ;


typedef struct {
   uint32_t value; 
   t_time   time;
} t_shared;

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
   t_time               tExec;   //chain execution time. if repQty > 0 or -1, this will be tStart + n*tPeriod FIXME
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
   t_ftmMsg*            pMsg;    //pointer to messages
   struct t_ftmChain*   pNext;   //pointer to next chain
   
} t_ftmChain;

//a plan is a linked list of chains
typedef struct {

   uint32_t       planQty;
   t_ftmChain*    plans[FTM_PLAN_MAX];
   t_ftmChain*    pBp;
   t_ftmChain*    pStart;
   t_pageSpace    pagedummy;
   
} t_ftmPage;

typedef struct {
   t_ftmPage   pPages[2];
   uint32_t    cmd;
   uint32_t    status;
   t_ftmPage*  pAct;
   t_ftmPage*  pIna;
   t_ftmChain* pNewBp;
   t_shared*   pSharedMem;
   uint64_t    tPrep;
   uint64_t    tDue;
   uint64_t    tTrn;
   t_ftmChain  idle;
   t_semaphore sema;
   uint32_t    sctr;
   uint32_t    debug[32];
} t_ftmIf;

volatile t_ftmIf* pFtmIf;
t_ftmChain* pCurrentChain;

void              prioQueueInit();
void              ftmInit(void);
void              processFtm();

void              cmdEval();
void showFtmPage(t_ftmPage* pPage);
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
