#ifndef _VISITOR_UPLOAD_CRAWLER_H_
#define _VISITOR_UPLOAD_CRAWLER_H_
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include "common.h"
#include "graph.h"
#include "alloctable.h"

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

class VisitorUploadCrawler {
    Graph&      g;
    vertex_t    v;
    AllocTable& at;
    int         cpu = -1;

    void updateStaging() const;
    void updateListDstStaging(amI x) const;
    void updateBlockStaging() const;
    vAdr getDefDst(void)    const;
    vAdr getDynSrc(void)    const;
    vAdr getQInfo(void)     const;
    vAdr getQBuf(void)      const;
    vAdr getCmdTarget(Command& el) const;
    vAdr getFlowDst(void)   const;
    vAdr getListDst(void)   const;

  public:
    VisitorUploadCrawler(Graph& g, vertex_t v, AllocTable& at)  : g(g), v(v), at(at) { auto x = at.lookupVertex(v); if (at.isOk(x)) cpu = x->cpu;}
    ~VisitorUploadCrawler() {};
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