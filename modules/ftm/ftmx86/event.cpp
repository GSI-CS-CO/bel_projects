#include <stdlib.h>
#include <stdio.h>
#include "event.h"
#include "ftm_common.h"
/*
void Event::serialiseB(itBuf ib) {
  uint64ToBytes(ib + EVT_OFFS_TIME,   this->tOffs);
  uint16ToBytes(ib + EVT_FLAGS,       this->flags);
}

void TimingMsg::serialise(itBuf ib) {
  Event::serialiseB(ib); //call protected base serialiser
  uint16ToBytes(ib + EVT_TYPE, EVT_TYPE_TMSG);
  uint64ToBytes(ib + _EVT_HDR_SIZE + EVT_TM_ID,  this->id);
  uint64ToBytes(ib + _EVT_HDR_SIZE + EVT_TM_PAR, this->par);
  uint32ToBytes(ib + _EVT_HDR_SIZE + EVT_TM_TEF, this->tef);
}
*/
void TimingMsg::show(void)  const{
  TimingMsg::show(0, "");
}

void TimingMsg::show(uint32_t cnt, const char* prefix)  const {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** TimingMsg @ %llu, ", p, (long long unsigned int)this->tOffs);
  printf("%sID 0x%08x%08x, Par 0x%08x%08x, Tef 0x%08x\n", p, (uint32_t)(this->id >> 32),
  (uint32_t)this->id, (uint32_t)(this->par >> 32), (uint32_t)this->par, this->tef);
}
/*
void Command::serialiseB(itBuf ib) {
  Event::serialiseB(ib); //call protected base serialiser
  uint16ToBytes(ib + EVT_TYPE, EVT_TYPE_CMD);
  uint64ToBytes(ib + _EVT_HDR_SIZE + EVT_CM_TIME,  this->tValid);
  uint32ToBytes(ib + _EVT_HDR_SIZE + EVT_CMD_RESERVED, 0); //pad
}
*/
void Command::show(void)  const {
  Command::show(0, "");
}

void Command::show(uint32_t cnt, const char* prefix)  const {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** Command   @ %llu, ", p, (long long unsigned int)this->tOffs);
  printf("%sValid @ %llu, ", p, (long long unsigned int)this->tValid);
}

void Noop::show(void) const {
  Noop::show(0, "");
}




void Noop::show(uint32_t cnt, const char* prefix)  const {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  Command::show( cnt, prefix);
  printf("%s%u x No Operation\n", p, this->qty);
}
/*
void Noop::serialise(itBuf ib) {
  Command::serialiseB(ib);
  uint32ToBytes(ib + _EVT_HDR_SIZE + EVT_CM_ACT, (ACT_TYPE_NOP | ((this->qty << ACT_FNF_QTY_POS) & ACT_FNF_QTY_MSK)));
}
*/
void Flow::show(void) const {
  Flow::show(0, "");
}

void Flow::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  uint16_t idxNext = NO_SUCCESSOR;
  //if (this->blNext != NULL) idxNext = blNext->getIdx();
  Command::show( cnt, p);
  //printf("%s%u x Flow Change to", p, this->qty);
  //if (idxNext == NO_SUCCESSOR)  printf(" END\n");
  //else                          printf(" Block %u\n", idxNext);
}
/*
void Flow::serialise(itBuf ib) {
  Command::serialiseB(ib);
  uint16_t idxNext = NO_SUCCESSOR;
  if (this->blNext != NULL) idxNext = blNext->getIdx();
  uint32ToBytes(ib + _EVT_HDR_SIZE + EVT_CM_ACT, (ACT_TYPE_FLOW | ((idxNext << ACT_FLOW_NEXT_POS) & ACT_FLOW_NEXT_MSK) | ((this->qty << ACT_FNF_QTY_POS) & ACT_FNF_QTY_MSK)));
}
*/

void Flush::show(void) const {
  Flush::show(0, "");
}

void Flush::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  Command::show( cnt, p);
  printf("%s Flush \n", p);
  if (this->qIl) printf("Interlock Q\n");
  if (this->qHi) printf("High Prio. Q up to idx %u\n", this->upToHi);
  if (this->qLo) printf("Low Prio. Q up to idx  %u\n", this->upToLo);
}

/*
void Flush::serialise(itBuf ib) {
  Command::serialiseB(ib);
  uint32_t act =  (ACT_TYPE_FLUSH | (this->qIl << ACT_FLUSH_IL_POS) | (this->qHi << ACT_FLUSH_HI_POS) | (this->qLo << ACT_FLUSH_LO_POS) |
                 ((this->upToHi & CMDQ_IDX_OF_MSK) << ACT_FLUSH_HI_RNG_POS) | ((this->upToLo & CMDQ_IDX_OF_MSK) << ACT_FLUSH_LO_RNG_POS) );
  uint32ToBytes(ib + _EVT_HDR_SIZE + EVT_CM_ACT, act);
  
}
*/
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





