#ifndef _META_H_
#define _META_H_
#include <stdint.h>
#include <stdlib.h>
#include "node.h"



class Meta : public Node {

protected:
  

public:
  Meta(const std::string& name, const std::string&  pattern, const std::string&  beamproc,  const uint32_t& hash, const uint8_t& cpu, uint8_t* b, uint32_t flags) : Node(name, pattern, beamproc, hash,  cpu, b, flags) {}
  Meta(const Meta& src) : Node(src) {}
  virtual ~Meta() = default;
  virtual void accept(const VisitorVertexWriter& v)     const = 0;
  virtual void accept(const VisitorUploadCrawler& v)    const = 0;
  virtual void accept(const VisitorDownloadCrawler& v)  const = 0;
  virtual void accept(const VisitorValidation& v)       const = 0;
  virtual void show(void)                               const = 0;
  virtual void show(uint32_t cnt, const char* sPrefix)  const = 0;
  virtual void serialise(const vAdr &va) const;
  void deserialise() {};
  bool isMeta(void) const {return true;}
/*
  const std::string&  getName() const {return Node::getName();}
  const uint32_t&     getHash() const {return this->hash;}
  const uint32_t&     getFlags() const {return this->flags;}
  const bool isPainted() const {return Node::isPainted();}
  */

};




//Command Queue - manages cmdq buffers and executes commands
class CmdQMeta : public Meta {


public:
  CmdQMeta(const std::string& name, const std::string&  pattern, const std::string&  beamproc,  const uint32_t& hash, const uint8_t& cpu, uint8_t* b, uint32_t flags) 
  : Meta(name, pattern, beamproc, hash, cpu, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_QUEUE << NFLG_TYPE_POS))) {}
  CmdQMeta(const CmdQMeta& src) : Meta(src) {}
  ~CmdQMeta()  {};
  node_ptr clone() const { return boost::make_shared<CmdQMeta>(CmdQMeta(*this)); }


  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
  virtual void accept(const VisitorUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorDownloadCrawler& v)  const override { v.visit(*this); }
  virtual void accept(const VisitorValidation& v)       const override { v.visit(*this); }

  void show(void)       const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise(const vAdr &va) const;
  
};


//Command Queue Buffer - receives commands
class CmdQBuffer : public Meta {

public:
  CmdQBuffer(const std::string& name, const std::string&  pattern, const std::string&  beamproc,  const uint32_t& hash, const uint8_t& cpu, uint8_t* b, uint32_t flags) 
  : Meta(name, pattern, beamproc, hash, cpu, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_QBUF << NFLG_TYPE_POS))) {}
  CmdQBuffer(const CmdQBuffer& src) : Meta(src) {}
  ~CmdQBuffer()  {};
  node_ptr clone() const { return boost::make_shared<CmdQBuffer>(CmdQBuffer(*this)); }

  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
  virtual void accept(const VisitorUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorDownloadCrawler& v)  const override { v.visit(*this); }
  virtual void accept(const VisitorValidation& v)       const override { v.visit(*this); }

  void show(void)       const;
  void show(uint32_t cnt, const char* sPrefix)  const;
  void serialise(const vAdr &va) const;

};

//Alternative Destinations List - used to recreate edges between nodes from lm32 binary
class DestList : public Meta {

public:
  DestList(const std::string& name, const std::string&  pattern, const std::string&  beamproc,  const uint32_t& hash, const uint8_t& cpu, uint8_t* b, uint32_t flags) 
  : Meta(name, pattern, beamproc, hash, cpu, b, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_ALTDST << NFLG_TYPE_POS))) {}
  ~DestList()  {};
  DestList(const DestList& src) : Meta(src) {}
  node_ptr clone() const { return boost::make_shared<DestList>(DestList(*this)); }

  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
  virtual void accept(const VisitorUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorDownloadCrawler& v)  const override { v.visit(*this); }
  virtual void accept(const VisitorValidation& v)       const override { v.visit(*this); }

  void show(void)       const;
  void show(uint32_t cnt, const char* sPrefix)  const;
  void serialise(const vAdr &va) const;

};



#endif
