#ifndef _VISITOR_DOWNLOAD_CRAWLER_H_
#define _VISITOR_DOWNLOAD_CRAWLER_H_
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include "common.h"
#include "graph.h"
#include "memunit.h"

class Event;
class Block;
class Meta;

class Command;
class Noop;
class TimingMsg;
class Flow;
class Flush;
class Wait;

class CmdQMeta;
class CmdQBuffer;
class DestList;

 class VisitorDownloadCrawler {
    vertex_t        v;
    MemUnit&        m;
    Graph&          g;
    AllocTable&     at;
    uint8_t*        b;
    const uint8_t&  cpu;
    uint32_t        tmpAdr;

    void setDefDst(void) const;

  public:
    VisitorDownloadCrawler(vertex_t v, MemUnit& m)  : v(v), m(m), g(m.getDownGraph()), at(m.getDownAllocTable()), b((uint8_t*)&g[v].np->getB()), cpu((uint8_t*)&g[v].np->getCpu()) {};
    ~VisitorDownloadCrawler() {};
    virtual void visit(const Block& el) const;
    virtual void visit(const TimingMsg& el) const;
    virtual void visit(const Flow& el) const;
    virtual void visit(const Flush& el) const;
    virtual void visit(const Noop& el) const;
    virtual void visit(const Wait& el) const;
    virtual void visit(const CmdQMeta& el) const;
    virtual void visit(const CmdQBuffer& el) const;
    virtual void visit(const DestList& el) const;

  };

#endif   