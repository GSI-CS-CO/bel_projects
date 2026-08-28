#ifndef _GLOBAL_H_
#define _GLOBAL_H_
#include <stdint.h>
#include <stdlib.h>
#include "node.h"

class Global : public Node {

std::string section;

public:
  Global(const std::string& name, const std::string&  pattern, const std::string&  beamproc,  const uint32_t& hash, const uint8_t& cpu, uint32_t flags, const std::string& section)
  : Node(name, pattern, beamproc, hash, cpu, ((flags & ~NFLG_TYPE_SMSK) | (NODE_TYPE_GLOBAL << NFLG_TYPE_POS))), section(section) {}
  Global(const Global& src) : Node(src), section(src.section) {}
  ~Global()  {};
  node_ptr clone() const { return boost::make_shared<Global>(Global(*this)); }


  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
  virtual void accept(const VisitorUploadCrawler& v)    const override { v.visit(*this); }
  virtual void accept(const VisitorDownloadCrawler& v)  const override { v.visit(*this); }
  virtual void accept(const VisitorValidation& v)       const override { v.visit(*this); }

  void show(void)       const;
  void show(uint32_t cnt, const char* sPrefix) const;
  void serialise(const mVal &m, uint8_t* b) const;
  void deserialise(uint8_t* b) {};
  bool isMeta(void) const {return false;}
  std::string getSection() const {return this->section;}

};

#endif
