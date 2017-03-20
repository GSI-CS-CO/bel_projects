#ifndef _EVENT_H_
#define _EVENT_H_

#include <stdlib.h>
#include <stdint.h>
#include "ftm_common.h"
#include "node.h"
#include "timeblock.h"
#include "visitor.h"




#define NO_SUCCESSOR -1
enum prio {NONE, LOW, HIGH, INTERLOCK};


class Event : public Node {
public:
  uint64_t tOffs;
  Event() {}
  Event(uint64_t tOffs, uint16_t flags) : tOffs(tOffs), flags(flags) {}
  virtual ~Event() {};
  virtual void show(void) = 0;
  virtual void show(uint32_t cnt, const char* sPrefix) = 0;
  virtual void serialise(itBuf ib) = 0;

protected:  

  uint16_t flags;
  void serialiseB(itBuf ib);
  virtual void acceptVertex(Visitor& v) = 0;
  virtual void acceptEdge(Visitor& v) = 0;
  
};

class TimingMsg : public Event {
  uint64_t id;
  uint64_t par;
  uint32_t tef;

public:
  TimingMsg(uint64_t tOffs, uint16_t flags, uint64_t id, uint64_t par, uint32_t tef) : Event (tOffs, flags), id(id), par(par), tef(tef) {}
  ~TimingMsg() {};
  uint64_t getId(){return id;}
  void show(void);
  void show(uint32_t cnt, const char* sPrefix);
  void serialise(itBuf ib);
  virtual void acceptVertex(Visitor& v) override { v.visitVertex(*this); }
  virtual void acceptEdge(Visitor& v) override { v.visitEdge(*this); }

};


class Command : public Event {
protected: 
  uint64_t tValid;
  void serialiseB(itBuf ib);

public:
  Command(uint64_t tOffs, uint16_t flags, uint64_t tValid) : Event (tOffs, flags), tValid(tValid) {}
  ~Command() {};
  virtual void show(void);
  virtual void show(uint32_t cnt, const char* sPrefix);
  virtual void serialise(itBuf ib) = 0;
	virtual void acceptVertex(Visitor& v) = 0;
  virtual void acceptEdge(Visitor& v) = 0;
};


class Noop : public Command {
  uint16_t qty;

public:
  Noop(uint64_t tOffs, uint16_t flags, uint64_t tValid, uint16_t qty) : Command( tOffs,  flags,  tValid) , qty(qty) {}
  ~Noop() {};
  void show(void);
  void show(uint32_t cnt, const char* sPrefix);
  void serialise(itBuf ib);
  virtual void acceptVertex(Visitor& v) override { v.visitVertex(*this); }
  virtual void acceptEdge(Visitor& v) override { v.visitEdge(*this); }
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
  void serialise(itBuf ib);
  virtual void acceptVertex(Visitor& v) override { v.visitVertex(*this); }
  virtual void acceptEdge(Visitor& v) override { v.visitEdge(*this); }
};

class Flush : public Command {
  bool qIl;
  bool qHi;
  bool qLo;
  uint8_t upToHi;
  uint8_t upToLo;  

;

public:
  Flush(uint64_t tOffs, uint16_t flags, uint64_t tValid, bool qIl, bool qHi, bool qLo ) 
        : Command( tOffs,  flags,  tValid) , qIl(qIl), qHi(qHi), qLo(qLo), upToHi(ACT_FLUSH_RANGE_ALL), upToLo(ACT_FLUSH_RANGE_ALL) {}
  Flush(uint64_t tOffs, uint16_t flags, uint64_t tValid, bool qIl, bool qHi, bool qLo, uint8_t upToHi, uint8_t upToLo) 
        : Command( tOffs,  flags,  tValid) , qIl(qIl), qHi(qHi), qLo(qLo), upToHi(upToHi), upToLo(upToLo) {}
  ~Flush() {};
  void show(void);
  void show(uint32_t cnt, const char* sPrefix);
  void set(prio target, uint8_t upTo);
  void clear(prio target);
  void serialise(itBuf ib);
  virtual void acceptVertex(Visitor& v) override { v.visitVertex(*this); }
  virtual void acceptEdge(Visitor& v) override { v.visitEdge(*this); }
};



#endif
