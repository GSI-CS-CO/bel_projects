#include <stdlib.h>
#include <stdio.h>
#include "event.h"
#include "ftm_common.h"


void Event::deserialise(uint8_t* b) {
  this->tOffs = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[EVT_OFFS_TIME]);
}

void Event::serialise(const mVal &m, uint8_t* b) const {
  Node::serialise(m, b);
  writeLeNumberToBeBytes(b + (ptrdiff_t)EVT_OFFS_TIME, this->tOffs);
}

void TimingMsg::deserialise(uint8_t* b) {
  Event::deserialise(b);
  this->id  = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[TMSG_ID]);
  this->par = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[TMSG_PAR]);
  this->res = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[TMSG_RES]);
  this->tef = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[TMSG_TEF]);
}

void TimingMsg::serialise(const mVal &m, uint8_t* b) const {
  Event::serialise(m, b);

  writeLeNumberToBeBytes(b + (ptrdiff_t)TMSG_ID,  this->id);
  writeLeNumberToBeBytes(b + (ptrdiff_t)TMSG_PAR, this->par);
  writeLeNumberToBeBytes(b + (ptrdiff_t)TMSG_RES, this->res);
  writeLeNumberToBeBytes(b + (ptrdiff_t)TMSG_TEF, this->tef);

  /* DynLinks - Legacy support. This will be removed in favour of Ref and Val Links */
  //Overwrite the buffer with the dyn map pairs we got is cheaper than checking first
  
  for (auto it = m.begin(); it != m.end(); it++) { 
    writeLeNumberToBeBytes(b + (ptrdiff_t)it->first, it->second); 
  }

}

void Switch::serialise(const mVal &m, uint8_t* b) const {
  Event::serialise(m, b);
  writeLeNumberToBeBytes(b + (ptrdiff_t)SWITCH_TARGET, m.at(SWITCH_TARGET));
  writeLeNumberToBeBytes(b + (ptrdiff_t)SWITCH_DEST,   m.at(SWITCH_DEST));
}

void Switch::deserialise(uint8_t* b) {
  Event::deserialise(b);
}

void StartThread::deserialise(uint8_t* b) {
  Event::deserialise(b);
  this->setStartOffs(writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[STARTTHREAD_STARTOFFS]));
  this->setThread(writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[STARTTHREAD_THR]));
}

void StartThread::serialise(const mVal &m, uint8_t* b) const {
  Event::serialise(m, b);
  writeLeNumberToBeBytes(b + (ptrdiff_t)STARTTHREAD_STARTOFFS, this->getStartOffs());
  writeLeNumberToBeBytes(b + (ptrdiff_t)STARTTHREAD_THR,  this->getThread());
}

void Origin::serialise(const mVal &m, uint8_t* b) const {
  Event::serialise(m, b);
  writeLeNumberToBeBytes(b + (ptrdiff_t)ORIGIN_DEST, m.at(ORIGIN_DEST));
  writeLeNumberToBeBytes(b + (ptrdiff_t)ORIGIN_CPU,  m.at(ORIGIN_CPU));
  writeLeNumberToBeBytes(b + (ptrdiff_t)ORIGIN_THR,  this->getThread());
}

void Origin::deserialise(uint8_t* b) {
  Event::deserialise(b);
  this->setThread(writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[ORIGIN_THR]));
}


void Command::deserialise(uint8_t* b)   {
  Event::deserialise(b);
  this->tValid  = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[CMD_VALID_TIME]);
  this->act     = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&b[CMD_ACT]);
}

void Command::serialise(const mVal &m, uint8_t* b) const {
  Event::serialise(m, b);
  writeLeNumberToBeBytes(b + (ptrdiff_t)CMD_TARGET,     m.at(CMD_TARGET));
  writeLeNumberToBeBytes(b + (ptrdiff_t)CMD_VALID_TIME, this->tValid);
  writeLeNumberToBeBytes(b + (ptrdiff_t)CMD_ACT,        this->act);
}

void Noop::deserialise(uint8_t* b)  {
  Command::deserialise(b);

}

void Noop::serialise(const mVal &m, uint8_t* b) const {
  Command::serialise(m, b);

}

void Flow::deserialise(uint8_t* b)  {
  Command::deserialise(b);
}

void Flow::serialise(const mVal &m, uint8_t* b) const {
  Command::serialise(m, b);
  writeLeNumberToBeBytes(b + (ptrdiff_t)CMD_FLOW_DEST, m.at(CMD_FLOW_DEST));
}

void Wait::deserialise(uint8_t* b)  {
  Command::deserialise(b);
  this->tWait = writeBeBytesToLeNumber<uint64_t>((uint8_t*)&b[CMD_WAIT_TIME]);
}

void Wait::serialise(const mVal &m, uint8_t* b) const {
  Command::serialise(m, b);
  writeLeNumberToBeBytes(b + (ptrdiff_t)CMD_WAIT_TIME, this->tWait);
}

void Flush::deserialise(uint8_t* b)  {
  Command::deserialise(b);
  this->frmIl  = b[CMD_FLUSHRNG_IL_FRM];
  this->toIl   = b[CMD_FLUSHRNG_IL_TO];
  this->frmHi  = b[CMD_FLUSHRNG_HI_FRM];
  this->toHi   = b[CMD_FLUSHRNG_HI_TO];
  this->frmLo  = b[CMD_FLUSHRNG_LO_FRM];
  this->toLo   = b[CMD_FLUSHRNG_LO_TO];
}

void Flush::serialise(const mVal &m, uint8_t* b) const {
  Command::serialise(m, b);
  writeLeNumberToBeBytes(b + (ptrdiff_t)CMD_FLUSH_DEST_OVR, m.at(CMD_FLUSH_DEST_OVR));
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
  if (prefix == nullptr) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** TimingMsg @ %llu, ", p, (long long unsigned int)this->tOffs);
  printf("%sID 0x%08x%08x, Par 0x%08x%08x, Tef 0x%08x\n", p, (uint32_t)(this->id >> 32),
  (uint32_t)this->id, (uint32_t)(this->par >> 32), (uint32_t)this->par, this->tef);
}

void Switch::show(void) const {
  Switch::show(0, "");
}

void Switch::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == nullptr) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** Switch @ %llu, ", p, (long long unsigned int)this->tOffs);
}


void Origin::show(void) const {
  Origin::show(0, "");
}

void Origin::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == nullptr) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** Origin @ %llu, ", p, (long long unsigned int)this->tOffs);
  printf("%s*** Thr @ %u, ", p, (unsigned int)this->getThread());
}    

void StartThread::show(void) const {
  StartThread::show(0, "");
}

void StartThread::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == nullptr) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** Origin @ %llu, ", p, (long long unsigned int)this->tOffs);
  printf("%s*** Thr @ %u, ", p, (unsigned int)this->getThread());
} 

void Command::show(void) const {
  Command::show(0, "");
}

void Command::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == nullptr) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** Command   @ %llu, ", p, (long long unsigned int)this->tOffs);
  printf("%sValid @ %llu, ", p, (long long unsigned int)this->tValid);
  printf("%sact @ 0x%08x, ", p, (unsigned int)this->act);
}

void Flush::show(void) const {
  Flush::show(0, "");
}

void Flush::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == nullptr) p = (char*)"";
  else p = (char*)prefix;
  Command::show( cnt, p);
  printf("%s Flush \n", p);
  uint32_t flushPrio = this->getFlushPrio();
  if (flushPrio & (1 << PRIO_IL)) printf("Interlock Q\n");
  if (flushPrio & (1 << PRIO_HI)) printf("High Prio. Q up to idx %u\n", this->toHi);
  if (flushPrio & (1 << PRIO_LO)) printf("Low Prio. Q up to idx  %u\n", this->toLo);
}

void Flow::show(void) const {
  Flow::show(0, "");
}

void Flow::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == nullptr) p = (char*)"";
  else p = (char*)prefix;

  Command::show( cnt, p);
}

void Wait::show(void) const {
  Wait::show(0, "");
}

void Wait::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == nullptr) p = (char*)"";
  else p = (char*)prefix;

  Command::show( cnt, p);
}

void Noop::show(void) const {
  Noop::show(0, "");
}

void Noop::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == nullptr) p = (char*)"";
  else p = (char*)prefix;
  Command::show( cnt, prefix);
  printf("%s%u x No Operation\n", p, this->getQty());
}
