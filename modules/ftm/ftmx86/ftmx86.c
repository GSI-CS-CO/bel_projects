#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "ftmx86.h"

t_FtmIf*       pFtmIf;

uint16_t getIdFID(uint64_t id)     {return ((uint16_t)(id >> ID_FID_POS))     & (ID_MSK_B16 >> (16 - ID_FID_LEN));}
uint16_t getIdGID(uint64_t id)     {return ((uint16_t)(id >> ID_GID_POS))     & (ID_MSK_B16 >> (16 - ID_GID_LEN));}
uint16_t getIdEVTNO(uint64_t id)   {return ((uint16_t)(id >> ID_EVTNO_POS))   & (ID_MSK_B16 >> (16 - ID_EVTNO_LEN));}
uint16_t getIdSID(uint64_t id)     {return ((uint16_t)(id >> ID_SID_POS))     & (ID_MSK_B16 >> (16 - ID_SID_LEN));}
uint16_t getIdBPID(uint64_t id)    {return ((uint16_t)(id >> ID_BPID_POS))    & (ID_MSK_B16 >> (16 - ID_BPID_LEN));}
uint16_t getIdSCTR(uint64_t id)    {return ((uint16_t)(id >> ID_SCTR_POS))    & (ID_MSK_B16 >> (16 - ID_SCTR_LEN));}



uint64_t getId(uint16_t fid, uint16_t gid, uint16_t evtno, uint16_t sid, uint16_t bpid, uint16_t sctr)
{
   uint64_t ret;
   
   ret =    ((uint64_t)fid    << ID_FID_POS)    |
            ((uint64_t)gid    << ID_GID_POS)    |
            ((uint64_t)evtno  << ID_EVTNO_POS)  |
            ((uint64_t)sid    << ID_SID_POS)    |
            ((uint64_t)bpid   << ID_BPID_POS)   |
            ((uint64_t)sctr   << ID_SCTR_POS);

   return ret;         
}

const t_ftmCycle Idle = { .tTrn           = 0,
                          .tStart         = 0,
                          .tPeriod        = 500,
                          .tExec          = 0,
                          .flags          = (FLAGS_IS_BP),
                          .condVal        = 0,
                          .condMsk        = 0,
                          .sigDst         = 0,
                          .sigVal         = 0,
                          .repQty         = -1,
                          .repCnt         = 0,
                          .msgQty         = 0,
                          .msgIdx         = 0,
                          .pMsg           = NULL,
                          .pNext          = NULL
                          };


//   pMsg  = (uint32_t)(uint32_t*)(buf - bufStart + FTM_CYC_LEN); 
//   pNext = (uint32_t)(uint32_t*)(buf - bufStart + FTM_CYC_LEN + msgQty * FTM_MSG_LEN);

uint8_t* serCycle(uint8_t*    buf, 
                  uint64_t    tTrn,
                  uint64_t    tStart,
                  uint64_t    tPeriod,
                  uint64_t    tExec,       
                  uint32_t    flags,
                  uint64_t    condVal,
                  uint64_t    condMsk,
                  uint32_t    sigDst, 
                  uint32_t    sigVal,       
                  uint32_t    repQty,        
                  uint32_t    repCnt,
                  uint32_t    msgQty,
                  uint32_t    msgIdx,
                  uint32_t    pMsg,
                  uint32_t    pNext)
{
   uint8_t i;
   
   
   
   for(i=0;i<FTM_TIME_SIZE;   i++) buf[FTM_CYC_TTRN         + i]  = (tTrn           >> (8*i)) & 0xff;
   for(i=0;i<FTM_TIME_SIZE;   i++) buf[FTM_CYC_TSTART       + i]  = (tStart         >> (8*i)) & 0xff;
   for(i=0;i<FTM_TIME_SIZE;   i++) buf[FTM_CYC_TPERIOD      + i]  = (tPeriod        >> (8*i)) & 0xff;
   for(i=0;i<FTM_TIME_SIZE;   i++) buf[FTM_CYC_TEXEC        + i]  = (tExec          >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_CYC_FLAGS        + i]  = (flags          >> (8*i)) & 0xff;
   for(i=0;i<FTM_DWORD_SIZE;  i++) buf[FTM_CYC_CONDVAL      + i]  = (condVal        >> (8*i)) & 0xff;
   for(i=0;i<FTM_DWORD_SIZE;  i++) buf[FTM_CYC_CONDMSK      + i]  = (condMsk        >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_CYC_SIGDST       + i]  = (sigDst         >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_CYC_SIGVAL       + i]  = (sigVal         >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_CYC_REPQTY       + i]  = (repQty         >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_CYC_REPCNT       + i]  = (repCnt         >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_CYC_MSGQTY       + i]  = (msgQty         >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_CYC_MSGIDX       + i]  = (msgIdx         >> (8*i)) & 0xff;
   
   for(i=0;i<FTM_PTR_SIZE;    i++) buf[FTM_CYC_PMSG         + i]  = (pMsg           >> (8*i)) & 0xff;
   for(i=0;i<FTM_PTR_SIZE;    i++) buf[FTM_CYC_PNEXT        + i]  = (pNext          >> (8*i)) & 0xff;
   
   return (buf + FTM_CYC_LEN);   

}



uint8_t* serMsg(  uint8_t* buf, 
                  uint64_t id,
                  uint64_t par,
                  uint32_t tef,
                  uint32_t res,
                  uint64_t ts,
                  uint64_t offs
                  )
{
   uint8_t i;
   for(i=0;i<FTM_DWORD_SIZE;  i++) buf[FTM_MSG_ID     + i]  = (id    >> (8*i)) & 0xff;
   for(i=0;i<FTM_DWORD_SIZE;  i++) buf[FTM_MSG_PAR    + i]  = (par   >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_MSG_TEF    + i]  = (tef   >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_MSG_RES    + i]  = (res   >> (8*i)) & 0xff;
   for(i=0;i<FTM_TIME_SIZE;   i++) buf[FTM_MSG_TS     + i]  = (ts    >> (8*i)) & 0xff;
   for(i=0;i<FTM_TIME_SIZE;   i++) buf[FTM_MSG_OFFS   + i]  = (offs  >> (8*i)) & 0xff;
   
   return buf + FTM_MSG_LEN;
}









void showPage(t_ftmPage* pPage)
{
  
}







































