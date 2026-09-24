#include <stdlib.h>
#include <stdio.h>
#include "meta.h"
#include "global.h"
#include "ftm_common.h"
#include "log.h"





void Meta::serialise(const mVal &m, uint8_t* b) const {
  Node::serialise(m, b);
};

void CmdQMeta::serialise(const mVal &m, uint8_t* b) const {
  Meta::serialise(m, b);

  for ( const auto &myPair : m ) {
    log<DEBUG>(L"Serialiser CmdQMeta: Inserting Adr: %1$#x Val %2$#x ")  % myPair.first % myPair.second;
    writeLeNumberToBeBytes(b + (ptrdiff_t)myPair.first,  myPair.second); 
  }

};

void CmdQBuffer::serialise(const mVal &m, uint8_t* b) const {
  Meta::serialise(m, b);
};

void DestList::serialise(const mVal &m, uint8_t* b) const {
  Meta::serialise(m, b);
  //FIXME how do we concat the maps if we have multiple things to add?

  //for each map entry, add the element to buffer
  for ( const auto &myPair : m ) {

    log<DEBUG>(L"Serialiser DstList: Inserting Adr: %1$#x Val %2$#x ")  % myPair.first % myPair.second;
    writeLeNumberToBeBytes(b + (ptrdiff_t)myPair.first,  myPair.second); 
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



