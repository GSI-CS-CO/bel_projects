#include <stdlib.h>
#include <stdio.h>
#include "meta.h"
#include "ftm_common.h"






void Meta::serialise(const mVal &m, uint8_t* b) const {
  Node::serialise(m, b);
};

void CmdQMeta::serialise(const mVal &m, uint8_t* b) const {
  Meta::serialise(m, b);
  auto startIt = va.begin() + ADR_CMDQ_BUF_ARRAY;
  //FIXME size check !
  for(auto it = startIt; it < va.end(); it++) {
    writeLeNumberToBeBytes(b + (ptrdiff_t)CMDQ_BUF_ARRAY + (it - startIt) * _32b_SIZE_,  *it);
  }

};

void CmdQBuffer::serialise(const mVal &m, uint8_t* b) const {
  Meta::serialise(m, b);
};

void DestList::serialise(const mVal &m, uint8_t* b) const {
  Meta::serialise(m, b);

  //for each map entry, add the element to buffer


  auto startIt = va.begin();
  
  for(auto it = startIt; it < va.end(); it++) {
    writeLeNumberToBeBytes(b + (ptrdiff_t)DST_ARRAY + (it - startIt) * _32b_SIZE_,  *it);
  }

};

void CmdQMeta::show(void) const {
  CmdQMeta::show(0, "");
};

void CmdQMeta::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == nullptr) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
};

void CmdQBuffer::show(void) const {
  CmdQBuffer::show(0, "");
};

void CmdQBuffer::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == nullptr) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);
};

void DestList::show(void) const {
  DestList::show(0, "");
};

void DestList::show(uint32_t cnt, const char* prefix) const {
  char* p;
  if (prefix == nullptr) p = (char*)"";
  else p = (char*)prefix;
  printf("%s***------- %3u -------\n", p, cnt);

};

