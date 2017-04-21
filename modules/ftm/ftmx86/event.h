#ifndef _EVENT_H_
#define _EVENT_H_

#include <stdlib.h>
#include <stdint.h>
#include "ftm_common.h"
#include "node.h"
#include "visitor.h"




#define NO_SUCCESSOR -1
enum prio {NONE, LOW, HIGH, INTERLOCK};

// An event to be sent over the timing network or bus. adds its own tOffs to threads current block time to obtain deadline
class Event : public Node {


protected:  
  uint64_t tOffs;
  void serialiseB(itAdr dest);


public:
  Event(std::string& name, uint32_t& hash, uint32_t flags, uint64_t tOffs) : Node(name, hash, flags), tOffs(tOffs) {}
  virtual ~Event()  {};

  
  virtual void show(void)                               const = 0;
  virtual void show(uint32_t cnt, const char* sPrefix)  const = 0;
  virtual void serialise(itAdr dest, itAdr custom)      const = 0;
  virtual void accept(const VisitorVertexWriter& v)     const = 0;

  
};

// std timing message for the ECA, sent over the timing network
class TimingMsg : public Event {
  uint64_t id;
  uint64_t par;
  uint32_t tef;

public:
  TimingMsg(std::string& name, uint32_t& hash, uint32_t flags, uint64_t tOffs,  uint64_t id, uint64_t par, uint32_t tef) : Event (name, hash, flags, tOffs), id(id), par(par), tef(tef) {}
  ~TimingMsg()  {};


  void show(void)                                       const;
  void show(uint32_t cnt, const char* sPrefix)          const;
  void serialise(itAdr dest, itAdr custom) {};          const;
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
};

// A command to be sent over the bus to a Queue inside the DM, not executed before tValid
class Command : public Event {
protected: 
  uint64_t tValid;
  void serialiseB(itAdr dest, itAdr custom);
  Command(std::string& name, uint32_t& hash, uint32_t flags, uint64_t tOffs, uint64_t tValid) : Event (name, hash, flags, tOffs), tValid(tValid) {}

public:
  
  ~Command() {};

  virtual void show(void) const;
  virtual void show(uint32_t cnt, const char* sPrefix) const;
  virtual void serialise(itAdr dest, itAdr custom)     const = 0;
	virtual void accept(const VisitorVertexWriter& v)    const = 0;


};

// Makes receiving Q do nothing when leaving block for N times
class Noop : public Command {
  uint16_t qty;

public:
  Noop(std::string& name, uint32_t& hash, uint32_t flags, uint64_t tOffs, uint64_t tValid, uint16_t qty) : Command(name, hash, flags, tOffs, tValid) , qty(qty) {}
  ~Noop() {};


  void show(void) const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise(itAdr dest, itAdr custom) const;
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
};

// Makes receiving Q select destination when leaving block for N times
class Flow : public Command {
  uint16_t qty;

public:
  Flow(std::string& name, uint32_t& hash, uint32_t flags, uint64_t tOffs, uint64_t tValid, uint16_t qty)
      : Command(name, hash, flags, tOffs, tValid) , qty(qty) {}
  ~Flow() {};


  void show(void) const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise(itAdr dest, itAdr custom) const;
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }

};

// Makes receiving Q add tWait instead of tPeriod to current time when leaving block once
class Wait : public Command {
  uint16_t qty;

public:
  
  Wait(std::string& name, uint32_t& hash, uint32_t flags, uint64_t tOffs, uint64_t tValid, uint64_t tWait) : Command(name, hash, flags, tOffs, tValid), tWait(tWait) {}
  ~Wait() {};


  void show(void) const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise(itAdr dest, itAdr custom) const;
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
};

// Makes receiving Q clear <prio> queue buffer when leaving block once
class Flush : public Command {
  bool qIl;
  bool qHi;
  bool qLo;
  uint8_t upToHi;
  uint8_t upToLo;  

;

public:
  Flush(std::string& name, uint32_t& hash, uint32_t flags, uint64_t tOffs, uint64_t tValid, bool qIl, bool qHi, bool qLo ) 
        : Command(name, hash, flags, tOffs, tValid) , qIl(qIl), qHi(qHi), qLo(qLo), upToHi(ACT_FLUSH_RANGE_ALL), upToLo(ACT_FLUSH_RANGE_ALL) {}
  Flush(std::string& name, uint32_t& hash, uint32_t flags, uint64_t tOffs, uint64_t tValid, bool qIl, bool qHi, bool qLo, uint8_t upToHi, uint8_t upToLo) 
        : Command(name, hash, flags, tOffs, tValid) , qIl(qIl), qHi(qHi), qLo(qLo), upToHi(upToHi), upToLo(upToLo) {}
  ~Flush() {};

  void show(void)  const;
  void show(uint32_t cnt, const char* sPrefix)  const;
  void set(prio target, uint8_t upTo);
  void clear(prio target);
  void serialise(itAdr dest, itAdr custom) const;
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
};



#endif
