#ifndef _NODE_H_
#define _NODE_H_
#include <string>
#include <stdint.h>
#include <stdlib.h>
#include "common.h"
#include "visitor.h"
#include "ftm_common.h"



class Node {


  //Allocator& al;
  const std::string& name;
  uint32_t flags;

  

public:
  
  Node(const std::string& name, uint32_t flags) : name(name), flags(flags) {} //what to do if it fails?
  virtual ~Node() {}
  //virtual ~Node() {al.deallocate(name);} //what to do if it fails?
  //Node(Allocator& al, const std:string& name, const uint32_t& hash) : al(al), flags(flags), name(name), {al.allocate(name);} //what to do if it fails?
  //virtual ~Node() {al.deallocate(name);} //what to do if it fails?

  //const std:string& getName() const {return name;}
  //const uint32_t& getHash()   const {//calc FNV hash}
  //const uint32_t getAdr()     const {return al.lookupAdr(name);}

  virtual void serialise() = 0;//const string &defaultDest, const string &target, const string vector &altDest, const string vector &meta)  = 0;
  virtual void show(void) const = 0;
  virtual void show(uint32_t cnt, const char* sPrefix)  const = 0;
  virtual void accept(const VisitorVertexWriter& v)     const = 0;
};





#endif
