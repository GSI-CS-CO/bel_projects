////////////////////////////////////////////////////////////////
//Triple Command Queue Structure
////////////////////////////////////////////////////////////////


#define PRIO_LO 0
#define PRIO_HI 1
#define PRIO_IL 2

//CMD structure
#define CMD_TS_OFF            0x0
#define CMD_ACT_OFF           (CMD_TS_OFF  + 8)
#define _CMD_SIZE             (CMD_ACT_OFF + 4)

//Action substructure
#define CMD_TYPE_MSK          0xf0000000
#define ACT_TYPE_NOP          0x00000000
#define ACT_TYPE_FLOW         0x10000000
#define ACT_TYPE_FLUSH        0x20000000 // save the bitshift
#define ACT_TYPE_POS          28

//Action of Nop type
#define ACT_FNF_QTY_MSK       0x0000ffff
#define ACT_FNF_QTY_POS       0
//Action of Flow type
#define ACT_FLOW_NEXT_MSK     0x0fff0000
#define ACT_FLOW_NEXT_POS     16
//Action of Flush type
#define ACT_FLUSH_LO_MSK      0x00010000   
#define ACT_FLUSH_HI_MSK      0x00020000   
#define ACT_FLUSH_IL_MSK      0x00040000   
#define ACT_FLUSH_LO_RNG_MSK  0x000000ff
#define ACT_FLUSH_LO_RNG_POS  0
#define ACT_FLUSH_HI_RNG_MSK  0x0000ff00
#define ACT_FLUSH_HI_RNG_POS  8

//CMDQ structure
#define CMDQ_QTY              4
#define _CMDQ_BUF_SIZE             (CMDQ_QTY * _CMD_SIZE)
#define CMDQ_IDX_OF_MSK       (2 * CMDQ_QTY -1) // x2 for overflow bit
#define CMDQ_IDX_MSK          (CMDQ_QTY -1) // 
#define _CMDQ_IDX_SIZE        4
#define CMDQ_OFF              0
#define CMDQ_RD_OFF           (CMDQ_OFF)
#define CMDQ_WR_OFF           (CMDQ_RD_OFF  + _CMDQ_IDX_SIZE)
#define CMDQ_BUF_OFF          (CMDQ_WR_OFF  + _CMDQ_IDX_SIZE)
#define _CMDQ_SIZE            (CMDQ_BUF_OFF + _CMDQ_BUF_SIZE)

////////////////////////////////////////////////////////////////
//CMDQS structure
////////////////////////////////////////////////////////////////
//Interlock Queue
#define CMDQS_OFF             0
#define CMDQ_LO_RD_OFF        (CMDQS_OFF       + 0)
#define CMDQ_LO_OFF           (CMDQ_LO_RD_OFF)
#define CMDQ_LO_WR_OFF        (CMDQ_LO_RD_OFF  + _CMDQ_IDX_SIZE)
#define CMDQ_LO_BUF_OFF       (CMDQ_LO_WR_OFF  + _CMDQ_IDX_SIZE)
//High Priority Queue
#define CMDQ_HI_RD_OFF        (CMDQ_LO_BUF_OFF + _CMDQ_BUF_SIZE)
#define CMDQ_HI_OFF           (CMDQ_HI_RD_OFF)
#define CMDQ_HI_WR_OFF        (CMDQ_HI_RD_OFF  + _CMDQ_IDX_SIZE)
#define CMDQ_HI_BUF_OFF       (CMDQ_HI_WR_OFF  + _CMDQ_IDX_SIZE)
//Low Priority Queue
#define CMDQ_IL_RD_OFF        (CMDQ_HI_BUF_OFF + _CMDQ_BUF_SIZE)
#define CMDQ_IL_OFF           (CMDQ_IL_RD_OFF)
#define CMDQ_IL_WR_OFF        (CMDQ_IL_RD_OFF  + _CMDQ_IDX_SIZE)
#define CMDQ_IL_BUF_OFF       (CMDQ_IL_WR_OFF  + _CMDQ_IDX_SIZE)
#define CMDQS_SIZE            (CMDQ_IL_BUF_OFF + _CMDQ_BUF_SIZE)

const uint8_t cOff[4] = {CMDQ_LO_OFF / 4, CMDQ_HI_OFF / 4, CMDQ_IL_OFF /4, CMDQ_IL_OFF /4};
  




//cannot use pointers directly, as we don't know if the queues were placed aligned. use (masked) counters instead that can overflow

uint32_t testQs[CMDQS_SIZE/4]

uint32_t *pCmdQ;
uint64_t now;

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

uint32_t* pushQ(uint32_t* pQs, uint32_t priority, uint32_t action, uint64_t time)
{
  uint32_t *pQ, *pCmd;
  
  pQ      = pQs + (cOff[priority] >> 2);       
  pCmd    =  (uint32_t*)(pQ   + (CMDQ_BUF_OFF + (wrIdx & CMDQ_IDX_MSK) * _CMD_SIZE) >> 2); //get ptr to current command

  *(uint64_t*)(pCmd + (CMD_TS_OFF   >> 2)  = time;
  *(uint32_t*)(pCmd + (CMD_TACT_OFF >> 2)  = action;
  *(uint32_t*)(pQ   + (CMDQ_WR_OFF  >> 2)) = (*(uint32_t*)(pQ   + (CMDQ_WR_OFF  >> 2)) + 1) & CMDQ_IDX_MSK;

  return pQs;
}




int checkQs(uint32* pQs, int idxnext, uint64_t now) {


  uint8_t rdIdx, qIlNotEmpty, qHiNotEmpty, qLoNotEmpty, qty, type;
  uint32_t *pCmd = NULL, *pAct = NULL;
  uint64_t ts;
//compare rd/wr indices to select Il or Hi Queue if they contain elements, Lo is default

  //masking all indices with CMDQ_IDX_OF_MSK would be safer, but takes a lot of time. leave out for now.
  //We rely on host writing conformant indices and mask our own updates
  qIlNotEmpty = ( *(pQ + (CMDQ_IL_WR_OFF >> 2)) != *(pQ + (CMDQ_IL_RD_OFF >> 2)) );
  qHiNotEmpty = ( *(pQ + (CMDQ_HI_WR_OFF >> 2)) != *(pQ + (CMDQ_HI_RD_OFF >> 2)) )
  qLoNotEmpty = ( *(pQ + (CMDQ_LO_WR_OFF >> 2)) != *(pQ + (CMDQ_LO_RD_OFF >> 2)) );
  queueIdx    = (qIlNotEmpty << 1) + qHiNotEmpty;

  //check if any elements were found
  if (queueIdx + qLoNotEmpty) {

    //now     = getSysTime();
    printf("Elements found in Q %u\n", queueIdx);

    pQ      = pQ + cOff[queueIdx];                                          //get ptr to non empty queue of highest priority
    rdIdx   = *(uint32_t*)(pQ   + (CMDQ_RD_OFF >> 2));                      //get read idx
    pCmd    =  (uint32_t*)(pQ   + (CMDQ_BUF_OFF + (rdIdx & CMDQ_IDX_MSK) * _CMD_SIZE) >> 2); //get ptr to current command
    ts      = *(uint64_t*)(pCmd + (CMD_TS_OFF   >> 2);                      //get current due time
    pAct    =  (uint32_t*)(pCmd + (CMD_TACT_OFF >> 2);                      //get ptr to current action

    //Check if element is Due
    if (now < ts) {
    //not due

    } else {
      //due
      //Get type
      printf("Element due, dl %llu, now %llu\n", ts, now);

      type = *pAct & CMD_TYPE_MSK;

      if type == ACT_TYPE_FLOW {
        idxnext = ((*pAct & ACT_FLOW_NEXT_MSK) >> ACT_FLOW_NEXT_POS);
      } 
      //get qty
      qty  = *pAct & ACT_FNF_QTY_MSK; 
      if qty <= 1 {
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
      
    }


  //Flush?
      if (type == ACT_TYPE_FLUSH) {
        //set 
        *(pQ + (CMDQ_RD_OFF >> 2)) =  (*pCmd & CMDQ_PTR_MSK) >> ;
      } else {
    } // due
  } else {
  // no elements in cmdq
  }

  return idxnext;
}

pushQ(pTestQs, PRIO_LO, createFlow(1, 3), 500)
pushQ(pTestQs, PRIO_LO, createFlow(2, 1), 550)

idx = 100;
idx = checkQs(pTestQs, idx, 490);
idx = checkQs(pTestQs, idx, 490);
idx = checkQs(pTestQs, idx, 500);
idx = checkQs(pTestQs, idx, 500);
idx = checkQs(pTestQs, idx, 500);
idx = checkQs(pTestQs, idx, 500);
idx = checkQs(pTestQs, idx, 550);
idx = checkQs(pTestQs, idx, 550);



