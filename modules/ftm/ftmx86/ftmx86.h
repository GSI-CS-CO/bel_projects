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
#define FTM_MSG_LEN           (FTM_MSG_OFFS  + 8)

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
#define FTM_CYC_LEN           (FTM_CYC_PNEXT          + 4)

extern uint32_t*       _startshared[];
extern uint32_t*       _endshared[];


typedef struct {
   uint64_t id;
   uint64_t par;
   uint32_t tef;
   uint32_t res;
   uint64_t ts;
   uint64_t offs;
} t_ftmMsg;

typedef struct {
   
   uint64_t             tTrn;          //worst case time transmission will take
   uint64_t             tStart;        //desired start time of this cycle
   uint64_t             tPeriod;       //cycle period
   uint64_t             tExec;         //cycle execution time. if repQty > 0 or -1, this will be tStart + n*tPeriod
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
                  uint32_t    pMsg,
                  uint32_t    pNext);

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
uint64_t getId(uint16_t fid, uint16_t gid, uint16_t evtno, uint16_t sid, uint16_t bpid, uint16_t sctr);
//int      sendCustomMsg(uint32_t* customMsg, unsigned char len,  );
extern t_FtmIf*       pFtmIf;
#endif
