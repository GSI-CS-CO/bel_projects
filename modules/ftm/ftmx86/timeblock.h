#ifndef _TIME_BLOCK_H_
#define _TIME_BLOCK_H_
#include <stdint.h>
#include <stdlib.h>
#include "node.h"
#include "visitor.h"

// timeblock, adds its own tPeriod to threads current block time. roughly eq. to beam process end. (e.g., evt-...-evt-Block ) 
class TimeBlock : public Node {
  uint64_t  period;

public:
  TimeBlock(std::string& name, uint32_t& hash, uint32_t flags, uint64_t period) : Node(name, hash, flags), period(period) {}
  ~TimeBlock()  {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }

  void show(void)       const {show(0, "");}
  void show(uint32_t cnt, const char* sPrefix)  const {printf("*** Block %s, Period %llu\n", sPrefix, (unsigned long long)period);}
  void serialise(itAdr dest, itAdr custom) const;
  

};





#endif
