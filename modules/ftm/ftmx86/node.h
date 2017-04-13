#ifndef _NODE_H_
#define _NODE_H_
#include <string>
#include <stdint.h>
#include <stdlib.h>
#include "common.h"
#include "visitor.h"




class Node {


  std::string&  name;
  uint32_t&     hash;
  uint32_t      flags;
  uint8_t       buf[MEM_BLOCK_SIZE];  
 

public:
  
  Node(std::string& name, uint32_t& hash, uint32_t flags) : name(name), hash(hash), flags(flags) {} //what to do if it fails?
  virtual ~Node() {}

  const std:string& getName() const {return name;}
  const uint32_t&   getHash() const {return hash;}

  void serialiseB(itAdr dest) {
    //insert hash, flags and default destination
    uint32ToBytes((uint8_t*)&buf[NODE_HASH,  this->hash);    
    uint32ToBytes((uint8_t*)&buf[NODE_FLAGS, this->flags);    
    if (dest.size() < 1) uint32ToBytes((uint8_t*)&buf[NODE_DEFDEST],   A_LM32_NULL_PTR); //quietly mutter that there is default Destination for this node. Insert NULL pointer
    else                 uint32ToBytes((uint8_t*)&buf[NODE_DEFDEST],   this->dest[A_DEFAULT]);
  }
  virtual void serialise(itAdr dest, itAdr custom) const = 0;
  virtual void show(void) const = 0;
  virtual void show(uint32_t cnt, const char* sPrefix)  const = 0;
  virtual void accept(const VisitorVertexWriter& v)     const = 0;
};





#endif
