////////////////////////////////////////////////////////////////
//Triple Command Queue Structure
////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "cmdq.h"  

//cannot use pointers directly, as we don't know if the queues were placed aligned. use (masked) counters instead that can overflow

const uint8_t cOff[4] = {CMDQ_LO_OFF / 4, CMDQ_HI_OFF / 4, CMDQ_IL_OFF /4, CMDQ_IL_OFF /4};

uint32_t createFlow(uint32_t nextIdx, uint32_t qty)
{
   return (ACT_TYPE_FLOW << ACT_TYPE_POS) | ((nextIdx << ACT_FLOW_NEXT_POS) & ACT_FLOW_NEXT_MSK) | ((qty << ACT_FNF_QTY_POS) & ACT_FNF_QTY_MSK);  
}

uint32_t createNop(uint32_t qty)
{
   return (ACT_TYPE_NOP << ACT_TYPE_POS) | ((0 << ACT_FLOW_NEXT_POS) & ACT_FLOW_NEXT_MSK) | ((qty << ACT_FNF_QTY_POS) & ACT_FNF_QTY_MSK);  
}

uint32_t createFlush(uint32_t qty)
{
   return (ACT_TYPE_NOP << ACT_TYPE_POS) | ((0 << ACT_FLOW_NEXT_POS) & ACT_FLOW_NEXT_MSK) | ((qty << ACT_FNF_QTY_POS) & ACT_FNF_QTY_MSK);  
}

uint32_t *pushQ(uint32_t *pQs, uint32_t priority, uint32_t action, uint64_t time)
{
  uint32_t *pQ, *pCmd;
  uint8_t wrIdx;
  
  pQ      = pQs + cOff[priority];
  wrIdx   = *(uint32_t*)(pQ   + (CMDQ_WR_OFF  >> 2));       
  pCmd    =  (uint32_t*)(pQ   + ((CMDQ_BUF_OFF + (wrIdx & CMDQ_IDX_MSK) * _CMD_SIZE) >> 2)); //get ptr to current command

  *(uint64_t*)(pCmd + (CMD_TS_OFF   >> 2)) = time;
  *(uint32_t*)(pCmd + (CMD_ACT_OFF  >> 2)) = action;
  *(uint32_t*)(pQ   + (CMDQ_WR_OFF  >> 2)) = (*(uint32_t*)(pQ   + (CMDQ_WR_OFF  >> 2)) + 1) & CMDQ_IDX_MSK;

  return pQs;
}




int32_t checkQs(uint32_t *pQs, int32_t idxnext, uint64_t now) {


  uint8_t rdIdx, qIlNotEmpty, qHiNotEmpty, qLoNotEmpty, qty, type, queueIdx;
  uint32_t *pCmd = NULL, *pAct = NULL, *pQ = NULL;
  uint64_t ts;
//compare rd/wr indices to select Il or Hi Queue if they contain elements, Lo is default

  //masking all indices with CMDQ_IDX_OF_MSK would be safer, but takes a lot of time. leave out for now.
  //We rely on host writing conformant indices and mask our own updates
  qIlNotEmpty = ( *(pQs + (CMDQ_IL_WR_OFF >> 2)) != *(pQs + (CMDQ_IL_RD_OFF >> 2)) );
  qHiNotEmpty = ( *(pQs + (CMDQ_HI_WR_OFF >> 2)) != *(pQs + (CMDQ_HI_RD_OFF >> 2)) );
  qLoNotEmpty = ( *(pQs + (CMDQ_LO_WR_OFF >> 2)) != *(pQs + (CMDQ_LO_RD_OFF >> 2)) );
  queueIdx    = (qIlNotEmpty << 1) + qHiNotEmpty;

  //check if any elements were found
  if (queueIdx + qLoNotEmpty) {

    //now     = getSysTime();
    printf("Elements found in Q %u\n", queueIdx);

    pQ      = pQ + cOff[queueIdx];                                          //get ptr to non empty queue of highest priority
    rdIdx   = *(uint32_t*)(pQ   + (CMDQ_RD_OFF >> 2));                      //get read idx
    pCmd    =  (uint32_t*)(pQ   + ((CMDQ_BUF_OFF + (rdIdx & CMDQ_IDX_MSK) * _CMD_SIZE) >> 2)); //get ptr to current command
    ts      = *(uint64_t*)(pCmd + (CMD_TS_OFF   >> 2));                      //get current due time
    pAct    =  (uint32_t*)(pCmd + (CMD_ACT_OFF >> 2));                      //get ptr to current action

    //Check if element is Due
    if (now < ts) {
    //not due

    } else {
      //due
      //Get type
      printf("Element due, dl %llu, now %llu\n", (long long unsigned int)ts, (long long unsigned int)now);

      type = *pAct & CMD_TYPE_MSK;

      if (type == ACT_TYPE_FLOW) {
        idxnext = ((*pAct & ACT_FLOW_NEXT_MSK) >> ACT_FLOW_NEXT_POS);
      } 
      //get qty
      qty  = *pAct & ACT_FNF_QTY_MSK; 
      if (qty <= 1) {
        //pop Q
        printf("Popping Q\n");
        *(pQ + (CMDQ_RD_OFF >> 2)) += 1;
        *(pQ + (CMDQ_RD_OFF >> 2)) &= CMDQ_IDX_OF_MSK; //trim by mask
      } else {
        //decrease generator
        printf("Qty %u, yield Generator\n", qty);
        *pAct &= ~ACT_FNF_QTY_MSK;
        *pAct |= qty-1;
      }
      
    


                //Flush?
                    if (type == ACT_TYPE_FLUSH) {
                      //set 
                      //*(pQ + (CMDQ_RD_OFF >> 2)) =  (*pCmd & CMDQ_PTR_MSK) >> ;
                    } else {}
    } // due
  } else {
  // no elements in cmdq
  }

  return idxnext;
}





