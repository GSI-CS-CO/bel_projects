#ifndef _FTM_H_
#define _FTM_H_
#include <inttypes.h>
#include <stdint.h>


//masks & constants
#define CMD_RST           		(1<<0)	//Reset FTM status and counters
#define CMD_START      		   (1<<1)	//Start FTM
#define CMD_IDLE   		      (1<<2)	//Jump into IDLE at next BP
#define CMD_STOP_REQ          (1<<3)	//Stop FTM if when it reaches IDLE state
#define CMD_STOP_NOW          (1<<4)	//Stop FTM immediately

#define CMD_COMMIT_PAGE       (1<<8)  //Commmit new data and validate
#define CMD_COMMIT_ALT        (1<<9)  //Commit alt Plan pointer. Will be selected at next BP if not NULL
#define CMD_PAGE_SWAP         (1<<10)  //swap Page at next BP

#define CMD_SHOW_ACT          (1<<11)
#define CMD_SHOW_INA          (1<<12)

#define CMD_DBG_0             (1<<16)  //DBG case 0
#define CMD_DBG_1             (1<<17)  //DBG case 1



#define STAT_RUNNING          (1<<0)   //the FTM is running
#define STAT_IDLE             (1<<1)   //the FTM is idling  
#define STAT_IDLE_REQ         (1<<2)   //alt ptr has been set to IDLE 
#define STAT_STOPPED          (1<<1)   //FTM is Stopped - will not execute anything 
#define STAT_ERROR            (1<<3)   //FTM encountered an error, check error register



#define TIMER_CYC_START       8
#define TIMER_CYC_PREP        TIMER_CYC_START+1 
#define TIMER_MSG_PREP        TIMER_CYC_PREP+1  
#define TIMER_ABS             CYC_ABS_TIME
#define TIMER_PER             (1<<2) 


#define TIMER_CYC_START_MSK   (1<<TIMER_CYC_START)
#define TIMER_CYC_PREP_MSK    (1<<TIMER_CYC_PREP) 
#define TIMER_MSG_PREP_MSK    (1<<TIMER_MSG_PREP)

#define TIMER_CFG_SUCCESS     0
#define TIMER_CFG_ERROR_0     -1
#define TIMER_CFG_ERROR_1     -2


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
#define FLAGS_IS_COND         (1<<1)
#define FLAGS_IS_START        (1<<2) // debug
#define FLAGS_IS_END          (1<<3) // debug

#define FTM_IS_RUNNING        (1<<0) 
#define FTM_IS_STOP_REQ       (1<<1) 

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

#define FTM_CYC_TTRN          0
#define FTM_CYC_TSTART        (FTM_CYC_TTRN           + 8)
#define FTM_CYC_TPERIOD       (FTM_CYC_TSTART         + 8)
#define FTM_CYC_TEXEC         (FTM_CYC_TPERIOD        + 8)
#define FTM_CYC_FLAGS         (FTM_CYC_TEXEC          + 8)
#define FTM_CYC_PCONDINDPUT   (FTM_CYC_FLAGS          + 4)
#define FTM_CYC_PCONDPATTERN  (FTM_CYC_PCONDINDPUT    + 4)
#define FTM_CYC_CONDMSK       (FTM_CYC_PCONDPATTERN   + 4)
#define FTM_CYC_REPQTY        (FTM_CYC_CONDMSK        + 4)
#define FTM_CYC_REPCNT        (FTM_CYC_REPQTY         + 4)
#define FTM_CYC_MSGQTY        (FTM_CYC_REPCNT         + 4)
#define FTM_CYC_MSGIDX        (FTM_CYC_MSGQTY         + 4)
#define FTM_CYC_PMSG          (FTM_CYC_MSGIDX         + 4)
#define FTM_CYC_PNEXT         (FTM_CYC_PMSG           + 4)
#define FTM_CYC_END_          (FTM_CYC_PNEXT          + 4)




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
   uint32_t cfg_ENA;
   uint32_t cfg_FIFO;    
   uint32_t cfg_IRQ;
   uint32_t cfg_AUTOPOP;
   uint32_t cfg_AUTOFLUSH_TIME;
   uint32_t cfg_AUTOFLUSH_MSGS;
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
               .cfg_ENA             = 1<<0,
               .cfg_FIFO            = 1<<1,    
               .cfg_IRQ             = 1<<2,
               .cfg_AUTOPOP         = 1<<3,
               .cfg_AUTOFLUSH_TIME  = 1<<4,
               .cfg_AUTOFLUSH_MSGS  = 1<<5,
               .force_POP           = 1<<0,
               .force_FLUSH         = 1<<1
};

typedef uint32_t t_status;

typedef struct {
   uint32_t hi;
   uint32_t lo;
} t_dw;

typedef union {
   uint64_t   v64;
   t_dw       v32;               
} u_dword;

typedef u_dword t_time ;

typedef struct {
   u_dword  id;
   u_dword  par;
   uint32_t tef;
   uint32_t res;
   u_dword  ts;
   u_dword  offs;
} t_ftmMsg;

typedef struct {
   
   u_dword              tTrn;          //worst case time transmission will take
   u_dword              tStart;        //desired start time of this cycle
   u_dword              tPeriod;       //cycle period
   u_dword              tExec;         //cycle execution time. if repQty > 0 or -1, this will be tStart + n*tPeriod
   uint32_t             flags;         //apart from CYC_IS_BP, this is just markers for status info & better debugging
   uint32_t*            pCondInput;    //pointer to location to poll for condition
   uint32_t*            pCondPattern;  //pointer to pattern to compare
   uint32_t             condMsk;       //mask for comparison in condition
   uint32_t             repQty;        //number of desired repetitions. -1 -> infinite, 0 -> none
   uint32_t             repCnt;        //running count of repetitions
   uint32_t             msgQty;        //Number of messages
   uint32_t             msgIdx;        //idx of the currently processed msg 
   t_ftmMsg*            pMsg;          //pointer to messages
   struct t_ftmCycle*   pNext;         //pointer to next cycle
   
} t_ftmCycle;

typedef struct {

   uint32_t       space[ ((4096/4 -32)/2  - 32) ];
   uint32_t       planQty;
   t_ftmCycle     pPlans[16];
   t_ftmCycle*    pAlt;
   t_ftmCycle*    pStart;
} t_ftmPage;

typedef struct {
   t_ftmPage   pPages[2];
   uint32_t    cmd;
   uint32_t    status;
   t_ftmPage*  pAct;
   t_ftmPage*  pIna;
   t_ftmCycle* pBP;
   uint64_t    tPrep;
} t_FtmIf;

volatile t_FtmIf* pFtmIf;
void              prioQueueInit(uint64_t trn, uint64_t due);
void              ftmInit(void);
void              demoInit(void);
t_ftmCycle*       processCycle(t_ftmCycle* this);    //checks for condition and if cycle is to be processed ( repQty != 0 )
t_ftmCycle*       processCycleAux(t_ftmCycle* this); //does the actual work
int               dispatchMsg(t_ftmMsg* pMsg);  //dispatch a message to prio queue
void              evalCmd();
void              showPage(t_ftmPage* pPage);



t_ftmCycle* deserCycle( uint8_t*    buf, t_ftmCycle* cyc);

uint8_t* serCycle(uint8_t*    buf, 
                  uint64_t    tTrn,
                  uint64_t    tStart,
                  uint64_t    tPeriod,
                  uint64_t    tExec,       
                  uint32_t    flags,
                  uint32_t*   pCondInput,    
                  uint32_t*   pCondPattern,  
                  uint32_t    condMsk,       
                  uint32_t    repQty,        
                  uint32_t    repCnt,
                  uint32_t    msgQty,
                  uint32_t    msgIdx,
                  t_ftmMsg*   pMsg,
                  t_ftmCycle* pNext);

t_ftmMsg* deserMsg(  uint8_t*  buf, t_ftmMsg* msg);

uint8_t*  serMsg(    uint8_t* buf, 
                     uint64_t id,
                     uint64_t par,
                     uint32_t tef,
                     uint32_t res,
                     uint64_t ts,
                     uint64_t offs);

uint16_t    getIdFID(uint64_t id);
uint16_t    getIdGID(uint64_t id);
uint16_t    getIdEVTNO(uint64_t id);
uint16_t    getIdSID(uint64_t id);
uint16_t    getIdBPID(uint64_t id);
uint16_t    getIdSCTR(uint64_t id);
//int      sendCustomMsg(uint32_t* customMsg, unsigned char len,  );

#endif
