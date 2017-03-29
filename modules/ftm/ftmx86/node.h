#ifndef _NODE_H_
#define _NODE_H_
#include <stdint.h>
#include <stdlib.h>
#include "common.h"
#include "visitor.h"
#include "ftm_common.h"



class Node {
  vBuf vB;

public:
  int Dummy;
  Node() {}
  virtual ~Node() {};
  virtual uint64_t getTPeriod() const = 0;
  virtual uint64_t getTOffs() const = 0;
  virtual void serialise(itBuf ib)  = 0;
  virtual void show(void) const = 0;
  virtual void show(uint32_t cnt, const char* sPrefix)  const = 0;
  virtual void accept(const VisitorVertexWriter& v)     const = 0;
  virtual void accept(const VisitorCreateMemBlock & v)  const = 0;
  virtual void accept(const VisitorAddEvtChildren & v)  const = 0;
};





#endif
