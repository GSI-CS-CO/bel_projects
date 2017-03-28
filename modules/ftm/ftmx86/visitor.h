#ifndef _VISITOR_H_
#define _VISITOR_H_
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include "common.h"

class Event;
class Command;
class Noop;
class TimeBlock;
class TimingMsg;
class Flow;
class Flush;



  class Visitor {
    std::ostream& out;
    static Graph defaultGraph;
    static vertex_t defaultVertex;
    vertex_t& n;
    Graph& g;
    void eventString(const Event& el) const;
    void commandString(const Command& el) const;
  public:
    Visitor(std::ostream& out) : out(out), n(defaultVertex), g(defaultGraph) {};
    Visitor(std::ostream& out, vertex_t& n, Graph& g) : out(out), n(n), g(g) {};
    ~Visitor() {};
    void visitVertex(const TimeBlock& el) const;
		void visitVertex(const TimingMsg& el) const;
    void visitVertex(const Flow& el) const;
    void visitVertex(const Flush& el) const;
    void visitVertex(const Noop& el) const;

    void visitSerialiser(const TimeBlock& el) const;
		void visitSerialiser(const Event& el) const;

    void visitEdge(const TimeBlock& el) const;
		void visitEdge(const TimingMsg& el) const;
    void visitEdge(const Flow& el) const;
    void visitEdge(const Flush& el) const;
    void visitEdge(const Noop& el) const;
  };

#endif
