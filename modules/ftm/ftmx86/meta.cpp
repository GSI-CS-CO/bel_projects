#include <stdlib.h>
#include <stdio.h>
#include "meta.h"
#include "ftm_common.h"


void Meta::serialise(vAdr &dest, vAdr &custom) {
  Node::serialise(dest, custom);
}

void Block::serialise(vAdr &dest, vAdr &custom) {
  Meta::serialise(dest, custom);
  
  writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_PERIOD,  this->tPeriod);
  if ((dest.size() > 1) && (custom.size() == 4)) {
    writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_ALT_DEST_PTR,  custom[CUST_ADR_BLOCK_ALT_LST]);  
    writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_IL_PTR,   custom[CUST_ADR_BLOCK_Q_IL]); 
    writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_HI_PTR,   custom[CUST_ADR_BLOCK_Q_HI]); 
    writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_LO_PTR,   custom[CUST_ADR_BLOCK_Q_LO]);   
    writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_WR_IDXS,  this->getWrIdxs());
    writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_RD_IDXS,  this->getRdIdxs());
  } else {
    // do a check and warn if inconsistent parameters are detected
    writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_ALT_DEST_PTR,  LM32_NULL_PTR);  
    writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_IL_PTR,   LM32_NULL_PTR); 
    writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_HI_PTR,   LM32_NULL_PTR); 
    writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_LO_PTR,   LM32_NULL_PTR);   
  }
}


void CmdQueue::serialise(vAdr &dest, vAdr &custom) {
  if (custom.size() < 1) {// we need at least one cmd buffer

    Meta::serialise(dest, custom);
    for(auto it = custom.begin(); it < custom.end(); it++) {
      writeLeNumberToBeBytes(b + (ptrdiff_t)CMDQ_BUF_ARRAY + (it - custom.begin()),  *it); 
    }
  }
}



void CmdQBuffer::serialise(vAdr &dest, vAdr &custom) {
  Meta::serialise(dest, custom);
}

void AltDestList::serialise(vAdr &dest, vAdr &custom) {
  Meta::serialise(dest, custom);
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
};

uint32_t Block::getRdIdxs(void) const {
  return (uint32_t)((this->rdIdxIl << 16) | (this->rdIdxHi << 8) | (this->rdIdxLo << 0));
};

void CmdQueue::show(void) const {
  CmdQueue::show(0, "");
}

void CmdQueue::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
}

void CmdQBuffer::show(void) const {
  CmdQBuffer::show(0, "");
}

void CmdQBuffer::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;

}

void AltDestList::show(void) const {
  AltDestList::show(0, "");
}

void AltDestList::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;

}

