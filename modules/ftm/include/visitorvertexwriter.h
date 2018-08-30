#ifndef _VISITOR_VERTEX_WRITER_H_
#define _VISITOR_VERTEX_WRITER_H_
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include "common.h"

class Node;

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


enum class FormatNum {DEC, HEX, HEX16, HEX32, HEX64, BIT, BOOL};



 class VisitorVertexWriter {
    std::ostream& out;
    void pushStart() const { out << "["; };
    void pushEnd()   const { out << "]"; };
    void pushPair(const std::string& p, uint64_t v, FormatNum format) const;
    void pushPair(const std::string& p, const std::string& v) const;
    void pushSingle(const std::string& p) const;
    void pushNodeInfo(const Node& el) const;
    void pushEventInfo(const Event& el) const;
    void pushCommandInfo(const Command& el) const;
    void pushPaintedEyecandy(const Node& el) const;
    void pushStartEyecandy(const Node& el) const;
    void pushStopEyecandy(const Node& el) const;
    void pushMembershipInfo(const Node& el) const;
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

#endif