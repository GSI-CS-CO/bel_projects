#ifndef _FTM_X86_H_
#define _FTM_X86_H_
#include <inttypes.h>
#include <stdint.h>
#include "../ftm_common.h"
#include <stdbool.h>
#define BUF_SIZE            FTM_PAGESIZE

#define FTM_RST_FPGA        0x0
#define FTM_RST_GET         (FTM_RST_FPGA + 4)
#define FTM_RST_SET         (FTM_RST_GET  + 4)
#define FTM_RST_CLR         (FTM_RST_SET  + 4)



#define FTM_PAGE_PLANQTY_OFFSET  0
#define FTM_PAGE_PLANS_OFFSET    (FTM_PAGE_PLANQTY_OFFSET + 4)
#define FTM_PAGE_BP_OFFSET       (FTM_PAGE_PLANS_OFFSET   + 4 * FTM_PLAN_MAX)

#define FTM_PAGE_PLANPTRS     (FTM_PAGE_QTY            +4)   
#define FTM_PAGE_PTR_BP       (FTM_PAGE_PLANPTRS       + FTM_PLAN_MAX * 4)
#define FTM_PAGE_PTR_START    (FTM_PAGE_PTR_BP         +4)
#define FTM_PAGE_PTR_SHAREDMEM (FTM_PAGE_PTR_START     +4)
#define _FTM_PAGE_LEN         (FTM_PAGE_PTR_SHAREDMEM  +4)

bool bigEndian;

typedef struct {
   uint64_t id;
   uint64_t par;
   uint32_t tef;
   uint32_t res;
   uint64_t ts;
   uint64_t offs;
} t_ftmMsg;

typedef struct t_ftmChain t_ftmChain;

struct t_ftmChain {   
   uint64_t             tStart;  //desired start time of this chain
   uint64_t             tPeriod; //chain period
   uint64_t             tExec;   //chain execution time. if repQty > 0 or -1, this will be tStart + n*tPeriod
   uint32_t             flags;   //apart from chain_IS_BP, this is just markers for status info & better debugging
   uint32_t             condSrc; //condition source
   uint32_t             condVal; //pattern to compare
   uint32_t             condMsk; //mask for comparison in condition
   uint32_t             sigCpu;  //destination cpu msi/shared
   uint32_t             sigDst;  //destination address msi/shared
   uint32_t             sigVal;  //signal value
   uint32_t             repQty;  //number of desired repetitions. -1 -> infinite, 0 -> none
   uint32_t             repCnt;  //running count of repetitions
   uint32_t             msgQty;  //Number of messages
   uint32_t             msgIdx;  //idx of the currently processed msg 
   t_ftmMsg*            pMsg;    //pointer to messages
   t_ftmChain*          pNext;   //pointer to next chain
   
};

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

t_ftmPage*  deserPage(t_ftmPage* pPage, uint8_t* pBufStart, uint32_t embeddedOffs);
uint8_t*    serPage  (t_ftmPage* pPage, uint8_t* pBufStart, uint32_t offset, uint8_t cpuId);
void showFtmPage(t_ftmPage* pPage);

t_ftmChain* getChain(t_ftmPage* pPage, uint32_t planIdx, uint32_t chainIdx);
t_ftmMsg* getMsg(t_ftmChain* pChain, uint32_t msgIdx);
t_ftmPage* freePage(t_ftmPage* pPage);

uint16_t    getIdFID(uint64_t id);
uint16_t    getIdGID(uint64_t id);
uint16_t    getIdEVTNO(uint64_t id);
uint16_t    getIdSID(uint64_t id);
uint16_t    getIdBPID(uint64_t id);
uint16_t    getIdSCTR(uint64_t id);
uint64_t    getId(uint16_t fid, uint16_t gid, uint16_t evtno, uint16_t sid, uint16_t bpid, uint16_t sctr);

#endif
