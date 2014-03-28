#ifndef _FTM_H_
#define _FTM_H_



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
#define TIMER_ABS       CYC_ABS_TIME
#define TIMER_PER        (1<<2) 


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

extern unsigned int*       _startshared[];
extern unsigned int*       _endshared[];



// Priority Queue RegisterLayout
static const struct {
   unsigned int rst;
   unsigned int force;
   unsigned int dbgSet;
   unsigned int dbgGet;
   unsigned int clear;
   unsigned int cfgGet;
   unsigned int cfgSet;
   unsigned int cfgClr;
   unsigned int dstAdr;
   unsigned int heapCnt;
   unsigned int msgCntO;
   unsigned int msgCntI;
   unsigned int tTrnHi;
   unsigned int tTrnLo;
   unsigned int tDueHi;
   unsigned int tDueLo;
   unsigned int capacity;
   unsigned int msgMax;
   unsigned int ebmAdr;
   unsigned int cfg_ENA;
   unsigned int cfg_FIFO;    
   unsigned int cfg_IRQ;
   unsigned int cfg_AUTOPOP;
   unsigned int cfg_AUTOFLUSH_TIME;
   unsigned int cfg_AUTOFLUSH_MSGS;
   unsigned int force_POP;
   unsigned int force_FLUSH;
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



typedef unsigned int t_status;



typedef struct {
   unsigned int hi;
   unsigned int lo;
} t_dw;

typedef union {
   unsigned long long   v64;
   t_dw                 v32;               
} u_dword;

typedef u_dword t_time ;

typedef struct {
   u_dword id;
   u_dword par;
   unsigned int tef;
   unsigned int res;
   u_dword ts;
   u_dword offs;
} t_ftmMsg;


typedef struct {
   
   const u_dword        tTrn;     //worst case time transmission will take
   const u_dword        tPrep;    //time offset to prepare msgs
   u_dword        tStart;   //desired start time of this cycle
   const u_dword        tPeriod;  //cycle period
         u_dword        tExec;    //cycle execution time. if repQty > 0 or -1, this will be tStart + n*tPeriod
   const unsigned int  flags;    //apart from CYC_IS_BP, this is just markers for status info & better debugging
   const unsigned int* pCondInput;   //pointer to location to poll for condition
   const unsigned int* pCondPattern; //pointer to pattern to compare
   const unsigned int  condMsk;     //mask for comparison in condition
   const unsigned int  repQty;   //number of desired repetitions. -1 -> infinite, 0 -> none
         unsigned int  repCnt;   //running count of repetitions
   const unsigned int  msgQty;   //Number of messages
         unsigned int  msgIdx;   //idx of the currently processed msg 
   const t_ftmMsg*     pMsg;     //pointer to messages
   const struct t_ftmCycle*  pNext;    //pointer to next cycle
   const unsigned int  planID;   //+++ to be removed, just for debugging
   const unsigned int  cycID;    //+++ to be removed, just for debugging
   
} t_ftmCycle;




typedef struct {

   unsigned int   space[ ((4096/4 -32)/2  - 32) ];
   unsigned int   planQty;
   t_ftmCycle     pPlans[16];
   t_ftmCycle*    pAlt;
   t_ftmCycle*    pStart;
} t_ftmPage;

typedef struct {
   t_ftmPage            pPages[2];
   unsigned int         cmd;
   unsigned int         status;
   t_ftmPage*  pAct;
   t_ftmPage*  pIna;
   t_ftmCycle*          pBP;
   unsigned long long   tPrep;
} t_FtmIf;



volatile t_FtmIf*   pFtmIf;


void prioQueueInit(unsigned long long trn, unsigned long long due);
void     ftmInit(void);
void     fesaInit(void);
t_ftmCycle*   processCycle(t_ftmCycle* this);    //checks for condition and if cycle is to be processed ( repQty != 0 )
t_ftmCycle*   processCycleAux(t_ftmCycle* this); //does the actual work
int      dispatchMsg(t_ftmMsg* pMsg);  //dispatch a message to prio queue
void     evalCmd();
void showPage(t_ftmPage* pPage);

unsigned short getIdFID(unsigned long long id);
unsigned short getIdGID(unsigned long long id);
unsigned short getIdEVTNO(unsigned long long id);
unsigned short getIdSID(unsigned long long id);
unsigned short getIdBPID(unsigned long long id);
unsigned short getIdSCTR(unsigned long long id);
//int      sendCustomMsg(unsigned int* customMsg, unsigned char len,  );








#endif
