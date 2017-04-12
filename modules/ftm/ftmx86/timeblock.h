#ifndef _TIME_BLOCK_H_
#define _TIME_BLOCK_H_
#include <stdint.h>
#include <stdlib.h>
#include "node.h"
#include "visitor.h"


class TimeBlock : public Node {
  uint64_t  period;

public:

  TimeBlock(const std::string& name, uint32_t flags, uint64_t period) : Node(name, flags), period(period) {}
  ~TimeBlock()  {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }

  //uint32_t getAdr()     const {return Node::getAdr();}
  void show(void)       const {show(0, "");}
  void show(uint32_t cnt, const char* sPrefix)  const {printf("*** Block %s, Period %llu\n", sPrefix, (unsigned long long)period);}
  void serialise()  {printf("I'am a serialised Timeblock\n");}
  

};





#endif
