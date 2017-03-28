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


protected:  
  uint64_t tOffs;
  uint16_t flags;
  void serialiseB(itBuf ib);


public:
  Event() {}
  Event(uint64_t tOffs, uint16_t flags) : tOffs(tOffs), flags(flags) {}
  virtual ~Event()  {};
  uint64_t getTPeriod() const { return -1;}
  uint64_t getTOffs() const { return tOffs;}
  uint16_t getFlags() const { return flags;}
  
  virtual void show(void)  const = 0;
  virtual void show(uint32_t cnt, const char* sPrefix) const = 0;
  virtual void serialise(itBuf ib) = 0;
  virtual void acceptVertex(const Visitor& v) const = 0;
  virtual void acceptEdge(const Visitor& v) const   = 0;
  virtual void acceptSerialiser(const Visitor& v) const override { v.visitSerialiser(*this); }

  
};

class TimingMsg : public Event {
  uint64_t id;
  uint64_t par;
  uint32_t tef;

public:
  TimingMsg(uint64_t tOffs, uint16_t flags, uint64_t id, uint64_t par, uint32_t tef) : Event (tOffs, flags), id(id), par(par), tef(tef) {}
  ~TimingMsg()  {};
  uint64_t getTOffs() const { return Event::getTOffs();}
  uint64_t getTPeriod() const { return Event::getTPeriod();}
  uint16_t getFlags() const { return Event::getFlags();} 
  uint64_t getId() const {return id;}
  uint64_t getPar() const {return par;}
  uint32_t getTef() const {return tef;}

  void show(void)  const;
  void show(uint32_t cnt, const char* sPrefix)  const;
  void serialise(itBuf ib);
  virtual void acceptVertex(const Visitor& v) const override { v.visitVertex(*this); }
  virtual void acceptEdge(const Visitor& v) const   override { v.visitEdge(*this); }
  virtual void acceptSerialiser(const Visitor& v) const override { Event::acceptSerialiser(v); }
};


class Command : public Event {
protected: 
  uint64_t tValid;
  void serialiseB(itBuf ib);


public:
  Command(uint64_t tOffs, uint16_t flags, uint64_t tValid) : Event (tOffs, flags), tValid(tValid) {}
  ~Command() {};
  uint64_t getTOffs() const { return Event::getTOffs();}
  uint16_t getFlags() const { return Event::getFlags();}
  uint64_t getTPeriod() const { return Event::getTPeriod();}
  uint64_t getTValid() const{ return tValid;}
  virtual void show(void) const;
  virtual void show(uint32_t cnt, const char* sPrefix) const;
  virtual void serialise(itBuf ib)     = 0;
	virtual void acceptVertex(const Visitor& v) const = 0;
  virtual void acceptEdge(const Visitor& v) const   = 0;
  virtual void acceptSerialiser(const Visitor& v) const { Event::acceptSerialiser(v); }


};


class Noop : public Command {
  uint16_t qty;

public:
  Noop(uint64_t tOffs, uint16_t flags, uint64_t tValid, uint16_t qty) : Command( tOffs,  flags,  tValid) , qty(qty) {}
  ~Noop() {};
  uint64_t getTOffs() const { return Command::getTOffs() ;}
  uint64_t getTPeriod() const { return Command::getTPeriod();}
  uint16_t getFlags() const { return Command::getFlags();} 
  uint64_t getTValid() const{ return Command::getTValid();}
  uint16_t getQty() const { return qty;} 

  void show(void) const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise(itBuf ib);
  virtual void acceptVertex(const Visitor& v) const override { v.visitVertex(*this); }
  virtual void acceptEdge(const Visitor& v) const   override { v.visitEdge(*this); }
  virtual void acceptSerialiser(const Visitor& v) const override { Command::acceptSerialiser(v); }
};

class Flow : public Command {
  uint16_t qty;
  TimeBlock *blNext;

public:
  Flow(uint64_t tOffs, uint16_t flags, uint64_t tValid, uint16_t qty, TimeBlock *blNext)
      : Command( tOffs,  flags,  tValid) , qty(qty), blNext(blNext) {}
  ~Flow() {};
  uint64_t getTOffs() const { return Command::getTOffs();}
  uint64_t getTPeriod() const { return Command::getTPeriod();}
  uint16_t getFlags() const { return Command::getFlags();} 
  uint64_t getTValid() const{ return Command::getTValid();}
  uint16_t getQty() const { return qty;} 
  TimeBlock* getNext() const { return blNext;}

  void show(void) const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise(itBuf ib);
  virtual void acceptVertex(const Visitor& v) const override { v.visitVertex(*this); }
  virtual void acceptEdge(const Visitor& v) const   override { v.visitEdge(*this); }
  virtual void acceptSerialiser(const Visitor& v) const override { Command::acceptSerialiser(v); }
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
  uint64_t getTOffs() const { return Command::getTOffs();}
  uint64_t getTPeriod() const { return Command::getTPeriod();}
  uint16_t getFlags() const { return Command::getFlags();} 
  uint64_t getTValid() const{ return Command::getTValid();}
  bool getFlushQil() const { return qIl;}
  bool getFlushQhi() const { return qHi;}
  bool getFlushQlo() const { return qLo;}
  uint8_t getFlushQ() const {return (uint8_t)((qIl << 2) | (qHi << 1) | (qLo << 0));}
  uint8_t getFlushUpToHi() const {return upToHi;}
  uint8_t getFlushUpToLo() const {return upToLo;} 


  void show(void)  const;
  void show(uint32_t cnt, const char* sPrefix)  const;
  void set(prio target, uint8_t upTo);
  void clear(prio target);
  void serialise(itBuf ib);
  virtual void acceptVertex(const Visitor& v) const override { v.visitVertex(*this); }
  virtual void acceptEdge(const Visitor& v) const   override { v.visitEdge(*this); }
  virtual void acceptSerialiser(const Visitor& v) const override { Command::acceptSerialiser(v); }
};



#endif
