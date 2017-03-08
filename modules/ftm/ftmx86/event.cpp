#include <stdlib.h>
#include <stdio.h>
#include "event.h"



//TimingMsg::TimingMsg(uint64_t tOffs, uint16_t flags, uint64_t id, uint64_t par, uint32_t tef) : Event (tOffs, flags), id(id), par(par), tef(tef) {}

void TimingMsg::show(void) {
  TimingMsg::show(0, "");
}

void TimingMsg::show(uint32_t cnt, const char* prefix) {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** TimingMsg @ %llu\n", p, (long long unsigned int)this->tOffs);
  printf("%sID 0x%08x%08x, Par 0x%08x%08x, Tef 0x%08x\n", p, (uint32_t)(this->id >> 32),
  (uint32_t)this->id, (uint32_t)(this->par >> 32), (uint32_t)this->par, this->tef);
}



//Command::Command(uint64_t tOffs, uint16_t flags, uint64_t tValid, uint32_t act) : Event (tOffs, flags), tValid(tValid), act(act) {}

void Command::show(void) {
  Command::show(0, "");
}

void Command::show(uint32_t cnt, const char* prefix) {
  char* p;
  if (prefix == NULL) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** Command   @ %llu\n", p, (long long unsigned int)this->tOffs);
  printf("%sValid @ %llu, Action 0x%08x\n", p, (long long unsigned int)this->tValid, this->act);
}






