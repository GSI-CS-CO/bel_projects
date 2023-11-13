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
class Switch;
class Origin;
class StartThread;
class Flush;
class Wait;

class CmdQMeta;
class CmdQBuffer;
class DestList;



class VisitorUploadCrawler {
    Graph&      g;
    vertex_t    v;
    AllocTable& at;
    std::ostream& sLog;
    std::ostream& sErr;
    int         cpu = -1;
    uint8_t*    b = nullptr;

    //void updateStaging() const;
    //void updateListDstStaging(amI x) const;
    //void updateBlockStaging() const;
    mVal getDefDst(void)    const;
    mVal getDynSrc(void)    const;
    mVal getQInfo(void)     const;
    mVal getQBuf(void)      const;
    mVal getCmdTarget(Command& el) const;
    mVal getSwitchTarget(void) const;
    mVal getFlowDst(void)   const;
    mVal getSwitchDst(void)   const;
    mVal getFlushOvr(void)  const;
    mVal getListDst(void)   const;
    mVal getRefLinks() const;
    mVal getValLinks() const;
    static const std::string exIntro;
    vertex_vec_t getChildrenByEdgeType(vertex_t vStart, const std::string edgeType) const;
    //vertex_set_t getChildrenByEdgeType(vertex_t vStart, const std::string edgeType) const;
    uint32_t getEdgeTargetAdr(vertex_t vSrc, vertex_t vDst) const;
    vertex_t getOnlyChildByEdgeType(vertex_t vStart, const std::string edgeType) const;
    vAdr& childrenAdrs(vertex_set_t vs, vAdr& ret, const unsigned int minResults = 1, const unsigned int maxResults = 1, const bool allowPeers = false, const uint32_t resultPadData = LM32_NULL_PTR) const;

  public:
    VisitorUploadCrawler(Graph& g, vertex_t v, AllocTable& at, std::ostream& sLog, std::ostream& sErr)  : g(g), v(v), at(at), sLog(sLog), sErr(sErr) { auto x = at.lookupVertex(v); cpu = x->cpu; b = (uint8_t*)x->b; }
    ~VisitorUploadCrawler() {};
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

  };

#endif
