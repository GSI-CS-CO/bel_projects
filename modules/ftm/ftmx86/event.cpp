#include <stdlib.h>
#include <stdio.h>
#include "event.h"

Event::Event(void) {
  printf("Create Event Base\n");
}

TimingMsg::TimingMsg(uint64_t tOffs, uint16_t flags, uint64_t id, uint64_t par, uint32_t tef) : Event (tOffs, flags) id(id), par(par), tef(tef) {}

TimingMsg::show() {
  TimingMsg::Show(0, "");
}

TimingMsg::show(uint32_t cnt, const char* prefix) {
  char* p;
  if (prefix == NULL) p = "";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** TimingMsg @ %llu\n", p, (long long unsigned int)e->tOffs);
  printf("%sID 0x%08x%08x, Par 0x%08x%08x, Tef 0x%08x\n", p, (uint32_t)(e->t.id >> 32),
  (uint32_t)e->t.id, (uint32_t)(e->t.par >> 32), (uint32_t)e->t.par, e->t.tef);
}

TimingMsg::~TimingMsg() {}


Command::Command(uint64_t tOffs, uint16_t flags, uint64_t tValid, uint32_t act) : Event (tOffs, flags), tValid, act{
  this->tValid  = tValid;
  this->act     = act;
}

Command::show() {
  Command::Show(0, "");
}

Command::show(uint32_t cnt, const char* prefix) {
  char* p;
  if (prefix == NULL) p = "";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
  printf("%s*** Command   @ %llu\n", p, (long long unsigned int)e->tOffs);
  printf("%sValid @ %llu, Action 0x%08x\n", p, (long long unsigned int)e->c.tValid, e->c.act);
}

Command::~Command() {}




