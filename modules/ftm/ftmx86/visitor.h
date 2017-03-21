#ifndef _VISITOR_H_
#define _VISITOR_H_
#include <stdint.h>
#include <stdlib.h>
#include <iostream>

class Event;
class Command;
class Noop;
class TimeBlock;
class TimingMsg;
class Flow;
class Flush;



  class Visitor {
    std::ostream& out;
    void eventString(Event& el);
    void commandString(Command& el);
  public:
    Visitor(std::ostream& out) : out(out) {};
    ~Visitor() {};
    void visitVertex(TimeBlock& el);
		void visitVertex(TimingMsg& el);
    void visitVertex(Flow& el);
    void visitVertex(Flush& el);
    void visitVertex(Noop& el);

    void visitEdge(TimeBlock& el);
		void visitEdge(TimingMsg& el);
    void visitEdge(Flow& el);
    void visitEdge(Flush& el);
    void visitEdge(Noop& el);
  };

#endif
