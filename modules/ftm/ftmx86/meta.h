#ifndef _META_H_
#define _META_H_
#include <stdint.h>
#include <stdlib.h>
#include "node.h"
#include "visitor.h"


class Meta : public Node {

protected:
  

public:
  Meta(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) : Node(name, hash, b, flags) {}
  ~Meta()  {};
  virtual void accept(const VisitorVertexWriter& v)     const = 0;
  virtual void show(void)                               const = 0;
  virtual void show(uint32_t cnt, const char* sPrefix)  const = 0;
  virtual void serialise(vAdr &dest, vAdr &custom);
/*
  const std::string&  getName() const {return Node::getName();}
  const uint32_t&     getHash() const {return this->hash;}
  const uint32_t&     getFlags() const {return this->flags;}
  const bool isPainted() const {return Node::isPainted();}
  */

};


// Block, adds its own tPeriod to threads current block time. roughly eq. to beam process end. (e.g., evt-...-evt-Block ) 
class Block : public Meta {
  uint64_t  tPeriod;
  uint8_t rdIdxIl, rdIdxHi, rdIdxLo;
  uint8_t wrIdxIl, wrIdxHi, wrIdxLo;

public:
  Block(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tPeriod) 
  : Meta(name, hash, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_BLOCK << NFLG_TYPE_POS))), tPeriod(tPeriod) {}
  ~Block()  {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }

  void show(void)       const;
  void show(uint32_t cnt, const char* sPrefix)  const;
  void serialise(vAdr &dest, vAdr &custom);
  uint32_t getWrIdxs(void) const;
  uint32_t getRdIdxs(void) const;
  uint64_t getTPeriod() const {return this->tPeriod;}
};


//Command Queue - manages cmdq buffers and executes commands
class CmdQueue : public Meta {


public:
  CmdQueue(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) 
  : Meta(name, hash, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_QUEUE << NFLG_TYPE_POS))) {}
  ~CmdQueue()  {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }

  void show(void)       const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise(vAdr &dest, vAdr &custom);
  
};


//Command Queue Buffer - receives commands
class CmdQBuffer : public Meta {

public:
  CmdQBuffer(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) 
  : Meta(name, hash, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_QBUF << NFLG_TYPE_POS))) {}
  ~CmdQBuffer()  {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }

  void show(void)       const;
  void show(uint32_t cnt, const char* sPrefix)  const;
  void serialise(vAdr &dest, vAdr &custom);

};

//Alternative Destinations List - used to recreate edges between nodes from lm32 binary
class AltDestList : public Meta {

public:
  AltDestList(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags) 
  : Meta(name, hash, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_ALTDST << NFLG_TYPE_POS))) {}
  ~AltDestList()  {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }

  void show(void)       const;
  void show(uint32_t cnt, const char* sPrefix)  const;
  void serialise(vAdr &dest, vAdr &custom);

};



#endif
