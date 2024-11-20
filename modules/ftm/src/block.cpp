#include <stdlib.h>
#include <stdio.h>
#include "block.h"
#include "ftm_common.h"

void Block::deserialise(uint8_t* b)  {
  this->tPeriod  = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[BLOCK_PERIOD]);
  setWrIdxs(writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[BLOCK_CMDQ_WR_IDXS]));
  setRdIdxs(writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[BLOCK_CMDQ_RD_IDXS]));
}

void Block::serialise(const mVal &m, uint8_t* b) const {



  Node::serialise(m, b);

  //period field is a bit tricky if you want to be able to use 32 and 64b pointers. we need to set all non defined map keys to zero
  

  writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_PERIOD,  this->tPeriod); 
  //we don't necessarily have references for the block period. Check first to avoid exceptions if key not found
  //if (m.count(BLOCK_PERIOD_HI)) {writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_PERIOD_HI,     m.at(BLOCK_PERIOD_HI));}
  //if (m.count(BLOCK_PERIOD_LO)) {writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_PERIOD_LO,     m.at(BLOCK_PERIOD_LO));}
  
  //writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_ALT_DEST_PTR,  m.at(BLOCK_ALT_DEST_PTR));
  //writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_IL_PTR,   m.at(BLOCK_CMDQ_IL_PTR));
  //writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_HI_PTR,   m.at(BLOCK_CMDQ_HI_PTR));
  //writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_LO_PTR,   m.at(BLOCK_CMDQ_LO_PTR));
  writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_WR_IDXS,  this->getWrIdxs());
  writeLeNumberToBeBytes(b + (ptrdiff_t)BLOCK_CMDQ_RD_IDXS,  this->getRdIdxs());

  //FIXME : TEST ONLY
  for (auto it = m.begin(); it != m.end(); it++) { writeLeNumberToBeBytes(b + (ptrdiff_t)it->first, it->second); }

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

