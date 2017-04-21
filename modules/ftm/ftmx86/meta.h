#ifndef _META_H_
#define _META_H_
#include <stdint.h>
#include <stdlib.h>
#include "node.h"
#include "visitor.h"


class Meta : public Node {


public:
  Meta(std::string& name, uint32_t& hash, uint32_t flags) : Node(name, hash, flags) {}
  ~Meta()  {};
  virtual void accept(const VisitorVertexWriter& v)     const = 0;
  virtual void show(void)                               const = 0;
  virtual void show(uint32_t cnt, const char* sPrefix)  const = 0;
  virtual void serialise(itAdr dest, itAdr custom)      const = 0;
  

};


// timeblock, adds its own tPeriod to threads current block time. roughly eq. to beam process end. (e.g., evt-...-evt-Block ) 
class TimeBlock : public Meta {
  uint64_t  period;

public:
  TimeBlock(std::string& name, uint32_t& hash, uint32_t flags, uint64_t period) : Node(name, hash, flags), period(period) {}
  ~TimeBlock()  {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }

  void show(void)       const {show(0, "");}
  void show(uint32_t cnt, const char* sPrefix)  const {printf("*** Block %s, Period %llu\n", sPrefix, (unsigned long long)period);}
  void serialise(itAdr dest, itAdr custom) const;

};


//Command Queue - manages cmdq buffers and executes commands
class CmdQueue : public Meta {
  uint8_t rdIdxIl, rdIdxHi, rdIdxLo;
  uint8_t wrIdxIl, wrIdxHi, wrIdxLo;

public:
  CmdQueue(std::string& name, uint32_t& hash, uint32_t flags) : Node(name, hash, flags) {}
  CmdQueue(std::string& name, uint32_t& hash, uint32_t flags, ) : Node(name, hash, flags) {}
  ~CmdQueue()  {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }

  void show(void)       const {show(0, "");}
  void show(uint32_t cnt, const char* sPrefix)  const {printf("*** Block %s, Period %llu\n", sPrefix, (unsigned long long)period);}
  void serialise(itAdr dest, itAdr custom) const;
 

};


//Command Queue Buffer - receives commands
class CmdQBuffer : public Meta {

public:
  CmdQBuffer(std::string& name, uint32_t& hash, uint32_t flags, uint64_t period) : Node(name, hash, flags), period(period) {}
  ~CmdQBuffer()  {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }

  void show(void)       const {show(0, "");}
  void show(uint32_t cnt, const char* sPrefix)  const {printf("*** Block %s, Period %llu\n", sPrefix, (unsigned long long)period);}
  void serialise(itAdr dest, itAdr custom) const;
 

};





#endif
