#ifndef _VISITOR_DOWNLOAD_CRAWLER_H_
#define _VISITOR_DOWNLOAD_CRAWLER_H_
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include "common.h"
#include "graph.h"
#include "alloctable.h"
#include "covenanttable.h"


#include "visitorclasslist.h"



 class VisitorDownloadCrawler {
    Graph&          g;
    vertex_t        v;
    AllocTable&     at;
    CovenantTable&  ct;
    std::ostream& sLog;
    std::ostream& sErr;
    uint8_t*        b = nullptr;
    uint8_t         cpu;

    std::pair<uint8_t, AdrType> createCmd(const Command& el) const;
    std::pair<uint8_t, AdrType> createSwitch(const Switch& el) const;
    void setDefDst(void) const;
    void setRefLinks() const;
    static const std::string exIntro;

  public:
    VisitorDownloadCrawler(Graph& g, vertex_t v, AllocTable& at, CovenantTable& ct, std::ostream& sLog, std::ostream& sErr)  : g(g), v(v), at(at), ct(ct), sLog(sLog), sErr(sErr) { auto x = at.lookupVertex(v); cpu = x->cpu; b = (uint8_t*)x->b; };
    ~VisitorDownloadCrawler() {};
    virtual void visit(const Block& el) const;
    virtual void visit(const TimingMsg& el) const;
    virtual void visit(const Flow& el) const;
    virtual void visit(const Switch& el) const;
    virtual void visit(const Origin& el) const;
    virtual void visit(const StartThread& el) const;
    virtual void visit(const Flush& el) const;
    virtual void visit(const Noop& el) const;
    virtual void visit(const Wait& el) const;
    virtual void visit(const CmdQMeta& el) const;
    virtual void visit(const CmdQBuffer& el) const;
    virtual void visit(const DestList& el) const;
    virtual void visit(const Global& el) const;

  };

#endif