#ifndef _ACTION_H_
#define _ACTION_H_

#include <stdlib.h>
#include <stdint.h>
#include "timeblock.h"
enum prio {NONE, LOW, HIGH, INTERLOCK};



class Action {

public:
  Action() {}
  virtual ~Action() {};
  virtual void show(void) = 0;
  virtual void show(uint32_t cnt, const char* sPrefix) = 0;
  virtual uint8_t* serialise(uint8_t *pBuf, uint32_t &freeBytes) = 0;
};

class Noop : public Action {
  uint16_t qty;

public:
  Noop(uint16_t qty) : qty(qty) {}
  ~Noop() {};
  void show(void);
  void show(uint32_t cnt, const char* sPrefix);
  uint8_t* serialise(uint8_t *pBuf, uint32_t &freeBytes);
};

class Flow : public Action {
  uint16_t qty;
  TimeBlock *blNext;

public:
  Flow(uint16_t qty, TimeBlock *blNext) : qty(qty), blNext(blNext) {}
  ~Flow() {};
  void show(void);
  void show(uint32_t cnt, const char* sPrefix);
  uint8_t* serialise(uint8_t *pBuf, uint32_t &freeBytes);
};

class Flush : public Action {
  bool qIl;
  bool qHi;
  bool qLo;
  uint8_t upToHi;
  uint8_t upToLo;  

public:
  Flush(); 
  Flush(bool qIl, bool qHi, bool qLo, uint8_t upToHi, uint8_t upToLo)
                   : qIl(qIl), qHi(qHi), qLo(qLo), upToHi(upToHi), upToLo(upToLo) {}
  ~Flush() {};
  void show(void);
  void show(uint32_t cnt, const char* sPrefix);
  void set(prio target, uint8_t upTo);
  void clear(prio target);
  uint8_t* serialise(uint8_t *pBuf, uint32_t &freeBytes);
};

#endif
