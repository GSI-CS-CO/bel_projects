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

const t_ftmCycle Idle = { .tStart         = 0,
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
                          
uint8_t* uint32ToBytes(uint8_t* pBuf, uint32_t val)
{
   uint8_t i;
   for(i=0;i<FTM_WORD_SIZE;   i++) pBuf[i]  = val >> (8*(FTM_WORD_SIZE-i-1)) & 0xff;
   return pBuf+4;
}

uint8_t* uint64ToBytes(uint8_t* pBuf, uint64_t val)
{
   uint8_t i;
   for(i=0;i<FTM_DWORD_SIZE;   i++) pBuf[i]  = val >> (8*(FTM_DWORD_SIZE-i-1)) & 0xff;
   return pBuf+8;
}


uint32_t bytesToUint32(uint8_t* pBuf)
{
   uint8_t i;
   uint32_t val=0;
   
   for(i=0;i<FTM_WORD_SIZE;   i++) val |= (uint32_t)pBuf[i] << (8*(FTM_WORD_SIZE-i-1));
   return val;
}

uint64_t bytesToUint64(uint8_t* pBuf)
{
   uint8_t i;
   uint64_t val=0;
   
   for(i=0;i<FTM_DWORD_SIZE;   i++) val |= (uint64_t)pBuf[i] << (8*(FTM_DWORD_SIZE-i-1));
   return val;
}

uint8_t* serPage (t_ftmPage*  pPage, uint8_t*    pBufStart, uint32_t embeddedOffs) 
{
   uint8_t j, planIdx, cycIdx;
   uint8_t* pBuf = pBufStart;
   t_ftmCycle* pCyc;
   uint32_t pBufPlans[FTM_PLAN_MAX];
   
   
   pBuf += 4 + FTM_PLAN_MAX * 4 + 4 +4; //leave space for page meta info
   //write all plans to pBuffer
   for(planIdx = 0; planIdx < pPage->planQty; planIdx++)
   {
      pBufPlans[planIdx] = embeddedOffs + ((uint32_t)((uintptr_t)pBuf - (uintptr_t)pBufStart));
      printf("Plan %02u/%02u starts @%08x\n", planIdx, pPage->planQty, pBufPlans[planIdx]);
      pCyc = pPage->plans[planIdx].pStart;
      cycIdx=0;
      while(cycIdx++ < pPage->plans[planIdx].cycQty && pCyc != NULL)
      {
         printf("Cyc %02u/%02u starts @", cycIdx, pPage->plans[planIdx].cycQty);
         pBuf = serCycle(pCyc, pBufStart, pBuf, embeddedOffs);
         pCyc = (t_ftmCycle*)pCyc->pNext;
      }   
   }      
   // now write page meta
   uint32ToBytes(&pBufStart[FTM_PAGE_QTY], pPage->planQty);
   for(j=0;j<pPage->planQty;    j++)
      uint32ToBytes(&pBufStart[FTM_PAGE_PLANPTRS + j*FTM_WORD_SIZE], pBufPlans[j]);
   for(j=pPage->planQty;j<FTM_PLAN_MAX;    j++)
      uint32ToBytes(&pBufStart[FTM_PAGE_PLANPTRS + j*FTM_WORD_SIZE], 0xCAFEBABE);
         
   uint32ToBytes(&pBufStart[FTM_PAGE_IDX_BP],    pPage->idxBp);
   uint32ToBytes(&pBufStart[FTM_PAGE_IDX_START], pPage->idxStart);
      
   return pBuf;   

}


uint8_t* serCycle(t_ftmCycle* pCyc, uint8_t* pBufStart, uint8_t* pBuf, uint32_t embeddedOffs)
{
   uint8_t msgIdx;
   
   uint32_t pBufNext, pBufMsg;
   t_ftmMsg* pMsg = pCyc->pMsg; 
   
   uint64ToBytes(&pBuf[FTM_CYC_TSTART],    pCyc->tStart);
   uint64ToBytes(&pBuf[FTM_CYC_TPERIOD],   pCyc->tPeriod);
   uint64ToBytes(&pBuf[FTM_CYC_TEXEC],     0);
   uint32ToBytes(&pBuf[FTM_CYC_FLAGS],     pCyc->flags);
   uint64ToBytes(&pBuf[FTM_CYC_CONDVAL],   pCyc->condVal);
   uint64ToBytes(&pBuf[FTM_CYC_CONDMSK],   pCyc->condMsk);
   uint32ToBytes(&pBuf[FTM_CYC_SIGDST],    pCyc->sigDst);
   uint32ToBytes(&pBuf[FTM_CYC_SIGVAL],    pCyc->sigVal);
   uint32ToBytes(&pBuf[FTM_CYC_REPQTY],    pCyc->repQty);
   uint32ToBytes(&pBuf[FTM_CYC_REPCNT],    0);
   uint32ToBytes(&pBuf[FTM_CYC_MSGQTY],    pCyc->msgQty);
   uint32ToBytes(&pBuf[FTM_CYC_MSGIDX],    0);
   
   pBufMsg  = embeddedOffs + _FTM_CYC_LEN + ( (uint32_t)( (uintptr_t)pBuf - (uintptr_t)pBufStart ) );
   uint32ToBytes(&pBuf[FTM_CYC_PMSG],    pBufMsg);
   
   pBufNext = pBufMsg + pCyc->msgQty * _FTM_MSG_LEN;
   uint32ToBytes(&pBuf[FTM_CYC_PNEXT],    pBufNext);
   
   pBuf +=  _FTM_CYC_LEN;
   for(msgIdx = 0; msgIdx < pCyc->msgQty; msgIdx++) pBuf = serMsg(&pMsg[msgIdx], pBuf);   
   
   return pBuf;   

}

uint8_t* serMsg(  t_ftmMsg* pMsg, uint8_t* pBuf)
{
   uint64ToBytes(&pBuf[FTM_MSG_ID],     pMsg->id);
   uint64ToBytes(&pBuf[FTM_MSG_PAR],    pMsg->par);
   uint32ToBytes(&pBuf[FTM_MSG_TEF],    pMsg->tef); 
   uint32ToBytes(&pBuf[FTM_MSG_RES],    pMsg->res); 
   uint64ToBytes(&pBuf[FTM_MSG_TS],     0);
   uint64ToBytes(&pBuf[FTM_MSG_OFFS],   pMsg->offs);
   
   return pBuf + _FTM_MSG_LEN;
}


t_ftmPage* deserPage(t_ftmPage* pPage, uint8_t* pBufStart, uint32_t embeddedOffs)
{
   uint8_t* pBuf = pBufStart;
   uint32_t j, cycQty;
   uint8_t* pBufPlans[FTM_PLAN_MAX];
   t_ftmCycle* pCyc      = NULL;
      t_ftmCycle* pNext = NULL;
   
   //printf("deserpage\n");
   //get plan qty
   if(pPage == NULL) printf("page not allocated\n");
   
   pPage->idxStart   = bytesToUint32(&pBufStart[FTM_PAGE_IDX_START]); 
   pPage->idxBp      = bytesToUint32(&pBufStart[FTM_PAGE_IDX_BP]);
   
   pPage->planQty = bytesToUint32(&pBufStart[FTM_PAGE_QTY]);
   for(j=0;j<pPage->planQty;    j++)
   {
      
      //read the offset to start of first cycle
      pBufPlans[j] = pBufStart + (uintptr_t)bytesToUint32(&pBufStart[FTM_PAGE_PLANPTRS + j*FTM_WORD_SIZE]) - (uintptr_t)embeddedOffs;
      
      //allocate first cycle and a Next Cycle
      cycQty = 1;
      pCyc      = calloc(1, sizeof(t_ftmCycle));
      pNext     = calloc(1, sizeof(t_ftmCycle));
      //set plan start to first cycle
      pPage->plans[j].pStart = pCyc;
      //deserialise (pBufStart +  offset) to pCyc and fix pCycs next ptr

      pBuf = deserCycle(pCyc, pNext, pBufPlans[j], pBufStart, embeddedOffs);

 
      //deserialise cycles until we reached the end or we reached max
      
      while(!(pCyc->flags & FLAGS_IS_END) && cycQty < 1024)
      {
         pCyc = pNext;
         pNext = calloc(1, sizeof(t_ftmCycle));
         pBuf = deserCycle(pCyc, pNext, pBuf, pBufStart, embeddedOffs);
         cycQty++;
      } 
      //no next after the end, free this one
      //free(pNext);
      pCyc->pNext = NULL;
      //save to plan how many cycles it contains
      pPage->plans[j].cycQty = cycQty;
   }
   pPage->idxBp      = bytesToUint32(&pBufStart[FTM_PAGE_IDX_BP]);
   pPage->idxStart   = bytesToUint32(&pBufStart[FTM_PAGE_IDX_START]);
 
   return pPage;     
}

uint8_t* deserCycle(t_ftmCycle* pCyc, t_ftmCycle* pNext, uint8_t* pCycStart, uint8_t* pBufStart, uint32_t embeddedOffs)
{
   uint8_t* pBuf = pCycStart;
   uint32_t msgIdx;

      //printf("desercyc\n");
      //printf("pCyc %p pNext %p pBufPLansJ %p pBuf: %p\n", pCyc, pNext, pCycStart, pBufStart); 
   pCyc->tStart   = bytesToUint64(&pCycStart[FTM_CYC_TSTART]);
   pCyc->tPeriod  = bytesToUint64(&pCycStart[FTM_CYC_TPERIOD]);
   pCyc->flags    = bytesToUint32(&pCycStart[FTM_CYC_FLAGS]);
   pCyc->condVal  = bytesToUint64(&pCycStart[FTM_CYC_CONDVAL]);
   pCyc->condMsk  = bytesToUint64(&pCycStart[FTM_CYC_CONDMSK]);
   pCyc->sigDst   = bytesToUint32(&pCycStart[FTM_CYC_SIGDST]);
   pCyc->sigVal   = bytesToUint32(&pCycStart[FTM_CYC_SIGVAL]);
   pCyc->repQty   = bytesToUint32(&pCycStart[FTM_CYC_REPQTY]);
   pCyc->repCnt   = bytesToUint32(&pCycStart[FTM_CYC_REPCNT]);
   pCyc->msgQty   = bytesToUint32(&pCycStart[FTM_CYC_MSGQTY]);
   pCyc->msgIdx   = bytesToUint32(&pCycStart[FTM_CYC_MSGIDX]);
   //allocate space for this cycles messages
   pCyc->pMsg     = calloc(pCyc->msgQty, sizeof(t_ftmMsg));
   pCyc->pNext    = (struct t_ftmCycle*)pNext;
   //deserialise messages until msgQty is reached
   pBuf = &pBuf[_FTM_CYC_LEN];
   for(msgIdx = 0; msgIdx < pCyc->msgQty; msgIdx++) pBuf = deserMsg(pBuf, &pCyc->pMsg[msgIdx]);   
   //set pBuf ptr to 
   return pBufStart + (uintptr_t)bytesToUint32(&pCycStart[FTM_CYC_PNEXT]) - (uintptr_t)embeddedOffs;
   
}

uint8_t* deserMsg(uint8_t* pBuf, t_ftmMsg* pMsg)
{
   //printf("desermsg\n");   
   pMsg->id    = bytesToUint64(&pBuf[FTM_MSG_ID]);
   pMsg->par   = bytesToUint64(&pBuf[FTM_MSG_PAR]); 
   pMsg->tef   = bytesToUint64(&pBuf[FTM_MSG_TEF]);
   pMsg->res   = bytesToUint64(&pBuf[FTM_MSG_RES]);
   pMsg->ts    = bytesToUint64(&pBuf[FTM_MSG_TS]); 
   pMsg->offs  = bytesToUint64(&pBuf[FTM_MSG_OFFS]); 
   
   return pBuf + (uintptr_t)_FTM_MSG_LEN; 
}



//buf + _FTM_MSG_LEN;

void showFtmPage(t_ftmPage* pPage)
{
   uint32_t planIdx, cycIdx, msgIdx;
   t_ftmCycle* pCyc  = NULL;
   t_ftmMsg*   pMsg  = NULL;
   
   printf("---PAGE \n");
   printf("StartPlan:\t%c\nAltPlan:\t%c\n", pPage->idxStart + 'A', pPage->idxBp + 'A');
         
   for(planIdx = 0; planIdx < pPage->planQty; planIdx++)
   {
      printf("\t---PLAN %c\n", planIdx+'A');
      cycIdx = 0;
      pCyc = pPage->plans[planIdx].pStart;
      while(cycIdx++ < pPage->plans[planIdx].cycQty && pCyc != NULL)
      {
         printf("\t\t---CYC %c%u\n", planIdx+'A', cycIdx-1);
         printf("\t\tStart:\t\t%08x%08x\n\t\tperiod:\t\t%08x%08x\n\t\trep:\t\t\t%08x\n\t\tmsg:\t\t\t%08x\n", 
         (uint32_t)(pCyc->tStart>>32), (uint32_t)pCyc->tStart, 
         (uint32_t)(pCyc->tPeriod>>32), (uint32_t)pCyc->tPeriod,
         pCyc->repQty,
         pCyc->msgQty);
         
         printf("\t\tFlags:\t");
         if(pCyc->flags & FLAGS_IS_BP) printf("-IS_BP\t");
         if(pCyc->flags & FLAGS_IS_COND_MSI) printf("-IS_CMSI\t");
         if(pCyc->flags & FLAGS_IS_COND_SHARED) printf("-IS_CSHA\t");
         if(pCyc->flags & FLAGS_IS_SIG) printf("-IS_SIG");
         if(pCyc->flags & FLAGS_IS_END) printf("-IS_END");
         printf("\n");
         
         printf("\t\tCondVal:\t%08x%08x\n\t\tCondMsk:\t%08x%08x\n\t\tSigDst:\t\t\t%08x\n\t\tSigVal:\t\t\t%08x\n", 
         (uint32_t)(pCyc->condVal>>32), (uint32_t)pCyc->condVal, 
         (uint32_t)(pCyc->condMsk>>32), (uint32_t)pCyc->condMsk,
         pCyc->sigDst,
         pCyc->sigVal);  
         
         pMsg = pCyc->pMsg;
         for(msgIdx = 0; msgIdx < pCyc->msgQty; msgIdx++)
         {
            printf("\t\t\t---MSG %c%u%c\n", planIdx+'A', cycIdx-1, msgIdx+'A');
            printf("\t\t\tid:\t%08x%08x\n\t\t\tpar:\t%08x%08x\n\t\t\ttef:\t\t%08x\n\t\t\toffs:\t%08x%08x\n", 
            (uint32_t)(pMsg[msgIdx].id>>32), (uint32_t)pMsg[msgIdx].id, 
            (uint32_t)(pMsg[msgIdx].par>>32), (uint32_t)pMsg[msgIdx].par,
            pMsg[msgIdx].tef,
            (uint32_t)(pMsg[msgIdx].offs>>32), (uint32_t)pMsg[msgIdx].offs);   
         }
         pCyc = (t_ftmCycle*)pCyc->pNext;
      }
           
   }   
   
}






































