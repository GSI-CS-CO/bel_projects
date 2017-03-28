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
  //TimeBlock(uint64_t start, uint64_t period, bool cmdQ) : period(period), cmdQ(cmdQ) {}
  TimeBlock(uint64_t period, bool cmdQ) : cpu(0), idx(0), adr(0), period(period), cmdQ(cmdQ) {}
  ~TimeBlock()  {};
  virtual void acceptVertex(const Visitor& v) const override { v.visitVertex((const TimeBlock&)*this); }
  virtual void acceptEdge(const Visitor& v) const override { v.visitEdge((const TimeBlock&)*this); }
  virtual void acceptSerialiser(const Visitor& v) const override { v.visitSerialiser((const TimeBlock&)*this); }
/*
           downloadTable() const
  void     allocate(uint8_t cpu, uint16_t idx, uint32_t adr ); //replace by call to allocator
  void     deallocate() const; //call to allocator free
*/
  uint8_t  getCpu() const     {return cpu;}
  uint16_t getIdx() const     {return idx;}
  uint32_t getAdr() const     {return adr;}
  uint64_t getTOffs() const   {return -1;}
  uint64_t getTPeriod() const  {return period;}
  bool hasCmdQ() const {return cmdQ;}
  void show(void) const {show(0, "");}
  void show(uint32_t cnt, const char* sPrefix)  const {printf("I'am a Timeblock\n");}
  void serialise(itBuf ib)  {printf("I'am a serialised Timeblock\n");}

  
/*
  uint32_t getEvtQty() const {return gsub.size() const;}
  uint32_t getSize() const {} //call to allocator map
  bool     hasQ() const {return cmdQ;}
*/

};





#endif
