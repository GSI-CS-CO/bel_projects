#ifndef _FTM_H_
#define _FTM_H_
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "dbg.h"
#include "mini_sdb.h"
#include "irq.h"
#include "aux.h"


#define MSI_SIG             0
#define TLU_CH_TEST         0x60

#define FTM_PLAN_MAX        16
#define FTM_PAGEDATA        0x0600
#define FTM_PAGEMETA        (4 + 4 * FTM_PLAN_MAX + 4 + 4)   
#define FTM_PAGESIZE        (FTM_PAGEDATA + FTM_PAGEMETA)


#define FTM_CMD_OFFSET      (2*FTM_PAGESIZE)
#define FTM_STAT_OFFSET     (FTM_CMD_OFFSET     + 4)
#define FTM_PACT_OFFSET     (FTM_STAT_OFFSET    + 4)
#define FTM_PINA_OFFSET     (FTM_PACT_OFFSET    + 4)
#define FTM_TPREP_OFFSET    (FTM_PINA_OFFSET    + 4)
#define FTM_IDLE_OFFSET     (FTM_TPREP_OFFSET   + 8)



//masks & constants
#define CMD_RST           		0x0001	//Reset FTM status and counters
#define CMD_START      		   0x0002	//Start FTM
#define CMD_IDLE   		      0x0004	//Jump into IDLE at next BP
#define CMD_STOP_REQ          0x0008	//Stop FTM if when it reaches IDLE state
#define CMD_STOP_NOW          0x0010	//Stop FTM immediately

#define CMD_COMMIT_PAGE       0x0100  //Commmit new data and validate
#define CMD_COMMIT_BP         0x0200  //Commit alt Plan pointer. Will be selected at next BP if not NULL
#define CMD_PAGE_SWAP         0x0400  //swap Page at next BP
#define CMD_SHOW_ACT          0x0800
#define CMD_SHOW_INA          0x1000
#define CMD_DBG_0             0x2000  //DBG case 0
#define CMD_DBG_1             0x4000  //DBG case 1

#define STAT_RUNNING          (1<<0)   //the FTM is running
#define STAT_IDLE             (1<<1)   //the FTM is idling  
#define STAT_STOP_REQ         (1<<2)   //alt ptr has been set to IDLE 
#define STAT_ERROR            (1<<3)   //FTM encountered an error, check error register
#define STAT_WAIT             (1<<4)   //FTM waiting on condition


#define ID_MSK_B16            0xffff
#define ID_FID_LEN            4
#define ID_GID_LEN            12
#define ID_EVTNO_LEN          12
#define ID_SID_LEN            12
#define ID_BPID_LEN           14
#define ID_SCTR_LEN           10
#define ID_FID_POS            (ID_GID_LEN + ID_EVTNO_LEN + ID_SID_LEN + ID_BPID_LEN + ID_SCTR_LEN)
#define ID_GID_POS            (ID_EVTNO_LEN + ID_SID_LEN + ID_BPID_LEN + ID_SCTR_LEN)
#define ID_EVTNO_POS          (ID_SID_LEN + ID_BPID_LEN + ID_SCTR_LEN)
#define ID_SID_POS            (ID_BPID_LEN + ID_SCTR_LEN)
#define ID_BPID_POS           (ID_SCTR_LEN)
#define ID_SCTR_POS           0


#define FLAGS_IS_BP           (1<<0)

#define FLAGS_IS_COND_MSI     (1<<4)
#define FLAGS_IS_COND_SHARED  (1<<5)
#define FLAGS_IS_COND_ADR     (1<<6)
#define FLAGS_IS_COND_TIME    (1<<7)

#define FLAGS_IS_SIG_MSI      (1<<8)
#define FLAGS_IS_SIG_SHARED   (1<<9)
#define FLAGS_IS_SIG_ADR      (1<<10)
#define FLAGS_IS_SIG_TIME     (1<<11)

#define FLAGS_IS_SIG_FIRST    (1<<12)
#define FLAGS_IS_SIG_LAST     (1<<13)
#define FLAGS_IS_SIG_ALL      (1<<14)

#define FLAGS_IS_START        (1<<16) // debug
#define FLAGS_IS_END          (1<<17) // debug
#define FLAGS_IS_ENDLOOP      (1<<18)

#define FTM_TIME_SIZE         8
#define FTM_DWORD_SIZE        8
#define FTM_WORD_SIZE         4
#define FTM_PTR_SIZE          4

#define FTM_MSG_ID            0
#define FTM_MSG_PAR           (FTM_MSG_ID    + 8)
#define FTM_MSG_TEF           (FTM_MSG_PAR   + 8)
#define FTM_MSG_RES           (FTM_MSG_TEF   + 4)
#define FTM_MSG_TS            (FTM_MSG_RES   + 4)
#define FTM_MSG_OFFS          (FTM_MSG_TS    + 8)
#define FTM_MSG_END_          (FTM_MSG_OFFS  + 8)

#define FTM_CHAIN_TTRN          0
#define FTM_CHAIN_TSTART        (FTM_CHAIN_TTRN           + 8)
#define FTM_CHAIN_TPERIOD       (FTM_CHAIN_TSTART         + 8)
#define FTM_CHAIN_TEXEC         (FTM_CHAIN_TPERIOD        + 8)
#define FTM_CHAIN_FLAGS         (FTM_CHAIN_TEXEC          + 8)
#define FTM_CHAIN_PCONDINDPUT   (FTM_CHAIN_FLAGS          + 4)
#define FTM_CHAIN_PCONDPATTERN  (FTM_CHAIN_PCONDINDPUT    + 4)
#define FTM_CHAIN_CONDMSK       (FTM_CHAIN_PCONDPATTERN   + 4)
#define FTM_CHAIN_REPQTY        (FTM_CHAIN_CONDMSK        + 4)
#define FTM_CHAIN_REPCNT        (FTM_CHAIN_REPQTY         + 4)
#define FTM_CHAIN_MSGQTY        (FTM_CHAIN_REPCNT         + 4)
#define FTM_CHAIN_MSGIDX        (FTM_CHAIN_MSGQTY         + 4)
#define FTM_CHAIN_PMSG          (FTM_CHAIN_MSGIDX         + 4)
#define FTM_CHAIN_PNEXT         (FTM_CHAIN_PMSG           + 4)
#define FTM_CHAIN_END_          (FTM_CHAIN_PNEXT          + 4)




extern uint32_t*       _startshared[];
extern uint32_t*       _endshared[];


// Priority Queue RegisterLayout
static const struct {
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
} r_FPQ = {    .rst        =  0x00 >> 2,
               .force      =  0x04 >> 2,
               .dbgSet     =  0x08 >> 2,
               .dbgGet     =  0x0c >> 2,
               .clear      =  0x10 >> 2,
               .cfgGet     =  0x14 >> 2,
               .cfgSet     =  0x18 >> 2,
               .cfgClr     =  0x1C >> 2,
               .dstAdr     =  0x20 >> 2,
               .heapCnt    =  0x24 >> 2,
               .msgCntO    =  0x28 >> 2,
               .msgCntI    =  0x2C >> 2,
               .tTrnHi     =  0x30 >> 2,
               .tTrnLo     =  0x34 >> 2,
               .tDueHi     =  0x38 >> 2,
               .tDueLo     =  0x3C >> 2,
               .capacity   =  0x40 >> 2,
               .msgMax     =  0x44 >> 2,
               .ebmAdr     =  0x48 >> 2,
               .tsAdr      =  0x4C >> 2,
               .tsCh       =  0x50 >> 2,
               .cfg_ENA             = 1<<0,
               .cfg_FIFO            = 1<<1,    
               .cfg_IRQ             = 1<<2,
               .cfg_AUTOPOP         = 1<<3,
               .cfg_AUTOFLUSH_TIME  = 1<<4,
               .cfg_AUTOFLUSH_MSGS  = 1<<5,
               .cfg_MSG_ARR_TS      = 1<<6,
               .force_POP           = 1<<0,
               .force_FLUSH         = 1<<1
};

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
   uint64_t    tPrep;
   uint64_t    tDue;
   uint64_t    tTrn;
   t_ftmChain  idle;
   t_shared*   pSharedMem;
   t_semaphore sema;
} t_ftmIf;

volatile t_ftmIf* pFtmIf;
t_ftmChain* pCurrentChain;

void              prioQueueInit();
void              ftmInit(void);
void              sigSend(t_ftmChain* c);
uint8_t           condValid(t_ftmChain* c);

void              processFtm();
t_ftmChain*       processChain(t_ftmChain* c);    //checks for condition and if chain is to be processed ( repQty != 0 )
t_ftmChain*       processChainAux(t_ftmChain* c); //does the actual work
int               dispatch(t_ftmMsg* pMsg);  //dispatch a message to prio queue
void              cmdEval();
void showFtmPage(t_ftmPage* pPage);
void showStatus();

extern unsigned int * pEcaAdr;
extern unsigned int * pEbmAdr;
extern unsigned int * pFPQctrl;

uint16_t    getIdFID(uint64_t id);
uint16_t    getIdGID(uint64_t id);
uint16_t    getIdEVTNO(uint64_t id);
uint16_t    getIdSID(uint64_t id);
uint16_t    getIdBPID(uint64_t id);
uint16_t    getIdSCTR(uint64_t id);

#endif
