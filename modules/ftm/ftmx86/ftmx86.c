#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "ftmx86.h"
#include <etherbone.h>

extern uint8_t cpuId;

uint16_t getIdFID(uint64_t id)     {return ((uint16_t)(id >> ID_FID_POS))     & (ID_MSK_B16 >> (16 - ID_FID_LEN));}
uint16_t getIdGID(uint64_t id)     {return ((uint16_t)(id >> ID_GID_POS))     & (ID_MSK_B16 >> (16 - ID_GID_LEN));}
uint16_t getIdEVTNO(uint64_t id)   {return ((uint16_t)(id >> ID_EVTNO_POS))   & (ID_MSK_B16 >> (16 - ID_EVTNO_LEN));}
uint16_t getIdSID(uint64_t id)     {return ((uint16_t)(id >> ID_SID_POS))     & (ID_MSK_B16 >> (16 - ID_SID_LEN));}
uint16_t getIdBPID(uint64_t id)    {return ((uint16_t)(id >> ID_BPID_POS))    & (ID_MSK_B16 >> (16 - ID_BPID_LEN));}
uint16_t getIdSCTR(uint64_t id)    {return ((uint16_t)(id >> ID_SCTR_POS))    & (ID_MSK_B16 >> (16 - ID_SCTR_LEN));}



uint64_t getId(uint16_t fid, uint16_t gid, uint16_t evtno, uint16_t sid, uint16_t bpid, uint16_t sctr)
{
   uint64_t ret;
   
   //printf("ID: %u %u %u %u %u %u\n", fid, gid, evtno, sid, bpid, sctr);
   ret =    ((uint64_t)fid    << ID_FID_POS)    |
            ((uint64_t)gid    << ID_GID_POS)    |
            ((uint64_t)evtno  << ID_EVTNO_POS)  |
            ((uint64_t)sid    << ID_SID_POS)    |
            ((uint64_t)bpid   << ID_BPID_POS)   |
            ((uint64_t)sctr   << ID_SCTR_POS);

   return ret;         
}

const t_ftmChain Idle = { .tStart         = 0,
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
   for(i=0;i<FTM_WORD_SIZE;   i++) pBuf[i]  = val >> (8*i) & 0xff;
   return pBuf+4;
}

uint8_t* uint64ToBytes(uint8_t* pBuf, uint64_t val)
{
   uint32ToBytes(pBuf+0, (uint32_t)(val>>32));
   uint32ToBytes(pBuf+4, (uint32_t)val);
   return pBuf+8;
}


uint32_t bytesToUint32(uint8_t* pBuf)
{
   uint8_t i;
   uint32_t val=0;
   
   for(i=0;i<FTM_WORD_SIZE;   i++) val |= (uint32_t)pBuf[i] << (8*i);
   return val;
}

uint64_t bytesToUint64(uint8_t* pBuf)
{
   uint64_t val=0;
   
   
   val |= (uint64_t)bytesToUint32(pBuf+0)<<32;
   val |= (uint64_t)bytesToUint32(pBuf+4);
   return val;
}

uint8_t* serPage (t_ftmPage*  pPage, uint8_t*    pBufStart, uint32_t embeddedOffs) 
{
   uint8_t j, planIdx, chainIdx;
   uint8_t* pBuf = pBufStart;
   t_ftmChain* pChain;
   uint32_t pBufPlans[FTM_PLAN_MAX];
   
    pBuf += FTM_PAGEMETA;
    //leave space for page meta info
   //write all plans to pBuffer
   for(planIdx = 0; planIdx < pPage->planQty; planIdx++)
   {
      pBufPlans[planIdx] = embeddedOffs + ((uint32_t)((uintptr_t)pBuf - (uintptr_t)pBufStart));
      //printf("Plan %02u/%02u starts @%08x\n", planIdx, pPage->planQty, pBufPlans[planIdx]);
      pChain = pPage->plans[planIdx].pStart;
      chainIdx=0;
      while(chainIdx++ < pPage->plans[planIdx].chainQty && pChain != NULL)
      {
         //printf("Chain %02u/%02u starts @", chainIdx, pPage->plans[planIdx].chainQty);
         pBuf = serChain(pChain, pBufStart, pBuf, embeddedOffs);
         pChain = (t_ftmChain*)pChain->pNext;
      }   
   }
   
   // now write page meta
  
   uint32ToBytes(&pBufStart[FTM_PAGE_QTY], pPage->planQty);
   for(j=0;j<pPage->planQty;    j++)
      uint32ToBytes(&pBufStart[FTM_PAGE_PLANPTRS + j*FTM_WORD_SIZE], pBufPlans[j]);
   for(j=pPage->planQty;j<FTM_PLAN_MAX;    j++)
      uint32ToBytes(&pBufStart[FTM_PAGE_PLANPTRS + j*FTM_WORD_SIZE], FTM_NULL);
   
   
   if( pPage->idxBp == 0xdeadbeef)    pPage->pBp = FTM_IDLE_OFFSET;
   else pPage->pBp     = pBufPlans[pPage->idxBp];
   
   if( pPage->idxStart == 0xdeadbeef) pPage->pStart = FTM_IDLE_OFFSET;
   else pPage->pStart  = pBufPlans[pPage->idxStart];
   
   //printf("BP: %08x, Start: %08x\n", pPage->pBp, pPage->pStart);
   
    uint32ToBytes(&pBufStart[FTM_PAGE_PTR_BP],         pPage->pBp);
   uint32ToBytes(&pBufStart[FTM_PAGE_PTR_START],      pPage->pStart);
 
   uint32ToBytes(&pBufStart[FTM_PAGE_PTR_SHAREDMEM], pPage->pSharedMem);
   
   return pBuf;   

}


uint8_t* serChain(t_ftmChain* pChain, uint8_t* pBufStart, uint8_t* pBuf, uint32_t embeddedOffs)
{
   uint8_t msgIdx;
   
   uint32_t pBufNext, pBufMsg, sigDst, condSrc;
   t_ftmMsg* pMsg = pChain->pMsg; 
   
   sigDst = 0;
   condSrc = 0;
   
   
   //FIXME: change to proper sdb find
   if (pChain->flags & FLAGS_IS_SIG_MSI)           sigDst = 0x40000800 + pChain->sigCpu * 0x100;
   else if (pChain->flags & FLAGS_IS_SIG_SHARED)   sigDst = 0x40001000 + pChain->sigCpu * 0xC;
   else if (pChain->flags & FLAGS_IS_SIG_ADR)      sigDst = pChain->sigDst;
   
   //FIXME: change to proper sdb find
   if (pChain->flags & FLAGS_IS_COND_MSI)          condSrc = 0x0;
   else if (pChain->flags & FLAGS_IS_COND_SHARED)  condSrc = 0x40001000 + cpuId * 0xC;
   else if (pChain->flags & FLAGS_IS_COND_ADR)     condSrc = pChain->condSrc;
   
   uint64ToBytes(&pBuf[FTM_CHAIN_TSTART],    pChain->tStart);
   uint64ToBytes(&pBuf[FTM_CHAIN_TPERIOD],   pChain->tPeriod);
   uint64ToBytes(&pBuf[FTM_CHAIN_TEXEC],     0);
   uint32ToBytes(&pBuf[FTM_CHAIN_FLAGS],     pChain->flags);
   uint32ToBytes(&pBuf[FTM_CHAIN_CONDSRC],   condSrc);
   uint32ToBytes(&pBuf[FTM_CHAIN_CONDVAL],   pChain->condVal);
   uint32ToBytes(&pBuf[FTM_CHAIN_CONDMSK],   pChain->condMsk);
   uint32ToBytes(&pBuf[FTM_CHAIN_SIGDST],    sigDst);
   uint32ToBytes(&pBuf[FTM_CHAIN_SIGVAL],    pChain->sigVal);
   uint32ToBytes(&pBuf[FTM_CHAIN_REPQTY],    pChain->repQty);
   uint32ToBytes(&pBuf[FTM_CHAIN_REPCNT],    0);
   uint32ToBytes(&pBuf[FTM_CHAIN_MSGQTY],    pChain->msgQty);
   uint32ToBytes(&pBuf[FTM_CHAIN_MSGIDX],    0);
   
   pBufMsg  = embeddedOffs + _FTM_CHAIN_LEN + ( (uint32_t)( (uintptr_t)pBuf - (uintptr_t)pBufStart ) );
   uint32ToBytes(&pBuf[FTM_CHAIN_PMSG],    pBufMsg);
   
   if(pChain->pNext != NULL) pBufNext = pBufMsg + pChain->msgQty * _FTM_MSG_LEN;
   else                     pBufNext = FTM_NULL;
   uint32ToBytes(&pBuf[FTM_CHAIN_PNEXT],    pBufNext);
   
   pBuf +=  _FTM_CHAIN_LEN;
   for(msgIdx = 0; msgIdx < pChain->msgQty; msgIdx++) pBuf = serMsg(&pMsg[msgIdx], pBuf);   
   
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
   uint32_t j, chainQty;
   uint8_t* pBufPlans[FTM_PLAN_MAX];
   t_ftmChain* pChain  = NULL;
   t_ftmChain* pNext = NULL;
   
   
   //get plan qty
   if(pPage == NULL) printf("page not allocated\n");
   
   
   
   pPage->planQty = bytesToUint32(&pBufStart[FTM_PAGE_QTY]);
   for(j=0;j<pPage->planQty;    j++)
   {
      
      //read the offset to start of first chain
      pBufPlans[j] = pBufStart + (uintptr_t)bytesToUint32(&pBufStart[FTM_PAGE_PLANPTRS + j*FTM_WORD_SIZE]) - (uintptr_t)embeddedOffs;
      
      //allocate first chain and a Next Chain
      chainQty = 1;
      pChain      = calloc(1, sizeof(t_ftmChain));
      pNext     = calloc(1, sizeof(t_ftmChain));
      //set plan start to first chain
      pPage->plans[j].pStart = pChain;
      //deserialise (pBufStart +  offset) to pChain and fix pChains next ptr

      pBuf = deserChain(pChain, pNext, pBufPlans[j], pBufStart, embeddedOffs);
      
 
      //deserialise chains until we reached the end or we reached max
      
      while(!(pChain->flags & FLAGS_IS_END) && chainQty < 1024)
      {
         pChain = pNext;
         pNext = calloc(1, sizeof(t_ftmChain));
         pBuf = deserChain(pChain, pNext, pBuf, pBufStart, embeddedOffs);
         chainQty++;
      } 
      //printf("deserpage\n");
      //no next after the end, free this one
      //free(pNext);
      pChain->pNext = NULL;
      //save to plan how many chains it contains
      pPage->plans[j].chainQty = chainQty;
   }
   
   pPage->pBp        = bytesToUint32(&pBufStart[FTM_PAGE_PTR_BP]); 
   pPage->pStart     = bytesToUint32(&pBufStart[FTM_PAGE_PTR_START]);
   
   if       (pPage->pBp    == FTM_NULL)         { pPage->idxBp = 0xcafebabe;}
   else if  (pPage->pBp    == FTM_IDLE_OFFSET)  {pPage->idxBp  = 0xdeadbeef;}
   else for (j=0;j<pPage->planQty;    j++)      
   {
      if(pBufPlans[j]-pBufStart == ((uintptr_t)pPage->pBp - (uintptr_t)embeddedOffs)) {pPage->idxBp = j;}
   }  
      
   if       (pPage->pStart == FTM_NULL)           {pPage->idxStart = 0xcafebabe;}
   else if  (pPage->pStart == FTM_IDLE_OFFSET)    {pPage->idxStart = 0xdeadbeef;}
   else for(j=0;j<pPage->planQty;    j++) 
   {
      if(pBufPlans[j]-pBufStart == ((uintptr_t)pPage->pStart - (uintptr_t)embeddedOffs)) {pPage->idxStart = j;}
   }    
   //pPage->pSharedMem = pBufStart[FTM_PAGE_PTR_SHAREDMEM];    
      
   return pPage;     
}

uint8_t* deserChain(t_ftmChain* pChain, t_ftmChain* pNext, uint8_t* pChainStart, uint8_t* pBufStart, uint32_t embeddedOffs)
{
   uint8_t* pBuf = pChainStart;
   uint32_t msgIdx;

      //printf("deserchain\n");
      //printf("pChain %p pNext %p pBufPLansJ %p pBuf: %p\n", pChain, pNext, pChainStart, pBufStart); 
   pChain->tStart   = bytesToUint64(&pChainStart[FTM_CHAIN_TSTART]);
   pChain->tPeriod  = bytesToUint64(&pChainStart[FTM_CHAIN_TPERIOD]);
   pChain->flags    = bytesToUint32(&pChainStart[FTM_CHAIN_FLAGS]);
   pChain->condVal  = bytesToUint32(&pChainStart[FTM_CHAIN_CONDVAL]);
   pChain->condMsk  = bytesToUint32(&pChainStart[FTM_CHAIN_CONDMSK]);
   pChain->sigDst   = bytesToUint32(&pChainStart[FTM_CHAIN_SIGDST]);
   pChain->sigVal   = bytesToUint32(&pChainStart[FTM_CHAIN_SIGVAL]);
   pChain->repQty   = bytesToUint32(&pChainStart[FTM_CHAIN_REPQTY]);
   pChain->repCnt   = bytesToUint32(&pChainStart[FTM_CHAIN_REPCNT]);
   pChain->msgQty   = bytesToUint32(&pChainStart[FTM_CHAIN_MSGQTY]);
   pChain->msgIdx   = bytesToUint32(&pChainStart[FTM_CHAIN_MSGIDX]);
   //allocate space for this chains messages
   pChain->pMsg     = calloc(pChain->msgQty, sizeof(t_ftmMsg));
   pChain->pNext    = (struct t_ftmChain*)pNext;
   //deserialise messages until msgQty is reached
   pBuf = &pBuf[_FTM_CHAIN_LEN];
   for(msgIdx = 0; msgIdx < pChain->msgQty; msgIdx++) pBuf = deserMsg(pBuf, &pChain->pMsg[msgIdx]);   
   //set pBuf ptr to 
   return pBufStart + (uintptr_t)bytesToUint32(&pChainStart[FTM_CHAIN_PNEXT]) - (uintptr_t)embeddedOffs;
   
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



void showFtmPage(t_ftmPage* pPage)
{
   uint32_t planIdx, chainIdx, msgIdx;
   t_ftmChain* pChain  = NULL;
   t_ftmMsg*   pMsg  = NULL;
   
   printf("---PAGE \n");
   printf("StartPlan:\t");
   
   if(pPage->idxStart == 0xdeadbeef) printf("idle\n");
   else { 
          if(pPage->idxStart == 0xcafebabe) printf("NULL\n");
          else printf("%c\n", pPage->idxStart + 'A');
        } 
   
   printf("AltPlan:\t");
   if(pPage->idxBp == 0xdeadbeef) printf("idle\n");
   else { 
          if(pPage->idxBp == 0xcafebabe) printf("NULL\n");
          else printf("%c\n", pPage->idxBp + 'A');
        }  
   
         
   for(planIdx = 0; planIdx < pPage->planQty; planIdx++)
   {
      printf("\t---PLAN %c\n", planIdx+'A');
      chainIdx = 0;
      pChain = pPage->plans[planIdx].pStart;
      while(chainIdx++ < pPage->plans[planIdx].chainQty && pChain != NULL)
      {
         printf("\t\t---CHAIN %c%u\n", planIdx+'A', chainIdx-1);
         printf("\t\tStart:\t\t%08x%08x\n\t\tperiod:\t\t%08x%08x\n\t\trep:\t\t\t%08x\n\t\tmsg:\t\t\t%08x\n", 
         (uint32_t)(pChain->tStart>>32), (uint32_t)pChain->tStart, 
         (uint32_t)(pChain->tPeriod>>32), (uint32_t)pChain->tPeriod,
         pChain->repQty,
         pChain->msgQty);
         
         printf("\t\tFlags:\t");
         if(pChain->flags & FLAGS_IS_BP) printf("-IS_BP\t");
         if(pChain->flags & FLAGS_IS_COND_MSI) printf("-IS_CMSI\t");
         if(pChain->flags & FLAGS_IS_COND_SHARED) printf("-IS_CSHA\t");
         if(pChain->flags & FLAGS_IS_SIG_SHARED) printf("-IS_SIG_SHARED");
         if(pChain->flags & FLAGS_IS_SIG_MSI)    printf("-IS_SIG_MSI");
         if(pChain->flags & FLAGS_IS_END) printf("-IS_END");
         printf("\n");
         
         printf("\t\tCondVal:\t%08x\n\t\tCondMsk:\t%08x\n\t\tSigDst:\t\t\t%08x\n\t\tSigVal:\t\t\t%08x\n", 
         (uint32_t)pChain->condVal, 
         (uint32_t)pChain->condMsk,
         pChain->sigDst,
         pChain->sigVal);  
         
         pMsg = pChain->pMsg;
         
         for(msgIdx = 0; msgIdx < pChain->msgQty; msgIdx++)
         {
            printf("\t\t\t---MSG %c%u%c\n", planIdx+'A', chainIdx-1, msgIdx+'A');
           
            printf("\t\t\tid:\t%08x%08x\n\t\t\tpar:\t%08x%08x\n\t\t\ttef:\t\t%08x\n\t\t\toffs:\t%08x%08x\n", 
            (uint32_t)(pMsg[msgIdx].id>>32), (uint32_t)pMsg[msgIdx].id, 
            (uint32_t)(pMsg[msgIdx].par>>32), (uint32_t)pMsg[msgIdx].par,
            pMsg[msgIdx].tef,
            (uint32_t)(pMsg[msgIdx].offs>>32), (uint32_t)pMsg[msgIdx].offs);   
         }
         pChain = (t_ftmChain*)pChain->pNext;
      }
           
   }   
   
}






































