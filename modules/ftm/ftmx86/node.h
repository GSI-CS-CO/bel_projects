#ifndef _NODE_H_
#define _NODE_H_
#include <stdint.h>
#include <stdlib.h>
#include "common.h"
#include "visitor.h"



class Node {

public:
  int Dummy;
  Node() {}
  virtual ~Node() {};
  virtual uint64_t getTPeriod() const = 0;
  virtual uint64_t getTOffs() const = 0;
  virtual void serialise(itBuf ib)  = 0;
  virtual void show(void) const = 0;
  virtual void show(uint32_t cnt, const char* sPrefix) const = 0;
  virtual void acceptVertex(const Visitor& v) const = 0;
  virtual void acceptEdge(const Visitor& v) const = 0;
  virtual void acceptSerialiser(const Visitor& v) const = 0;
};





#endif
