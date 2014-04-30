#ifndef _FTM_H_
#define _FTM_H_
#include <inttypes.h>
#include <stdint.h>

#define FTM_PAGESIZE        8192  
#define FTM_SHARED_OFFSET   0xC000
#define FTM_CMD_OFFSET      (FTM_SHARED_OFFSET  + 2*FTM_PAGESIZE)
#define FTM_STAT_OFFSET     (FTM_CMD_OFFSET     + 4)
#define FTM_PACT_OFFSET     (FTM_STAT_OFFSET    + 4)
#define FTM_PINA_OFFSET     (FTM_PACT_OFFSET    + 4)
#define FTM_TPREP_OFFSET    (FTM_PINA_OFFSET    + 4)
#define FTM_IDLE_OFFSET     (FTM_TPREP_OFFSET   + 8)

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
#define FLAGS_IS_COND_MSI     (1<<1)
#define FLAGS_IS_COND_SHARED  (1<<2)
#define FLAGS_IS_SIG          (1<<3)
#define FLAGS_IS_START        (1<<4) // debug
#define FLAGS_IS_END          (1<<5) // debug

#define FTM_IS_RUNNING        (1<<0) 
#define FTM_IS_STOP_REQ       (1<<1) 

#define FTM_TIME_SIZE         8
#define FTM_DWORD_SIZE        8
#define FTM_WORD_SIZE         4
#define FTM_PTR_SIZE          4
#define FTM_NULL              0x0

#define FTM_MSG_ID            0
#define FTM_MSG_PAR           (FTM_MSG_ID    + 8)
#define FTM_MSG_TEF           (FTM_MSG_PAR   + 8)
#define FTM_MSG_RES           (FTM_MSG_TEF   + 4)
#define FTM_MSG_TS            (FTM_MSG_RES   + 4)
#define FTM_MSG_OFFS          (FTM_MSG_TS    + 8)
#define _FTM_MSG_LEN          (FTM_MSG_OFFS  + 8)


#define FTM_CYC_TSTART        0
#define FTM_CYC_TPERIOD       (FTM_CYC_TSTART         + 8)
#define FTM_CYC_TEXEC         (FTM_CYC_TPERIOD        + 8)
#define FTM_CYC_FLAGS         (FTM_CYC_TEXEC          + 8)
#define FTM_CYC_CONDVAL       (FTM_CYC_FLAGS          + 4)
#define FTM_CYC_CONDMSK       (FTM_CYC_CONDVAL        + 8)
#define FTM_CYC_SIGDST        (FTM_CYC_CONDMSK        + 8)
#define FTM_CYC_SIGVAL        (FTM_CYC_SIGDST         + 4)
#define FTM_CYC_REPQTY        (FTM_CYC_SIGVAL         + 4)
#define FTM_CYC_REPCNT        (FTM_CYC_REPQTY         + 4)
#define FTM_CYC_MSGQTY        (FTM_CYC_REPCNT         + 4)
#define FTM_CYC_MSGIDX        (FTM_CYC_MSGQTY         + 4)
#define FTM_CYC_PMSG          (FTM_CYC_MSGIDX         + 4)
#define FTM_CYC_PNEXT         (FTM_CYC_PMSG           + 4)
#define _FTM_CYC_LEN          (FTM_CYC_PNEXT          + 4)

#define FTM_PLAN_MAX          16

#define FTM_PAGE_QTY          0
#define FTM_PAGE_PLANPTRS     (FTM_PAGE_QTY            +4)   
#define FTM_PAGE_PTR_BP       (FTM_PAGE_PLANPTRS       + FTM_PLAN_MAX * 4)
#define FTM_PAGE_PTR_START    (FTM_PAGE_PTR_BP         +4)
#define FTM_PAGE_PTR_SHAREDMEM (FTM_PAGE_PTR_START     +4)
#define _FTM_PAGE_LEN         (FTM_PAGE_PTR_SHAREDMEM  +4)


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
   
   uint64_t             tStart;        //desired start time of this cycle
   uint64_t             tPeriod;       //cycle period
   uint64_t             tExec;         //cycle execution time. if repQty > 0 or -1, this will be tStart + n*tPeriod
   uint32_t             flags;         //apart from CYC_IS_BP, this is just markers for status info & better debugging
   uint64_t             condVal;       //pattern to compare
   uint64_t             condMsk;       //mask for comparison in condition
   uint32_t             sigDst;       //mask for comparison in condition
   uint32_t             sigVal;       //mask for comparison in condition
   uint32_t             repQty;        //number of desired repetitions. -1 -> infinite, 0 -> none
   uint32_t             repCnt;        //running count of repetitions
   uint32_t             msgQty;        //Number of messages
   uint32_t             msgIdx;        //idx of the currently processed msg 
   t_ftmMsg*            pMsg;          //pointer to messages
   struct t_ftmCycle*   pNext;         //pointer to next cycle
   
} t_ftmCycle;

typedef struct {
   uint32_t       cycQty;
   t_ftmCycle*    pStart;
} t_ftmPlan;


typedef struct {

   uint32_t       planQty;
   t_ftmPlan      plans[FTM_PLAN_MAX];
   uint32_t       idxBp;
   uint32_t       idxStart;
   uint32_t       pBp;
   uint32_t       PStart;
   uint32_t       pSharedMem;
} t_ftmPage;

t_ftmPage* deserPage(t_ftmPage* pPage, uint8_t* pBufStart, uint32_t embeddedOffs);
uint8_t* deserCycle(t_ftmCycle* pCyc, t_ftmCycle* pNext, uint8_t* pCycStart, uint8_t* pBufStart, uint32_t embeddedOffs);
uint8_t* deserMsg(  uint8_t*  buf, t_ftmMsg* msg);


uint8_t* uint32ToBytes(uint8_t* buf, uint32_t val);
uint8_t* uint64ToBytes(uint8_t* buf, uint64_t val);
uint32_t bytesToUint32(uint8_t* buf);
uint64_t bytesToUint64(uint8_t* buf);

uint8_t* serPage  (t_ftmPage*  pPage, uint8_t* bufStart, uint32_t offset);
uint8_t* serCycle (t_ftmCycle* pCyc, uint8_t* bufStart, uint8_t* buf, uint32_t offset);
uint8_t* serMsg   (t_ftmMsg* pMsg, uint8_t* buf);
void showFtmPage(t_ftmPage* pPage);

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
