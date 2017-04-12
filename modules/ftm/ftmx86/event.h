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
  void serialiseB(itBuf ib);


public:
  Event(const std::string& name, uint32_t flags, uint64_t tOffs) : Node(name, flags), tOffs(tOffs) {}
  virtual ~Event()  {};

  
  virtual void show(void)                               const = 0;
  virtual void show(uint32_t cnt, const char* sPrefix)  const = 0;
  virtual void serialise()                            = 0;
  virtual void accept(const VisitorVertexWriter& v)     const = 0;

  
};

class TimingMsg : public Event {
  uint64_t id;
  uint64_t par;
  uint32_t tef;

public:
  TimingMsg(const std::string& name, uint32_t flags, uint64_t tOffs,  uint64_t id, uint64_t par, uint32_t tef) : Event (name, flags, tOffs), id(id), par(par), tef(tef) {}
  ~TimingMsg()  {};


  void show(void)                                       const;
  void show(uint32_t cnt, const char* sPrefix)          const;
  void serialise() {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
};


class Command : public Event {
protected: 
  uint64_t tValid;
  void serialiseB(itBuf ib);
  Command(const std::string& name, uint32_t flags, uint64_t tOffs, uint64_t tValid) : Event (name, flags, tOffs), tValid(tValid) {}

public:
  
  ~Command() {};

  virtual void show(void) const;
  virtual void show(uint32_t cnt, const char* sPrefix) const;
  virtual void serialise()     = 0;
	virtual void accept(const VisitorVertexWriter& v)     const = 0;


};


class Noop : public Command {
  uint16_t qty;

public:
  Noop(const std::string& name, uint32_t flags, uint64_t tOffs, uint64_t tValid, uint16_t qty) : Command(name, flags, tOffs, tValid) , qty(qty) {}
  ~Noop() {};


  void show(void) const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise() {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
};

class Flow : public Command {
  uint16_t qty;

public:
  Flow(const std::string& name, uint32_t flags, uint64_t tOffs, uint64_t tValid, uint16_t qty)
      : Command(name, flags, tOffs, tValid) , qty(qty) {}
  ~Flow() {};


  void show(void) const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise() {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }

};

class Flush : public Command {
  bool qIl;
  bool qHi;
  bool qLo;
  uint8_t upToHi;
  uint8_t upToLo;  

;

public:
  Flush(const std::string& name, uint32_t flags, uint64_t tOffs, uint64_t tValid, bool qIl, bool qHi, bool qLo ) 
        : Command(name, flags, tOffs, tValid) , qIl(qIl), qHi(qHi), qLo(qLo), upToHi(ACT_FLUSH_RANGE_ALL), upToLo(ACT_FLUSH_RANGE_ALL) {}
  Flush(const std::string& name, uint32_t flags, uint64_t tOffs, uint64_t tValid, bool qIl, bool qHi, bool qLo, uint8_t upToHi, uint8_t upToLo) 
        : Command(name, flags, tOffs, tValid) , qIl(qIl), qHi(qHi), qLo(qLo), upToHi(upToHi), upToLo(upToLo) {}
  ~Flush() {};

  void show(void)  const;
  void show(uint32_t cnt, const char* sPrefix)  const;
  void set(prio target, uint8_t upTo);
  void clear(prio target);
  void serialise() {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
};



#endif
