#ifndef _TIME_BLOCK_H_
#define _TIME_BLOCK_H_
#include <stdint.h>
#include <stdlib.h>
#include "node.h"
#include "visitor.h"


class TimeBlock : public Node {
  uint8_t   cpu;
  uint16_t  idx;
  uint32_t  adr;
  uint64_t  period;
  bool      cmdQ;

public:
  TimeBlock(uint64_t start, uint64_t period, bool cmdQ) : period(period), cmdQ(cmdQ) {}
  TimeBlock(uint64_t period, bool cmdQ) : period(period), cmdQ(cmdQ) {}
  ~TimeBlock() {};
  virtual void acceptVertex(Visitor& v) override { v.visitVertex(*this); }
  virtual void acceptEdge(Visitor& v) override { v.visitEdge(*this); }
/*
           downloadTable()
  void     allocate(uint8_t cpu, uint16_t idx, uint32_t adr ); //replace by call to allocator
  void     deallocate(); //call to allocator free
*/
  uint16_t getIdx() {return idx;}
  void show(void) {show(0, "");}
  void show(uint32_t cnt, const char* sPrefix)  {printf("I'am a Timeblock\n");}
  void serialise(itBuf ib) {printf("I'am a serialised Timeblock\n");}
/*
  uint32_t getEvtQty() {return gsub.size();}
  uint32_t getSize() {} //call to allocator map
  bool     hasQ() {return cmdQ;}
*/

};





#endif
