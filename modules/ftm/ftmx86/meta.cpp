#include <stdlib.h>
#include <stdio.h>
#include "Meta.h"
#include "ftm_common.h"


void TimeBlock::serialise(itAdr dest, itAdr custom) {

  uint16ToBytes((uint8_t*)&buf[EVT_TYPE, EVT_TYPE_TMSG);
  uint64ToBytes((uint8_t*)&buf[_EVT_HDR_SIZE + EVT_TM_ID,  this->id);
  uint64ToBytes((uint8_t*)&buf[_EVT_HDR_SIZE + EVT_TM_PAR, this->par);
  uint32ToBytes((uint8_t*)&buf[_EVT_HDR_SIZE + EVT_TM_TEF, this->tef);
}


void CmdQueue::serialise(itAdr dest, itAdr custom) {
   if (custom.size() < 1) //scream and shout, we didn't get told what our target queue is!

  Meta::serialiseB(dest); //call protected base serialiser
  uint64ToBytes((uint8_t*)&buf[_EVT_HDR_SIZE + EVT_CM_TIME],  this->tValid);
  uint32ToBytes((uint8_t*)&buf[_EVT_HDR_SIZE + EVT_CMD_RESERVED], 0); //pad
}



void CmdQBuffer::serialise(itAdr dest, itAdr custom) {
  uint16ToBytes((uint8_t*)&buf[EVT_TYPE], CMD_TYPE_NOOP);  
  uint32ToBytes((uint8_t*)&buf[_EVT_HDR_SIZE + EVT_CM_ACT], (ACT_TYPE_NOP | ((this->qty << ACT_FNF_QTY_POS) & ACT_FNF_QTY_MSK)));
}

void AltDestList::serialise(itAdr dest, itAdr custom) {
  uint16ToBytes((uint8_t*)&buf[EVT_TYPE], CMD_TYPE_NOOP);  
  uint32ToBytes((uint8_t*)&buf[_EVT_HDR_SIZE + EVT_CM_ACT], (ACT_TYPE_NOP | ((this->qty << ACT_FNF_QTY_POS) & ACT_FNF_QTY_MSK)));
}


void TimeBlock::show(void) {
  TimeBlock::show(0, "");
}

void TimeBlock::show(uint32_t cnt, const char* prefix) {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** TimeBlock @ %llu, ", p, (long long unsigned int)this->tOffs);
  printf("%sID 0x%08x%08x, Par 0x%08x%08x, Tef 0x%08x\n", p, (uint32_t)(this->id >> 32),
  (uint32_t)this->id, (uint32_t)(this->par >> 32), (uint32_t)this->par, this->tef);
}

v

void CmdQueue::show(void) const {
  Flush::show(0, "");
}

void CmdQueue::show(uint32_t cnt, const char* prefix) {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  Command::show( cnt, p);
  printf("%s Flush \n", p);
  if (this->qIl) printf("Interlock Q\n");
  if (this->qHi) printf("High Prio. Q up to idx %u\n", this->upToHi);
  if (this->qLo) printf("Low Prio. Q up to idx  %u\n", this->upToLo);
}

void CmdQBuffer::show(void) {
  Flow::show(0, "");
}

void CmdQBuffer::show(uint32_t cnt, const char* prefix) {
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

void AltDestList::show(void) {
  Flow::show(0, "");
}

void AltDestList::show(uint32_t cnt, const char* prefix) {
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

