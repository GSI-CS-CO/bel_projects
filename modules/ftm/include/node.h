#ifndef _NODE_H_
#define _NODE_H_
#include <string>
#include <stdint.h>
#include <stdlib.h>
#include "common.h"
#include "visitoruploadcrawler.h"
#include "visitordownloadcrawler.h"
#include "visitorvertexwriter.h"
#include "visitorvalidation.h"
#include "ftm_common.h"
#include "dotstr.h"

using namespace DotStr::Misc;


class Node {

  


protected:    
  std::string name;
  //FIXME having more references to vertex properties here is not pretty, but we are in a hurry
  std::string pattern;
  std::string beamproc;

 
  const uint32_t     hash;
  const uint8_t      cpu;
  uint32_t      flags;
  

  


public:
  //Node(const std::string& name, const uint32_t& hash, const uint8_t& cpu, uint8_t* b, uint32_t flags) : name(name), pattern(sUndefined), beamproc(sUndefined), hash(hash), cpu(cpu), b(b), flags(flags)  {}
  Node(const std::string& name, const std::string&  pattern, const std::string&  beamproc, const uint32_t& hash, const uint8_t& cpu, uint32_t flags) 
  : name(name), pattern(pattern), beamproc(beamproc), hash(hash), cpu(cpu), flags(flags)  {} 
  Node(const Node& src) : name(src.name), pattern(src.pattern), beamproc(src.beamproc), hash(src.hash), cpu(src.cpu), flags(src.flags) {}
  virtual ~Node() = default;
  virtual node_ptr clone() const = 0; 
  

  std::string  getName() const {return std::string(this->name);}
  std::string  getPattern() const {return std::string(this->pattern);}
  std::string  getBeamproc() const {return std::string(this->beamproc);}
  const uint32_t&     getHash() const {return this->hash;}

  const uint8_t&      getCpu()  const {return this->cpu;}
  const uint32_t&     getFlags() const {return this->flags;}
  void     setFlags(uint32_t flags) {this->flags |= flags;}
  void     clrFlags(uint32_t flags) {this->flags &= ~flags;}
  //Check if this node is an entry or exit point to a pattern or beamprocess
  const bool isPainted()  const {return (bool)(this->flags & NFLG_PAINT_LM32_SMSK);}
  const bool isMarked()   const {return (bool)(this->flags & NFLG_PAINT_HOST_SMSK);}
  const bool isDebug0()   const {return (bool)(this->flags & NFLG_DEBUG0_SMSK);}
  const bool isDebug1()   const {return (bool)(this->flags & NFLG_DEBUG1_SMSK);}
  const bool isBpEntry()  const {return (bool)(this->flags & NFLG_BP_ENTRY_LM32_SMSK);}
  const bool isBpExit()   const {return (bool)(this->flags & NFLG_BP_EXIT_LM32_SMSK);}
  const bool isPatEntry() const {return (bool)(this->flags & NFLG_PAT_ENTRY_LM32_SMSK);}
  const bool isPatExit()  const {return (bool)(this->flags & NFLG_PAT_EXIT_LM32_SMSK);}


  virtual void show(void)                               const = 0;
  virtual void show(uint32_t cnt, const char* sPrefix)  const = 0;
  virtual void accept(const VisitorVertexWriter& v)     const = 0;
  virtual void accept(const VisitorUploadCrawler& v)    const = 0;
  virtual void accept(const VisitorDownloadCrawler& v)  const = 0;
  virtual void accept(const VisitorValidation& v)  const = 0;
  virtual bool isMeta(void) const {return false;}
  virtual bool isBlock(void) const {return false;}
  virtual bool isEvent(void) const {return false;}
  virtual void deserialise(uint8_t* b) = 0;
  virtual void serialise(const vAdr &va, uint8_t* b) const {
    std::memset(b, 0, _MEM_BLOCK_SIZE);
    writeLeNumberToBeBytes(b + (ptrdiff_t)NODE_DEF_DEST_PTR,  va[ADR_DEF_DST]);
    writeLeNumberToBeBytes(b + (ptrdiff_t)NODE_HASH,   this->hash);
    writeLeNumberToBeBytes(b + (ptrdiff_t)NODE_FLAGS,  this->flags);
  }

};





#endif
