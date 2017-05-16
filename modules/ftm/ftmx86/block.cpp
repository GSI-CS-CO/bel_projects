#include <stdlib.h>
#include <stdio.h>
#include "block.h"
#include "ftm_common.h"



void Block::serialise(const vAdr &va) const {

 
    
  Node::serialise(va);
  
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
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** Block @ %llu, ", p, (long long unsigned int)this->tPeriod);
}

uint32_t Block::getWrIdxs(void) const {
  return (uint32_t)((this->wrIdxIl << 16) | (this->wrIdxHi << 8) | (this->wrIdxLo << 0));
}

uint32_t Block::getRdIdxs(void) const {
  return (uint32_t)((this->rdIdxIl << 16) | (this->rdIdxHi << 8) | (this->rdIdxLo << 0));
}

