#ifndef _FTM_H_
#define _FTM_H_
#include <inttypes.h>
#include <stdint.h>

#define FTM_PLAN_MAX        16
#define FTM_PAGEDATA        0x0600
#define FTM_PAGEMETA        (4 + 4 * FTM_PLAN_MAX + 4 + 4)   
#define FTM_PAGESIZE        (FTM_PAGEDATA + FTM_PAGEMETA) 

#define FTM_SHARED_OFFSET   0xC000
#define FTM_CMD_OFFSET      (FTM_SHARED_OFFSET  + 2*FTM_PAGESIZE)
#define FTM_STAT_OFFSET     (FTM_CMD_OFFSET     + 4)
#define FTM_PACT_OFFSET     (FTM_STAT_OFFSET    + 4)
#define FTM_PINA_OFFSET     (FTM_PACT_OFFSET    + 4)
#define FTM_TPREP_OFFSET    (FTM_PINA_OFFSET    + 4)
#define FTM_IDLE_OFFSET     (FTM_TPREP_OFFSET   + 8)

#define FTM_PAGE_PLANQTY_OFFSET  0
#define FTM_PAGE_PLANS_OFFSET    (FTM_PAGE_PLANQTY_OFFSET + 4)
#define FTM_PAGE_BP_OFFSET       (FTM_PAGE_PLANS_OFFSET   + 4 * FTM_PLAN_MAX)

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
#define STAT_STOPPED          (1<<1)   //FTM is Stopped - will not execute anything 
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
#define FLAGS_IS_COND_ALL     (1<<8)

#define FLAGS_IS_SIG_MSI      (1<<12)
#define FLAGS_IS_SIG_SHARED   (1<<13)
#define FLAGS_IS_SIG_ADR      (1<<14)
#define FLAGS_IS_SIG_TIME     (1<<15)

#define FLAGS_IS_SIG_FIRST    (1<<16)
#define FLAGS_IS_SIG_LAST     (1<<17)
#define FLAGS_IS_SIG_ALL      (1<<18)

#define FLAGS_IS_START        (1<<20) 
#define FLAGS_IS_END          (1<<21) 
#define FLAGS_IS_ENDLOOP      (1<<22)
#define FLAGS_IS_PERS_REP_CNT (1<<23)

#define SIG_SH_SENDER_ID      0
#define SIG_SH_FORMAT         1
#define SIG_SH_WORDS          2
#define SIG_SH_RESERVED       3

#define SIG_MSI_SENDER_ID     0
#define SIG_MSI_MSG           1


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


#define FTM_CHAIN_TSTART        0
#define FTM_CHAIN_TPERIOD       (FTM_CHAIN_TSTART         + 8)
#define FTM_CHAIN_TEXEC         (FTM_CHAIN_TPERIOD        + 8)
#define FTM_CHAIN_FLAGS         (FTM_CHAIN_TEXEC          + 8)
#define FTM_CHAIN_CONDSRC       (FTM_CHAIN_FLAGS          + 4)
#define FTM_CHAIN_CONDVAL       (FTM_CHAIN_CONDSRC        + 4)
#define FTM_CHAIN_CONDMSK       (FTM_CHAIN_CONDVAL        + 4)
#define FTM_CHAIN_SIGDST        (FTM_CHAIN_CONDMSK        + 4)
#define FTM_CHAIN_SIGVAL        (FTM_CHAIN_SIGDST         + 4)
#define FTM_CHAIN_REPQTY        (FTM_CHAIN_SIGVAL         + 4)
#define FTM_CHAIN_REPCNT        (FTM_CHAIN_REPQTY         + 4)
#define FTM_CHAIN_MSGQTY        (FTM_CHAIN_REPCNT         + 4)
#define FTM_CHAIN_MSGIDX        (FTM_CHAIN_MSGQTY         + 4)
#define FTM_CHAIN_PMSG          (FTM_CHAIN_MSGIDX         + 4)
#define FTM_CHAIN_PNEXT         (FTM_CHAIN_PMSG           + 4)
#define _FTM_CHAIN_LEN          (FTM_CHAIN_PNEXT          + 4)

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
   
   uint64_t             tStart;        //desired start time of this chain
   uint64_t             tPeriod;       //chain period
   uint64_t             tExec;         //chain execution time. if repQty > 0 or -1, this will be tStart + n*tPeriod
   uint32_t             flags;         //apart from chain_IS_BP, this is just markers for status info & better debugging
   uint32_t             condSrc;       //condition source
   uint32_t             condVal;       //pattern to compare
   uint32_t             condMsk;       //mask for comparison in condition
   uint32_t             sigCpu;       //destination cpu msi/shared
   uint32_t             sigDst;       //destination address msi/shared
   uint32_t             sigVal;       //signal value
   uint32_t             repQty;        //number of desired repetitions. -1 -> infinite, 0 -> none
   uint32_t             repCnt;        //running count of repetitions
   uint32_t             msgQty;        //Number of messages
   uint32_t             msgIdx;        //idx of the currently processed msg 
   t_ftmMsg*            pMsg;          //pointer to messages
   struct t_ftmChain*   pNext;         //pointer to next chain
   
} t_ftmChain;

typedef struct {
   uint32_t       chainQty;
   t_ftmChain*    pStart;
} t_ftmPlan;


typedef struct {

   uint32_t       planQty;
   t_ftmPlan      plans[FTM_PLAN_MAX];
   uint32_t       idxBp;
   uint32_t       idxStart;
   uint32_t       pBp;
   uint32_t       pStart;
   uint32_t       pSharedMem;
} t_ftmPage;

t_ftmPage* deserPage(t_ftmPage* pPage, uint8_t* pBufStart, uint32_t embeddedOffs);
uint8_t* deserChain(t_ftmChain* pChain, t_ftmChain* pNext, uint8_t* pChainStart, uint8_t* pBufStart, uint32_t embeddedOffs);
uint8_t* deserMsg(  uint8_t*  buf, t_ftmMsg* msg);


uint8_t* uint32ToBytes(uint8_t* buf, uint32_t val);
uint8_t* uint64ToBytes(uint8_t* buf, uint64_t val);
uint32_t bytesToUint32(uint8_t* buf);
uint64_t bytesToUint64(uint8_t* buf);

uint8_t* serPage  (t_ftmPage*  pPage, uint8_t* bufStart, uint32_t offset, uint8_t cpuId);
uint8_t* serChain (t_ftmChain* pChain, uint32_t pPlanStart, uint8_t* bufStart, uint8_t* buf, uint32_t offset, uint8_t cpuId);
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
//extern t_FtmIf*       pFtmIf;
#endif
