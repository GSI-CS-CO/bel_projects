#ifndef _VISITOR_H_
#define _VISITOR_H_
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
//#include "common.h"
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



  class VisitorVertexWriter {
    std::ostream& out;
    void eventString(const Event& el) const;
    void commandString(const Command& el) const;
  public:
    VisitorVertexWriter(std::ostream& out) : out(out) {};
    ~VisitorVertexWriter() {};
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

  

  class VisitorNodeUploadCrawler {
    vertex_t v;
    MemUnit& mmu;

    vAdr getDefDst(void)    const;
    vAdr getQInfo(void)     const;
    vAdr getQBuf(void)      const;
    vAdr getCmdTarget(void) const;
    vAdr getFlowDst(void)   const;
    vAdr getListDst(void)   const;

  public:
    VisitorNodeUploadCrawler(vertex_t v, MemUnit& mmu)  : v(v), mmu(mmu) {};
    ~VisitorNodeUploadCrawler() {};
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

  class VisitorNodeDownloadCrawler {
    vertex_t v;
    MemUnit& mmu;

    void setDefDst(void) const;

  public:
    VisitorNodeDownloadCrawler(vertex_t v, MemUnit& mmu)  : v(v), mmu(mmu) {};
    ~VisitorNodeDownloadCrawler() {};
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


/*
 class VisitorEdgeWriter {
    std::ostream& out;
  public:
    VisitorEdgeWriter(std::ostream& out) : out(out) {};
    ~VisitorEdgeWriter() {};
    virtual void visit(const Block& el) const;
    virtual void visit(const TimingMsg& el) const;
    virtual void visit(const Flow& el) const;
    virtual void visit(const Flush& el) const;
    virtual void visit(const Noop& el) const;
  };

*/
/*

  class VisitorCreateMemBlock {
    vertex_t n;
    Graph& g;
    static bool NodeSortPredicate(const node_ptr e1, const node_ptr e2);
  public:
    VisitorCreateMemBlock(vertex_t n, Graph& g, vBuf& vB) : n(n), g(g) {};
    ~VisitorCreateMemBlock() {};
    virtual void visit(const Block& el) const;
    virtual void visit(const Event& el) const {}; 

     
  };

  class VisitorAddEvtChildren {
    vertex_t n;
    Graph& g;
    npBuf& npB;
  public:
    VisitorAddEvtChildren(vertex_t n, Graph& g, npBuf& npB) : n(n), g(g), npB(npB) {};
    ~VisitorAddEvtChildren() {};
    virtual void visit(const Block& el) const {}
    virtual void visit(const Event& el) const;
  };
*/
#endif
