#ifndef _BLOCK_H_
#define _BLOCK_H_
#include <stdint.h>
#include <stdlib.h>
#include "node.h"



class Block : public Node {

protected:
  
  uint64_t tPeriod;
  uint8_t rdIdxIl, rdIdxHi, rdIdxLo;
  uint8_t wrIdxIl, wrIdxHi, wrIdxLo;


public:
  Block(const std::string& name, const std::string&  pattern, const std::string&  beamproc, const uint32_t& hash, const uint8_t& cpu, uint32_t flags) 
  : Node(name, pattern, beamproc, hash, cpu, flags), tPeriod(0),
    rdIdxIl(0), rdIdxHi(0), rdIdxLo(0), wrIdxIl(0), wrIdxHi(0), wrIdxLo(0) {}
  Block(const std::string& name, const std::string&  pattern, const std::string&  beamproc, const uint32_t& hash, const uint8_t& cpu, uint32_t flags, uint64_t tPeriod) 
  : Node(name, pattern, beamproc, hash, cpu, flags), tPeriod(tPeriod),
    rdIdxIl(0), rdIdxHi(0), rdIdxLo(0), wrIdxIl(0), wrIdxHi(0), wrIdxLo(0) {}
  Block(const Block& src) : Node(src), tPeriod(src.tPeriod), rdIdxIl(src.rdIdxIl), rdIdxHi(src.rdIdxHi), rdIdxLo(src.rdIdxLo), wrIdxIl(src.wrIdxIl), wrIdxHi(src.wrIdxHi), wrIdxLo(src.wrIdxLo) {}
  ~Block()  {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
  virtual void accept(const VisitorUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorDownloadCrawler& v)  const override { v.visit(*this); }
  virtual void accept(const VisitorValidation& v)       const override { v.visit(*this); }

  void show(void)       const;
  void show(uint32_t cnt, const char* sPrefix)  const;
  void serialise(const vAdr &va, uint8_t* b) const;
  void deserialise(uint8_t* b);
  uint32_t getWrIdxs(void) const {return (uint32_t)((this->wrIdxIl << 16) | (this->wrIdxHi << 8) | (this->wrIdxLo << 0));};
  uint32_t getRdIdxs(void) const {return (uint32_t)((this->rdIdxIl << 16) | (this->rdIdxHi << 8) | (this->rdIdxLo << 0));};
  void setWrIdxs(uint32_t wrIdxs) {this->wrIdxIl = (wrIdxs >> 16) & 0xff; this->wrIdxHi = (wrIdxs >> 8) & 0xff; this->wrIdxLo = (wrIdxs >> 0) & 0xff; };
  void setRdIdxs(uint32_t rdIdxs) {this->rdIdxIl = (rdIdxs >> 16) & 0xff; this->rdIdxHi = (rdIdxs >> 8) & 0xff; this->rdIdxLo = (rdIdxs >> 0) & 0xff; };
  uint64_t getTPeriod() const {return this->tPeriod;}
  bool isBlock(void) const {return true;}
  virtual node_ptr clone() const = 0;

};

// except type field in fĺags, these are identical to superclass

class BlockFixed : public Block {

public:
  BlockFixed(const std::string& name, const std::string&  pattern, const std::string&  beamproc, const uint32_t& hash, const uint8_t& cpu, uint32_t flags) 
  : Block(name, pattern, beamproc, hash, cpu, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_BLOCK_FIXED << NFLG_TYPE_POS))) {}
  BlockFixed(const std::string& name, const std::string&  pattern, const std::string&  beamproc, const uint32_t& hash, const uint8_t& cpu, uint32_t flags, uint64_t tPeriod) 
  : Block(name, pattern, beamproc, hash, cpu, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_BLOCK_FIXED << NFLG_TYPE_POS)), tPeriod) {}
  BlockFixed(const BlockFixed& src) : Block(src) {}
  node_ptr clone() const override { 
    //node_ptr tmp = (node_ptr)( new BlockFixed(*this));
    //  std::cout << "BlockFixed Clone " << this->name << "this: " << this << " cpy " << tmp << std::endl; 
    return boost::make_shared<BlockFixed>(BlockFixed(*this)); 
  }
  

};



class BlockAlign : public Block {

public:
  BlockAlign(const std::string& name, const std::string&  pattern, const std::string&  beamproc, const uint32_t& hash, const uint8_t& cpu, uint32_t flags) 
  : Block(name, pattern, beamproc, hash, cpu, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_BLOCK_ALIGN << NFLG_TYPE_POS))) {}
  BlockAlign(const std::string& name, const std::string&  pattern, const std::string&  beamproc, const uint32_t& hash, const uint8_t& cpu, uint32_t flags, uint64_t tPeriod) 
  : Block(name, pattern, beamproc, hash, cpu, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_BLOCK_ALIGN << NFLG_TYPE_POS)), tPeriod) {}
  BlockAlign(const BlockAlign& src) : Block(src) {}
  node_ptr clone() const override { 
    //std::cout << "BlockAlign Clone " << this->name << std::endl; 
    return boost::make_shared<BlockAlign>(BlockAlign(*this)); 
  }
};




#endif
