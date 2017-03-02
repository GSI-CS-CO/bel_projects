////////////////////////////////////////////////////////////////
//Triple Command Queue Structure
////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "cmdq.h"  

//cannot use pointers directly, as we don't know if the queues were placed aligned. use (masked) counters instead that can overflow

const uint8_t cOff[4] = {CMDQ_LO_OFF / 4, CMDQ_HI_OFF / 4, CMDQ_IL_OFF /4, CMDQ_IL_OFF /4};

const uint8_t sIL[] = "INTERLOCK";
const uint8_t sHI[] = "High";
const uint8_t sLO[] = "low";

const uint8_t *str[3] = {sLO, sHI, sIL};

uint32_t createFlow(uint32_t nextIdx, uint32_t qty)
{
   return (ACT_TYPE_FLOW | ((nextIdx << ACT_FLOW_NEXT_POS) & ACT_FLOW_NEXT_MSK) | ((qty << ACT_FNF_QTY_POS) & ACT_FNF_QTY_MSK));  
}

uint32_t createNop(uint32_t qty)
{
   return (ACT_TYPE_NOP | ((0 << ACT_FLOW_NEXT_POS) & ACT_FLOW_NEXT_MSK) | ((qty << ACT_FNF_QTY_POS) & ACT_FNF_QTY_MSK));  
}

uint32_t createFlush(uint8_t flushHi, uint8_t flushIdxHi, uint8_t flushLo, uint8_t flushIdxLo)
{
   return (ACT_TYPE_FLUSH | ((flushHi & 1) << ACT_FLUSH_HI_POS) | ((flushIdxHi & CMDQ_IDX_OF_MSK) << ACT_FLUSH_HI_RNG_POS)
         | ((flushLo & 1) << ACT_FLUSH_LO_POS) | ((flushIdxLo & CMDQ_IDX_OF_MSK) << ACT_FLUSH_LO_RNG_POS) );  
}

uint32_t *pushQ(uint32_t *pQs, uint32_t priority, uint32_t action, uint64_t time)
{
  uint32_t *pQ, *pCmd;
  uint8_t wrIdx, rdIdx;
  
  pQ      = pQs + cOff[priority];
  wrIdx   = *(uint32_t*)(pQ   + (CMDQ_WR_OFF  >> 2));
  rdIdx   = *(uint32_t*)(pQ   + (CMDQ_RD_OFF  >> 2));
  pCmd    =  (uint32_t*)(pQ   + ((CMDQ_BUF_OFF + (wrIdx & CMDQ_IDX_MSK) * _CMD_SIZE) >> 2)); //get ptr to current command

  //check if full (rdidx equals wridx and OF bits differ)
  if (((rdIdx & CMDQ_IDX_MSK) == (wrIdx & CMDQ_IDX_MSK)) && ((rdIdx & CMDQ_IDX_OF_MSK) != (wrIdx & CMDQ_IDX_OF_MSK))) {
    printf("### %s: Queue is full!\n", str[priority]);
  } else {
    *(uint64_t*)(pCmd + (CMD_TS_OFF  >> 2)) = time;
    *(uint32_t*)(pCmd + (CMD_ACT_OFF >> 2)) = action;
    *(uint32_t*)(pQ   + (CMDQ_WR_OFF >> 2)) = (wrIdx + 1) & CMDQ_IDX_OF_MSK;
  }
  return pQs;
}

uint8_t getWrIdx(uint32_t *pQs, uint32_t priority) {
  return (*(uint8_t*)(pQs + cOff[priority] + (CMDQ_WR_OFF  >> 2)) & CMDQ_IDX_OF_MSK);
}

uint8_t getRdIdx(uint32_t *pQs, uint32_t priority) {
  return (*(uint8_t*)(pQs + cOff[priority] + (CMDQ_RD_OFF  >> 2)) & CMDQ_IDX_OF_MSK);
}


int32_t checkQs(uint32_t *pQs, int32_t idxnext, uint64_t now) {


  uint8_t rdIdx, qIlNotEmpty, qHiNotEmpty, qLoNotEmpty, qIlDue, qHiDue, qLoDue, qty, queueIdx;
  uint32_t type;
  uint32_t *pCmd = NULL, *pAct = NULL, *pQ = NULL;
  uint64_t ts;
//compare rd/wr indices to select Il or Hi Queue if they contain elements, Lo is default

  //masking all indices with CMDQ_IDX_OF_MSK would be safer, but takes a lot of time. leave out for now.
  //We rely on host writing conformant indices and mask our own updates
  printf("  ILWR: %x HIWR: %x LOWR: %x\n", *(pQs + (CMDQ_IL_WR_OFF >> 2)), *(pQs + (CMDQ_HI_WR_OFF >> 2)), *(pQs + (CMDQ_LO_WR_OFF >> 2)));
  printf("  ILRD: %x HIRD: %x LORD: %x\n", *(pQs + (CMDQ_IL_RD_OFF >> 2)), *(pQs + (CMDQ_HI_RD_OFF >> 2)), *(pQs + (CMDQ_LO_RD_OFF >> 2)));

  qIlNotEmpty = ( *(pQs + (CMDQ_IL_WR_OFF >> 2)) != *(pQs + (CMDQ_IL_RD_OFF >> 2)) );
  qHiNotEmpty = ( *(pQs + (CMDQ_HI_WR_OFF >> 2)) != *(pQs + (CMDQ_HI_RD_OFF >> 2)) );
  qLoNotEmpty = ( *(pQs + (CMDQ_LO_WR_OFF >> 2)) != *(pQs + (CMDQ_LO_RD_OFF >> 2)) );

  printf("  IL: %d HI: %d LO: %d\n", qIlNotEmpty, qHiNotEmpty, qLoNotEmpty);

  if (qIlNotEmpty | qHiNotEmpty | qLoNotEmpty) {   //for performance

    

    qIlDue = *(uint64_t*)(pQs + ((CMDQ_IL_BUF_OFF + CMD_TS_OFF + (*(pQs + (CMDQ_IL_RD_OFF >> 2)) & CMDQ_IDX_MSK) * _CMD_SIZE) >> 2)) <= now;
    qHiDue = *(uint64_t*)(pQs + ((CMDQ_HI_BUF_OFF + CMD_TS_OFF + (*(pQs + (CMDQ_HI_RD_OFF >> 2)) & CMDQ_IDX_MSK) * _CMD_SIZE) >> 2)) <= now;
    qLoDue = *(uint64_t*)(pQs + ((CMDQ_LO_BUF_OFF + CMD_TS_OFF + (*(pQs + (CMDQ_LO_RD_OFF >> 2)) & CMDQ_IDX_MSK) * _CMD_SIZE) >> 2)) <= now;
    


    printf("  IL: %d HI: %d LO: %d ILD: %d HID: %d LOD: %d\n", qIlNotEmpty, qHiNotEmpty, qLoNotEmpty, qIlDue, qHiDue, qLoDue);

    queueIdx    = (qIlNotEmpty & qIlDue) + (qHiNotEmpty  & qHiDue);

    //check if any elements were found and are due
    if (queueIdx + (qLoNotEmpty & qLoDue)) {

      

      //now     = getSysTime();

      printf("!!! %s: Elements found, ", str[queueIdx]);

      pQ      = pQs + cOff[queueIdx];                                          //get ptr to non empty queue of highest priority
      rdIdx   = *(uint32_t*)(pQ   + (CMDQ_RD_OFF >> 2));                      //get read idx
      pCmd    =  (uint32_t*)(pQ   + ((CMDQ_BUF_OFF + (rdIdx & CMDQ_IDX_MSK) * _CMD_SIZE) >> 2)); //get ptr to current command
      ts      = *(uint64_t*)(pCmd + (CMD_TS_OFF   >> 2));                      //get current due time
      pAct    =  (uint32_t*)(pCmd + (CMD_ACT_OFF >> 2));                      //get ptr to current action

     /*     
      //Check if element is Due
      if (now < ts) {
      //not due
        printf("NOT due, dl %llu, now %llu\n", (long long unsigned int)ts, (long long unsigned int)now);
      } else {
  */
        //due
        //Get type
        printf("due, 0x%08x, dl %llu, now %llu\n", *pAct, (long long unsigned int)ts, (long long unsigned int)now);

        type = *pAct & CMD_TYPE_MSK;

        //Flow or Nop?
        printf("Act: 0x%08x Type: 0x%08x\n", *pAct, type);

        if (type == ACT_TYPE_FLOW) {
          idxnext = ((*pAct & ACT_FLOW_NEXT_MSK) >> ACT_FLOW_NEXT_POS);
        }

        if (type == ACT_TYPE_FLUSH) {
          printf("Flush\n");
          if ((queueIdx == PRIO_IL) && (*pAct & ACT_FLUSH_HI_MSK)) { //Flush of HI Q requested? Allowed from IL Q
            printf("Flush HI Q up to Idx %x\n", ((*pAct & ACT_FLUSH_HI_RNG_MSK) >> ACT_FLUSH_HI_RNG_POS) & CMDQ_IDX_OF_MSK);
            *((pQs + cOff[PRIO_HI]) + (CMDQ_RD_OFF >> 2)) = ((*pAct & ACT_FLUSH_HI_RNG_MSK) >> ACT_FLUSH_HI_RNG_POS) & CMDQ_IDX_OF_MSK;
          }
          if (((queueIdx == PRIO_IL) | (queueIdx == PRIO_HI)) && (*pAct & ACT_FLUSH_LO_MSK)) { //Flush of LO Q requested? Allowed from IL Q and HI Q
            printf("Flush LO Q up to Idx %x\n", ((*pAct & ACT_FLUSH_LO_RNG_MSK) >> ACT_FLUSH_LO_RNG_POS) & CMDQ_IDX_OF_MSK);
            *((pQs + cOff[PRIO_LO]) + (CMDQ_RD_OFF >> 2)) = ((*pAct & ACT_FLUSH_LO_RNG_MSK) >> ACT_FLUSH_LO_RNG_POS) & CMDQ_IDX_OF_MSK;
          }
          qty = 1;
        } else {
          qty  = *pAct & ACT_FNF_QTY_MSK; 
        }
        
        
        if (qty <= 1) {
          //pop Q
          printf("Qty %u, popping queue\n", qty);
          *(pQ + (CMDQ_RD_OFF >> 2)) += 1;
          *(pQ + (CMDQ_RD_OFF >> 2)) &= CMDQ_IDX_OF_MSK; //trim by mask
        } else {
          //decrease generator
          printf("Qty %u, yield Generator\n", qty);
          *pAct &= ~ACT_FNF_QTY_MSK;
          *pAct |= qty-1;
        }
        
     // } // due
    } else {
    // no elements in cmdq
      printf("*** No Elements\n");
    }
  }
  return idxnext;
}





