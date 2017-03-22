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
  virtual uint64_t getTPeriod() = 0;
  virtual uint64_t getTOffs() = 0;
  virtual void serialise(itBuf ib) = 0;
  virtual void show(void) = 0;
  virtual void show(uint32_t cnt, const char* sPrefix) = 0;
  virtual void acceptVertex(Visitor& v) = 0;
  virtual void acceptEdge(Visitor& v) = 0;
};





#endif
