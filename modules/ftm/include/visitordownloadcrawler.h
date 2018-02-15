#ifndef _VISITOR_DOWNLOAD_CRAWLER_H_
#define _VISITOR_DOWNLOAD_CRAWLER_H_
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



 class VisitorDownloadCrawler {
    Graph&          g;
    vertex_t        v;
    AllocTable&     at;
    std::ostream& sLog;
    std::ostream& sErr;
    uint8_t*        b;
    uint8_t         cpu;

    void setDefDst(void) const;
    static const std::string exIntro;

  public:
    VisitorDownloadCrawler(Graph& g, vertex_t v, AllocTable& at, std::ostream& sLog, std::ostream& sErr)  : g(g), v(v), at(at), sLog(sLog), sErr(sErr) { auto x = at.lookupVertex(v); if (at.isOk(x)) {cpu = x->cpu; b = ((AllocMeta*)&(*x))->b;} };
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