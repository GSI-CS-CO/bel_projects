#ifndef _EVENT_H_
#define _EVENT_H_

#include <stdlib.h>
#include <stdint.h>
#include "ftm_common.h"
#include "node.h"
#include "visitor.h"


enum prio {NONE, LOW, HIGH, INTERLOCK};

// An event to be sent over the timing network or bus. adds its own tOffs to threads current block time to obtain deadline
class Event : public Node {


protected:  
  uint64_t tOffs;
  


public:
  Event(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Node(name, hash, b, flags) {}
  Event(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs) : Node(name, hash, b, flags), tOffs(tOffs) {}
  virtual ~Event()  {};

  
  virtual void show(void)                               const = 0;
  virtual void show(uint32_t cnt, const char* sPrefix)  const = 0;
  virtual void accept(const VisitorVertexWriter& v)     const = 0;
  virtual void accept(const VisitorNodeUploadCrawler& v)      const = 0;
  virtual void accept(const VisitorNodeDownloadCrawler& v)    const = 0;
  const uint64_t getTOffs() const {return this->tOffs;}
  virtual void serialise(const vAdr &va) const;
  virtual void deserialise();
  bool isMeta(void) const {return false;}
};

// std timing message for the ECA, sent over the timing network
class TimingMsg : public Event {
  uint64_t id;
  uint64_t par;
  uint32_t tef;
  uint32_t res;

public:
  TimingMsg(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Event(name, hash, b, flags) {}
  TimingMsg(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs,  uint64_t id, uint64_t par, uint32_t tef, uint32_t res) 
  : Event (name, hash, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_TMSG << NFLG_TYPE_POS)), tOffs), id(id), par(par), tef(tef), res(res) {}
  ~TimingMsg()  {};


  void show(void)                                       const;
  void show(uint32_t cnt, const char* sPrefix)          const;
  void serialise(const vAdr &va) const;
  void deserialise();
  virtual void accept(const VisitorVertexWriter& v)         const override { v.visit(*this); }
  virtual void accept(const VisitorNodeUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorNodeDownloadCrawler& v)  const override { v.visit(*this); }
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

  Command(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Event(name, hash, b, flags) {}
  Command(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs, uint64_t tValid) : Event (name, hash, b, flags, tOffs), tValid(tValid) {}

public:
  
  ~Command() {};

  virtual void show(void) const;
  virtual void show(uint32_t cnt, const char* sPrefix) const;

  virtual void accept(const VisitorVertexWriter& v)         const = 0;
  virtual void accept(const VisitorNodeUploadCrawler& v)    const = 0;
  virtual void accept(const VisitorNodeDownloadCrawler& v)  const = 0;
  const uint64_t getTValid() const {return this->tValid;}
  const uint32_t getAct() const {return this->act;}
  virtual void serialise(const vAdr &va) const;
  virtual void deserialise();
};

// Makes receiving Q do nothing when leaving block for N times
class Noop : public Command {
  uint16_t qty;

public:
  Noop(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Command(name, hash, b, flags) {}
  Noop(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs, uint64_t tValid, uint16_t qty) 
  : Command(name, hash, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_CNOOP << NFLG_TYPE_POS)), tOffs, tValid) , qty(qty) {}
  ~Noop() {};


  void show(void) const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise(const vAdr &va) const;
  void deserialise();
  virtual void accept(const VisitorVertexWriter& v)         const override { v.visit(*this); }
  virtual void accept(const VisitorNodeUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorNodeDownloadCrawler& v)  const override { v.visit(*this); }
  const uint16_t getQty() const {return (this->flags >> ACT_QTY_POS) & ACT_QTY_MSK;}

};

// Makes receiving Q select destination when leaving block for N times
class Flow : public Command {
  uint16_t qty;

public:
  Flow(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Command(name, hash, b, flags) {}
  Flow(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs, uint64_t tValid, uint16_t qty)
      : Command(name, hash, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_CFLOW << NFLG_TYPE_POS)), tOffs, tValid) , qty(qty) {}
  ~Flow() {};


  void show(void) const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise(const vAdr &va) const;
  void deserialise();
  virtual void accept(const VisitorVertexWriter& v)         const override { v.visit(*this); }
  virtual void accept(const VisitorNodeUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorNodeDownloadCrawler& v)  const override { v.visit(*this); }
  const uint16_t getQty() const {return (this->flags >> ACT_QTY_POS) & ACT_QTY_MSK;}

};

// Makes receiving Q add tWait instead of tPeriod to current time when leaving block once
class Wait : public Command {
  uint16_t qty;
  uint64_t tWait;
public:
  Wait(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Command(name, hash, b, flags) {}
  Wait(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs, uint64_t tValid, uint64_t tWait) 
  : Command(name, hash, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_CWAIT << NFLG_TYPE_POS)), tOffs, tValid), tWait(tWait) {}
  ~Wait() {};


  void show(void) const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise(const vAdr &va) const;
  void deserialise();
  virtual void accept(const VisitorVertexWriter& v)         const override { v.visit(*this); }
  virtual void accept(const VisitorNodeUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorNodeDownloadCrawler& v)  const override { v.visit(*this); }


};

// Makes receiving Q clear <prio> queue buffer when leaving block once
class Flush : public Command {
  uint8_t prio, mode;
  bool qIl, qHi, qLo;

  uint8_t frmIl, toIl;
  uint8_t frmHi, toHi;
  uint8_t frmLo, toLo;  


public:
  Flush(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Command(name, hash, b, flags) {}
  Flush(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs, uint64_t tValid, bool qIl, bool qHi, bool qLo ) 
        : Command(name, hash, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_CFLUSH << NFLG_TYPE_POS)), tOffs, tValid) , qIl(qIl), qHi(qHi), qLo(qLo), frmIl(0), toIl(0), frmHi(0), toHi(0), frmLo(0), toLo(0) {}
  Flush(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tOffs, uint64_t tValid, bool qIl, bool qHi, bool qLo, uint8_t frmIl, uint8_t toIl, uint8_t frmHi, uint8_t toHi, uint8_t frmLo, uint8_t toLo) 
        : Command(name, hash, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_CFLUSH << NFLG_TYPE_POS)), tOffs, tValid), qIl(qIl), qHi(qHi), qLo(qLo), frmIl(frmIl), toIl(toIl), frmHi(frmHi), toHi(toHi), frmLo(frmLo), toLo(toLo) {}
  ~Flush() {};

  void show(void)  const;
  void show(uint32_t cnt, const char* sPrefix)  const;
  const uint8_t getPrio(void) const;
  const uint8_t getMode(void) const;
  const uint16_t getRng(uint8_t q) const;

  void serialise(const vAdr &va) const;
  void deserialise();
  virtual void accept(const VisitorVertexWriter& v)         const override { v.visit(*this); }
  virtual void accept(const VisitorNodeUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorNodeDownloadCrawler& v)  const override { v.visit(*this); }

};



#endif
