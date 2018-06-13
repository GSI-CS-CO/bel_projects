#include <stdlib.h>
#include <stdio.h>
#include "block.h"
#include "ftm_common.h"

void Block::deserialise(uint8_t* b)  {
  this->tPeriod  = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[BLOCK_PERIOD]);
  setWrIdxs(writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[BLOCK_CMDQ_WR_IDXS]));
  setRdIdxs(writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[BLOCK_CMDQ_RD_IDXS]));
}

void Block::serialise(const vAdr &va, uint8_t* b) const {

 
    
  Node::serialise(va, b);
  writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_PERIOD,  this->tPeriod);
  writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_ALT_DEST_PTR,  va[ADR_BLOCK_DST_LST]);  
  writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_IL_PTR,   va[ADR_BLOCK_Q_IL]); 
  writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_HI_PTR,   va[ADR_BLOCK_Q_HI]); 
  writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_LO_PTR,   va[ADR_BLOCK_Q_LO]);   
  writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_WR_IDXS,  this->getWrIdxs());
  writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_RD_IDXS,  this->getRdIdxs());
  
}



void Block::show(void) const {
  Block::show(0, "");
}

void Block::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == nullptr) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** Block @ %llu, ", p, (long long unsigned int)this->tPeriod);
}

