#ifndef _BLOCK_H_
#define _BLOCK_H_
#include <stdint.h>
#include <stdlib.h>
#include "node.h"
#include "visitor.h"


class Block : public Node {

protected:
  
  uint64_t tPeriod;
  uint8_t rdIdxIl, rdIdxHi, rdIdxLo;
  uint8_t wrIdxIl, wrIdxHi, wrIdxLo;

public:
  Block(const std::string& name, const uint32_t& hash, uint8_t (&b)[_MEM_BLOCK_SIZE], uint32_t flags, uint64_t tPeriod) 
  : Node(name, hash, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_BLOCK << NFLG_TYPE_POS))), tPeriod(tPeriod),
    rdIdxIl(0), rdIdxHi(0), rdIdxLo(0), wrIdxIl(0), wrIdxHi(0), wrIdxLo(0) {}
  ~Block()  {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
  virtual void accept(const VisitorNodeCrawler& v)      const override { v.visit(*this); }

  void show(void)       const;
  void show(uint32_t cnt, const char* sPrefix)  const;
  void serialise(const vAdr &va) const;
  uint32_t getWrIdxs(void) const;
  uint32_t getRdIdxs(void) const;
  uint64_t getTPeriod() const {return this->tPeriod;}
  bool isMeta(void) const {return false;}

};






#endif
