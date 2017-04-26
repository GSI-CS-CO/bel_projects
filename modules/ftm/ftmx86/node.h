#ifndef _NODE_H_
#define _NODE_H_
#include <string>
#include <stdint.h>
#include <stdlib.h>
#include "common.h"
#include "visitor.h"
#include "ftm_common.h"




class Node {

  std::string&  name;
  uint32_t&     hash;


protected:    


  uint32_t      flags;
  uint8_t       buf[_MEM_BLOCK_SIZE];

  virtual void serialise(vAdr &dest, vAdr &custom) {
    uint8_t* b = (uint8_t*)&(this->buf[0]);
    uint32ToBytes(b + (ptrdiff_t)NODE_HASH,   this->hash);
    uint32ToBytes(b + (ptrdiff_t)NODE_FLAGS,  this->flags);
    if (dest.size() < 1) { uint32ToBytes(b + (ptrdiff_t)NODE_DEF_DEST_PTR, LM32_NULL_PTR); }     //no successor, insert null ptr
    else                 { uint32ToBytes(b + (ptrdiff_t)NODE_DEF_DEST_PTR, dest[DEST_ADR_DEF]);}
  }


public:
  
  Node(std::string& name, uint32_t& hash, uint32_t flags) : name(name), hash(hash), flags(flags) {} //what to do if it fails?
  virtual ~Node() {}

  const std::string&  getName() const {return this->name;}
  const uint32_t&     getHash() const {return this->hash;}
  const uint32_t&     getFlags() const {return this->flags;}
  const bool isPainted() const {return (bool)(this->flags >> NFLG_PAINT_LM32_POS) & NFLG_PAINT_LM32_MSK;}


  virtual void show(void)                               const = 0;
  virtual void show(uint32_t cnt, const char* sPrefix)  const = 0;
  virtual void accept(const VisitorVertexWriter& v)     const = 0;
};





#endif
