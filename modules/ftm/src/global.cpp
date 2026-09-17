#include <stdlib.h>
#include <stdio.h>
#include "global.h"
#include "ftm_common.h"
#include "log.h"


void Global::serialise(const mVal &m, uint8_t* b) const {
  Node::serialise(m, b);
  //dont't do thing - a global location is just an adress in the alloctable, the buffer is not used.
};

void Global::show(void) const {
  Global::show(0, "");
};

void Global::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == nullptr) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);

};


