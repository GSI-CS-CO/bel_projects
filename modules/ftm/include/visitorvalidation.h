#ifndef _VISITOR_VALIDATION_CRAWLER_H_
#define _VISITOR_VALIDATION_CRAWLER_H_

#include "common.h"
#include "dotstr.h"
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include "graph.h"
#include "validation.h"
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



class VisitorValidation {
    Graph&      g;
    vertex_t    v;
    AllocTable& at;
    int         cpu = -1;
    bool        force;


    //all of these have no return value as any violation is worth an exception
    void eventSequenceCheck() const { Validation::eventSequenceCheck(v,g, force); }; //check if event sequence is well behaved
    void metaSequenceCheck()  const { Validation::metaSequenceCheck(v,g);  }; //check if meta tree is well behaved


  public:
    VisitorValidation(Graph& g, vertex_t v, AllocTable& at, bool allowNegative)  : g(g), v(v), at(at), force(allowNegative) { auto x = at.lookupVertex(v); if (at.isOk(x)) cpu = x->cpu;}
    ~VisitorValidation() {};
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
