#ifndef _EVENT_H_
#define _EVENT_H_

#include <stdlib.h>
#include <stdint.h>
#include "ftm_common.h"
#include "node.h"



enum prio {NONE, LOW, HIGH, INTERLOCK};

// An event to be sent over the timing network or bus. adds its own tOffs to threads current block time to obtain deadline
class Event : public Node {


protected:  
  uint64_t tOffs;
  


public:
  Event(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Node(name, hash, cpu, b, flags) {}
  Event(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs) : Node(name, hash, cpu, b, flags), tOffs(tOffs) {}
  virtual ~Event()  {};

  
  virtual void show(void)                               const = 0;
  virtual void show(uint32_t cnt, const char* sPrefix)  const = 0;
  virtual void accept(const VisitorVertexWriter& v)     const = 0;
  virtual void accept(const VisitorUploadCrawler& v)      const = 0;
  virtual void accept(const VisitorDownloadCrawler& v)    const = 0;
  const uint64_t getTOffs() const {return this->tOffs;}
  virtual void serialise(const vAdr &va) const;
  virtual void deserialise();
  
};

// std timing message for the ECA, sent over the timing network
class TimingMsg : public Event {
  uint64_t id;
  uint64_t par;
  uint32_t tef;
  uint32_t res;

public:
  TimingMsg(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Event(name, hash, cpu, b, flags) {}
  TimingMsg(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs,  uint64_t id, uint64_t par, uint32_t tef, uint32_t res) 
  : Event (name, hash, cpu, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_TMSG << NFLG_TYPE_POS)), tOffs), id(id), par(par), tef(tef), res(res) {}
  ~TimingMsg()  {};

  node_ptr clone() const { return boost::make_shared<TimingMsg>(*this); }

  void show(void)                                       const;
  void show(uint32_t cnt, const char* sPrefix)          const;
  void serialise(const vAdr &va) const;
  void deserialise();
  virtual void accept(const VisitorVertexWriter& v)         const override { v.visit(*this); }
  virtual void accept(const VisitorUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorDownloadCrawler& v)  const override { v.visit(*this); }
  const uint64_t getId() const {return this->id;}
  const uint64_t getPar() const {return this->par;}
  const uint32_t getTef() const {return this->tef;}
  const uint32_t getRes() const {return this->res;}

};

// A command to be sent over the bus to a Queue inside the DM, not executed before tValid
class Command : public Event {
protected: 
  uint64_t tValid;
  uint32_t act;


  Command(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Event(name, hash, cpu, b, flags), tValid(0), act(0) {}
  Command(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs, uint64_t tValid, uint32_t act) : Event (name, hash, cpu, b, flags, tOffs), tValid(tValid), act(act) {}
public:
  
  ~Command() {};

  virtual void show(void) const;
  virtual void show(uint32_t cnt, const char* sPrefix) const;

  virtual void accept(const VisitorVertexWriter& v)     const = 0;
  virtual void accept(const VisitorUploadCrawler& v)    const = 0;
  virtual void accept(const VisitorDownloadCrawler& v)  const = 0;
  virtual void serialise(const vAdr &va) const;
  virtual void deserialise();
  virtual const uint64_t getTValid()  const {return this->tValid;}
  virtual const uint32_t getAct()     const {return this->act;}
  virtual const void setAct(uint32_t act) {this->act |= act;}
  virtual const void clrAct(uint32_t act) {this->act &= ~act;}
  virtual const uint16_t getQty()     const {return (this->act >> ACT_QTY_POS) & ACT_QTY_MSK;}
  virtual const uint16_t getPrio()    const {return (this->act >> ACT_PRIO_POS) & ACT_PRIO_MSK;}
};

// Makes receiving Q do nothing when leaving block for N times
class Noop : public Command {


public:
  Noop(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Command(name, hash, cpu, b, flags) {}
  Noop(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs, uint64_t tValid, uint8_t prio, uint16_t qty) 
  : Command(name, hash, cpu, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_CNOOP << NFLG_TYPE_POS)), tOffs, tValid, (ACT_TYPE_NOOP << ACT_TYPE_POS) | (prio & ACT_PRIO_MSK) << ACT_PRIO_POS | (qty & ACT_QTY_MSK) << ACT_QTY_POS ) {}
  ~Noop() {};
  node_ptr clone() const { return boost::make_shared<Noop>(*this); }

  void show(void) const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise(const vAdr &va) const;
  void deserialise();
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
  virtual void accept(const VisitorUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorDownloadCrawler& v)  const override { v.visit(*this); }


};

// Makes receiving Q select destination when leaving block for N times
class Flow : public Command {

public:
  Flow(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Command(name, hash, cpu, b, flags) {}
  Flow(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs, uint64_t tValid, uint8_t prio, uint16_t qty)
      : Command(name, hash, cpu, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_CFLOW << NFLG_TYPE_POS)), tOffs, tValid, (ACT_TYPE_FLOW << ACT_TYPE_POS) | (prio & ACT_PRIO_MSK) << ACT_PRIO_POS | (qty & ACT_QTY_MSK) << ACT_QTY_POS )   {}
  ~Flow() {};
    node_ptr clone() const { return boost::make_shared<Flow>(*this); }

  void show(void) const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise(const vAdr &va) const;
  void deserialise();
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
  virtual void accept(const VisitorUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorDownloadCrawler& v)  const override { v.visit(*this); }
  

};

// Makes receiving Q add tWait instead of tPeriod to current time when leaving block once
class Wait : public Command {
  uint64_t tWait;
public:
  Wait(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Command(name, hash, cpu, b, flags) {}
  Wait(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs, uint64_t tValid,  uint8_t prio, uint64_t tWait) 
  : Command(name, hash, cpu, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_CWAIT << NFLG_TYPE_POS)), tOffs, tValid, (ACT_TYPE_WAIT << ACT_TYPE_POS) | (prio & ACT_PRIO_MSK) << ACT_PRIO_POS | 1 << ACT_QTY_POS), tWait(tWait) {}
  ~Wait() {};
  node_ptr clone() const { return boost::make_shared<Wait>(*this); }

  void show(void) const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise(const vAdr &va) const;
  void deserialise();
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
  virtual void accept(const VisitorUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorDownloadCrawler& v)  const override { v.visit(*this); }


};

// Makes receiving Q clear <prio> queue buffer when leaving block once
class Flush : public Command {
  uint8_t prio, mode;
  bool qIl, qHi, qLo;

  uint8_t frmIl, toIl;
  uint8_t frmHi, toHi;
  uint8_t frmLo, toLo;  


public:
  Flush(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Command(name, hash, cpu, b, flags) {}
  Flush(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs, uint64_t tValid, uint8_t prio, bool qIl, bool qHi, bool qLo ) 
        : Command(name, hash, cpu, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_CFLUSH << NFLG_TYPE_POS)), tOffs, tValid, (ACT_TYPE_FLUSH << ACT_TYPE_POS) | (prio & ACT_PRIO_MSK) << ACT_PRIO_POS | (1 & ACT_QTY_MSK) << ACT_QTY_POS), qIl(qIl), qHi(qHi), qLo(qLo), frmIl(0), toIl(0), frmHi(0), toHi(0), frmLo(0), toLo(0) {}
  Flush(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs, uint64_t tValid, uint8_t prio, bool qIl, bool qHi, bool qLo, uint8_t frmIl, uint8_t toIl, uint8_t frmHi, uint8_t toHi, uint8_t frmLo, uint8_t toLo) 
        : Command(name, hash, cpu, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_CFLUSH << NFLG_TYPE_POS)), tOffs, tValid, (ACT_TYPE_FLUSH << ACT_TYPE_POS) | (prio & ACT_PRIO_MSK) << ACT_PRIO_POS | (1 & ACT_QTY_MSK) << ACT_QTY_POS), qIl(qIl), qHi(qHi), qLo(qLo), frmIl(frmIl), toIl(toIl), frmHi(frmHi), toHi(toHi), frmLo(frmLo), toLo(toLo) {}
  ~Flush() {};
    node_ptr clone() const { return boost::make_shared<Flush>(*this); }

  void show(void)  const;
  void show(uint32_t cnt, const char* sPrefix)  const;
  const uint8_t getFlushPrio(void)  const {return ((this->act >> ACT_FLUSH_PRIO_POS) & ACT_FLUSH_PRIO_MSK);}
  const uint8_t getMode(void)       const {return ((this->act >> ACT_FLUSH_MODE_POS) & ACT_FLUSH_MODE_MSK);}
  const uint16_t getRng(uint8_t q) const;

  void serialise(const vAdr &va) const;
  void deserialise();
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
  virtual void accept(const VisitorUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorDownloadCrawler& v)  const override { v.visit(*this); }

};



#endif
