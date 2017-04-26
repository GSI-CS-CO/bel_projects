#include <stdlib.h>
#include <stdio.h>
#include "event.h"
#include "ftm_common.h"




void Event::serialise(vAdr &dest, vAdr &custom) {
  uint8_t* b = (uint8_t*)&(this->buf[0]);
  Node::serialise(dest, custom);
  uint64ToBytes(b + (ptrdiff_t)EVT_OFFS_TIME, this->tOffs);
}

void TimingMsg::serialise(vAdr &dest, vAdr &custom) {
  uint8_t* b = (uint8_t*)&(this->buf[0]);
  Event::serialise(dest, custom);
     
  uint64ToBytes(b + (ptrdiff_t)TMSG_ID,  this->id);
  uint64ToBytes(b + (ptrdiff_t)TMSG_PAR, this->par);
  uint32ToBytes(b + (ptrdiff_t)TMSG_TEF, this->tef);
  uint32ToBytes(b + (ptrdiff_t)TMSG_RES, this->res);    
}


void Command::serialise(vAdr &dest, vAdr &custom) {
  uint8_t* b = (uint8_t*)&(this->buf[0]);
  if (custom.size() < 1) //scream and shout, we didn't get told what our target queue is!
  {
    Event::serialise(dest, custom);
    uint32ToBytes(b + (ptrdiff_t)CMD_TARGET,  custom[CUST_ADR_CMD_TARGET]);
    uint64ToBytes(b + (ptrdiff_t)CMD_VALID_TIME, this->tValid); 
  }
}



void Noop::serialise(vAdr &dest, vAdr &custom) {
  uint8_t* b = (uint8_t*)&(this->buf[0]);
  Command::serialise(dest, custom);
  uint32_t act = (ACT_TYPE_NOOP << ACT_TYPE_POS) | ((this->qty & ACT_QTY_MSK) << ACT_QTY_POS); 
  uint32ToBytes(b + (ptrdiff_t)CMD_ACT, act);
}



void Flow::serialise(vAdr &dest, vAdr &custom) {
  uint8_t* b = (uint8_t*)&(this->buf[0]);
  Command::serialise(dest, custom);
  //if (custom.size() < 2) //scream and shout, we didn't get told what to change the flow to!
  uint32_t act = (ACT_TYPE_FLOW << ACT_TYPE_POS) | ((this->qty & ACT_QTY_MSK) << ACT_QTY_POS); 
  uint32ToBytes(b + (ptrdiff_t)CMD_ACT, act);
  uint32ToBytes(b + (ptrdiff_t)CMD_FLOW_DEST, custom[CUST_ADR_CMD_FLOW_DEST]); 
}


void Wait::serialise(vAdr &dest, vAdr &custom) {
  uint8_t* b = (uint8_t*)&(this->buf[0]);
  Command::serialise(dest, custom);
  uint32_t act = (ACT_TYPE_WAIT << ACT_TYPE_POS) | ((this->qty & ACT_QTY_MSK) << ACT_QTY_POS); 
  uint32ToBytes(b + (ptrdiff_t)CMD_ACT, act);
  uint64ToBytes(b + (ptrdiff_t)CMD_WAIT_TIME, this->tWait);  

}

void Flush::serialise(vAdr &dest, vAdr &custom) {
  uint8_t* b = (uint8_t*)&(this->buf[0]);
  Command::serialise(dest, custom);
  uint32_t act = (ACT_TYPE_FLUSH << ACT_TYPE_POS) | ((this->getPrio() & ACT_FLUSH_PRIO_MSK) << ACT_FLUSH_PRIO_POS) | ((this->getMode() & ACT_FLUSH_MODE_MSK) << ACT_FLUSH_MODE_POS); 
  uint32ToBytes(b + (ptrdiff_t)CMD_ACT, act);
  buf[CMD_FLUSHRNG_IL_FRM]  = this->frmIl;
  buf[CMD_FLUSHRNG_IL_TO]   = this->toIl;
  buf[CMD_FLUSHRNG_HI_FRM]  = this->frmHi;
  buf[CMD_FLUSHRNG_HI_TO]   = this->toHi;
  buf[CMD_FLUSHRNG_LO_FRM]  = this->frmLo;
  buf[CMD_FLUSHRNG_LO_TO]   = this->toLo;
  
}

const uint8_t  Flush::getPrio(void) const {
  return ((this->act >> ACT_FLUSH_PRIO_POS) & ACT_FLUSH_PRIO_MSK);
}

const uint8_t  Flush::getMode(void) const {
  return ((this->act >> ACT_FLUSH_MODE_POS) & ACT_FLUSH_MODE_MSK);
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



void Noop::show(void) const {
  Noop::show(0, "");
}




void Noop::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  Command::show( cnt, prefix);
  printf("%s%u x No Operation\n", p, this->qty);
}
