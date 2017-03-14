#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <stdlib.h>
#include <stdint.h>
#include "timeblock.h"
enum prio {NONE, LOW, HIGH, INTERLOCK};



class Command : public Event {
  uint64_t tValid;

public:
  Command(uint64_t tOffs, uint16_t flags, uint64_t tValid) : Event (tOffs, flags), tValid(tValid) {}
  ~Command() {};
  void show(void);
  void show(uint32_t cnt, const char* sPrefix);
  uint8_t* serialise(uint8_t *pBuf, uint32_t &freeBytes);
};


class Noop : public Command {
  uint16_t qty;

public:
  Noop(uint64_t tOffs, uint16_t flags, uint64_t tValid, uint16_t qty) : Command( tOffs,  flags,  tValid) , qty(qty) {}
  ~Noop() {};
  void show(void);
  void show(uint32_t cnt, const char* sPrefix);
  uint8_t* serialise(uint8_t *pBuf, uint32_t &freeBytes);
};

class Flow : public Command {
  uint16_t qty;
  TimeBlock *blNext;

public:
  Flow(uint64_t tOffs, uint16_t flags, uint64_t tValid, uint16_t qty, TimeBlock *blNext)
      : Command( tOffs,  flags,  tValid) , qty(qty), blNext(blNext) {}
  ~Flow() {};
  void show(void);
  void show(uint32_t cnt, const char* sPrefix);
  uint8_t* serialise(uint8_t *pBuf, uint32_t &freeBytes);
};

class Flush : public Command {
  bool qIl;
  bool qHi;
  bool qLo;
  uint8_t upToHi;
  uint8_t upToLo;  

public:
  Flush(); 
  Flush(uint64_t tOffs, uint16_t flags, uint64_t tValid, bool qIl, bool qHi, bool qLo, uint8_t upToHi, uint8_t upToLo) 
        : Command( tOffs,  flags,  tValid) , qIl(qIl), qHi(qHi), qLo(qLo), upToHi(upToHi), upToLo(upToLo) {}
  ~Flush() {};
  void show(void);
  void show(uint32_t cnt, const char* sPrefix);
  void set(prio target, uint8_t upTo);
  void clear(prio target);
  uint8_t* serialise(uint8_t *pBuf, uint32_t &freeBytes);
};

#endif
