#include <stdlib.h>
#include <stdio.h>
#include "event.h"
#include "ftm_common.h"


void Event::deserialise() {
  this->tOffs = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[EVT_OFFS_TIME]);
}

void Event::serialise(const vAdr &va) const {
  Node::serialise(va);
  writeLeNumberToBeBytes(b + (ptrdiff_t)EVT_OFFS_TIME, this->tOffs);
}

void TimingMsg::deserialise() {
  Event::deserialise();
  this->id  = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[TMSG_ID]);
  this->par = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[TMSG_PAR]);
  this->tef = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[TMSG_TEF]);
  this->res = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[TMSG_RES]);
}

void TimingMsg::serialise(const vAdr &va) const {
  Event::serialise(va);

  uint64_t id  = this->id;
  uint64_t par = this->par;
  uint32_t tef = this->tef;
  uint32_t res = this->res;

  //Careful - the fact that these can be pointers does not mean the LM32 has to interprete them!
  //That still depends on the flags
  if (va[ADR_DYN_ID]    != LM32_NULL_PTR) id  &= ~0xffffffffULL; id   |= va[ADR_DYN_ID];;
  if (va[ADR_DYN_PAR0]  != LM32_NULL_PTR) par &= ~(0xffffffffULL << 32); par  |= ((uint64_t)va[ADR_DYN_PAR0] << 32);
  if (va[ADR_DYN_PAR1]  != LM32_NULL_PTR) par &= ~0xffffffffULL; par  |= va[ADR_DYN_PAR1];
  if (va[ADR_DYN_TEF]   != LM32_NULL_PTR) tef  = va[ADR_DYN_TEF];
  if (va[ADR_DYN_RES]   != LM32_NULL_PTR) res  = va[ADR_DYN_RES];


  writeLeNumberToBeBytes(b + (ptrdiff_t)TMSG_ID,  id);
  writeLeNumberToBeBytes(b + (ptrdiff_t)TMSG_PAR, par);
  writeLeNumberToBeBytes(b + (ptrdiff_t)TMSG_TEF, tef);
  writeLeNumberToBeBytes(b + (ptrdiff_t)TMSG_RES, res);    
}

void Command::deserialise()  {
  Event::deserialise();
  this->tValid  = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[CMD_VALID_TIME]);
  this->act     = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[CMD_ACT]);
}

void Command::serialise(const vAdr &va) const {
  Event::serialise(va);
  writeLeNumberToBeBytes(b + (ptrdiff_t)CMD_TARGET,     va[ADR_CMD_TARGET]);
  writeLeNumberToBeBytes(b + (ptrdiff_t)CMD_VALID_TIME, this->tValid);
  writeLeNumberToBeBytes(b + (ptrdiff_t)CMD_ACT,        this->act); 

}

void Noop::deserialise()  {
  Command::deserialise();
  
}

void Noop::serialise(const vAdr &va) const {
  Command::serialise(va);
  
}

void Flow::deserialise()  {
  Command::deserialise();
}

void Flow::serialise(const vAdr &va) const {
  Command::serialise(va);
  writeLeNumberToBeBytes(b + (ptrdiff_t)CMD_FLOW_DEST, va[ADR_CMD_FLOW_DEST]); 
}

void Wait::deserialise()  {
  Command::deserialise();
  this->tWait = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[CMD_WAIT_TIME]);
}

void Wait::serialise(const vAdr &va) const {
  Command::serialise(va);
  writeLeNumberToBeBytes(b + (ptrdiff_t)CMD_WAIT_TIME, this->tWait);  

}

void Flush::deserialise()  {
  Command::deserialise();
  this->frmIl  = b[CMD_FLUSHRNG_IL_FRM];
  this->toIl   = b[CMD_FLUSHRNG_IL_TO];
  this->frmHi  = b[CMD_FLUSHRNG_HI_FRM];
  this->toHi   = b[CMD_FLUSHRNG_HI_TO];
  this->frmLo  = b[CMD_FLUSHRNG_LO_FRM];
  this->toLo   = b[CMD_FLUSHRNG_LO_TO];
}

void Flush::serialise(const vAdr &va) const {
  Command::serialise(va);
  b[CMD_FLUSHRNG_IL_FRM]  = this->frmIl;
  b[CMD_FLUSHRNG_IL_TO]   = this->toIl;
  b[CMD_FLUSHRNG_HI_FRM]  = this->frmHi;
  b[CMD_FLUSHRNG_HI_TO]   = this->toHi;
  b[CMD_FLUSHRNG_LO_FRM]  = this->frmLo;
  b[CMD_FLUSHRNG_LO_TO]   = this->toLo;
  
}

const uint16_t Flush::getRng(uint8_t q) const {
  if (q ==  PRIO_IL) {return (uint16_t)((this->frmIl << 8) | this->toIl);}
  if (q ==  PRIO_HI) {return (uint16_t)((this->frmHi << 8) | this->toHi);}
  return (uint16_t)((this->frmLo << 8) | this->toLo);
}

void TimingMsg::show(void) const {
  TimingMsg::show(0, "");
}

void TimingMsg::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** TimingMsg @ %llu, ", p, (long long unsigned int)this->tOffs);
  printf("%sID 0x%08x%08x, Par 0x%08x%08x, Tef 0x%08x\n", p, (uint32_t)(this->id >> 32),
  (uint32_t)this->id, (uint32_t)(this->par >> 32), (uint32_t)this->par, this->tef);
}

void Command::show(void) const {
  Command::show(0, "");
}

void Command::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** Command   @ %llu, ", p, (long long unsigned int)this->tOffs);
  printf("%sValid @ %llu, ", p, (long long unsigned int)this->tValid);
}

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
  if (this->qHi) printf("High Prio. Q up to idx %u\n", this->toHi);
  if (this->qLo) printf("Low Prio. Q up to idx  %u\n", this->toLo);
}

void Flow::show(void) const {
  Flow::show(0, "");
}

void Flow::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  
  Command::show( cnt, p);
  
}

void Wait::show(void) const {
  Wait::show(0, "");
}

void Wait::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  
  Command::show( cnt, p);
  
}


void Noop::show(void) const {
  Noop::show(0, "");
}




void Noop::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  Command::show( cnt, prefix);
  printf("%s%u x No Operation\n", p, this->getQty());
}
