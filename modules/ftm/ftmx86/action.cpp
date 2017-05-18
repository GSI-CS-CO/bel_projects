#include <stdlib.h>
#include <stdio.h>
#include "action.h"
#include "ftm_common.h"





void Noop::show(void) {
  Noop::show(0, "");
}

void Noop::show(uint32_t cnt, const char* prefix) {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  printf("%s%u x No Operation\n", p, this->qty);
}

uint8_t* Noop::serialise(uint8_t* pBuf) {
  uint8_t *pC = (uint8_t*)pBuf;
  if (pC != NULL) {
    writeLeNumberToBeBytes(pC, (ACT_TYPE_NOP | ((this->qty << ACT_FNF_QTY_POS) & ACT_FNF_QTY_MSK)));
  }
  return pC;
}

void Flow::show(void) {
  Flow::show(0, "");
}

void Flow::show(uint32_t cnt, const char* prefix) {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  uint16_t idxNext = NO_SUCCESSOR;
  if (this->blNext != NULL) idxNext = blNext->idx;
  else 
  printf("%s%u x Flow Change to", p, this->qty);
  if (idxNext == NO_SUCCESSOR)  printf(" END\n");
  else                          printf(" Block %u\n", idxNext);
}

uint8_t* Flow::serialise(uint8_t* pBuf) {
  uint8_t *pC = (uint8_t*)pBuf;
  uint16_t idxNext = NO_SUCCESSOR;

  if (pC != NULL) {
    if (this->blNext != NULL) idxNext = blNext->idx;
    writeLeNumberToBeBytes(pC, (ACT_TYPE_FLOW | ((idxNext << ACT_FLOW_NEXT_POS) & ACT_FLOW_NEXT_MSK) | ((this->qty << ACT_FNF_QTY_POS) & ACT_FNF_QTY_MSK)));
  }
  return pC;
}

Flush::Flush(void) {
  this->qIl = false;
  this->qHi = false;
  this->qLo = false;
  this->upToHi = ACT_FLUSH_RANGE_ALL;
  this->upToLo = ACT_FLUSH_RANGE_ALL;
}

void Flush::show(void) {
  Flush::show(0, "");
}

void Flush::show(uint32_t cnt, const char* prefix) {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  printf("%s Flush \n", p);
  if (this->qIl) printf("Interlock Q\n");
  if (this->qHi) printf("High Prio. Q up to idx %u\n", this->upToHi);
  if (this->qLo) printf("Low Prio. Q up to idx  %u\n", this->upToLo);
}


uint8_t* Flush::serialise(uint8_t* pBuf) {
  uint8_t *pC = (uint8_t*)pBuf;
  uint32_t act =  (ACT_TYPE_FLUSH | (this->qIl << ACT_FLUSH_IL_POS) | (this->qHi << ACT_FLUSH_HI_POS) | (this->qLo << ACT_FLUSH_LO_POS) |
                 ((this->upToHi & CMDQ_IDX_OF_MSK) << ACT_FLUSH_HI_RNG_POS) | ((this->upToLo & CMDQ_IDX_OF_MSK) << ACT_FLUSH_LO_RNG_POS) ));
  if (pC != NULL) {
    writeLeNumberToBeBytes(pC, act);
  }
  return pC;
}

 void Flush::set(prio target, uint8_t upTo) {
    switch(target) {
      case(INTERLOCK) : this->qIl = true; break;
      case(HIGH)      : this->qHi = true; this->upToHi = upTo; break;
      case(LOW)       : this->qLo = true; this->upToLo = upTo; break;
      default         : break;
    } 
 }

 void Flush::clear(prio target) {
  switch(target) {
      case(INTERLOCK) : this->qIl = false; break;
      case(HIGH)      : this->qHi = false; break;
      case(LOW)       : this->qLo = false; break;
      default         : break;
    } 
  }


}

