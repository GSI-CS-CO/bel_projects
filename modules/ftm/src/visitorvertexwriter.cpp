#include "visitorvertexwriter.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"

using namespace VisitorVertexWriter;

void eventString(const Event& el) const {
  out << " [shape=\"oval\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"white\"";
  out << ", tOffs=" <<  std::dec << el.getTOffs() << ", flags=\"0x" << std::hex << el.getFlags();
}

void commandString(const Command& el) const {
  out << ", tValid=" <<  std::dec << el.getTValid();
}

void visit(const Block& el) const  { 
  out << " [type=\"Block\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"white\"";
  out << ", tPeriod=" <<  std::dec << el.getTPeriod();
  out << "]";
}

void visit(const TimingMsg& el) const {
  eventString((Event&)el);
  out << "\", type=\"TMsg\", color=\"black\"";
  out << ", id=\"0x" << std::hex << el.getId();
  out << "\", par=\"0x" << std::hex << el.getPar();
  out << "\", tef=\"0x" << std::hex << el.getTef();
  out << "\", res=\"0x" << std::hex << el.getRes();
  out << "\"]";
}

void visit(const Noop& el) const { 
  eventString((Event&)el);
  out << "\", type=\"Noop\", color=\"pink\"";
  commandString((Command&) el);
  out << ", qty=" <<  std::dec << el.getQty();
  out << "]";
}

void visit(const Flow& el) const  { 
  eventString((Event&)el);
  out << "\", type=\"Flow\", color=\"magenta\"";
  commandString((Command&) el);
  out << ", qty=" <<  std::dec << el.getQty();
  out << "]";
}

void visit(const Flush& el) const { 
  eventString((Event&)el);
  out << "\", type=\"Flush\", color=\"red\"";
  commandString((Command&) el);
  out << ", prio=" <<  std::dec << el.getPrio();
  out << "]";
}

void visit(const Wait& el) const {
  out << " [shape=\"oval\"";
  if(el.isPainted()) out << ", color=\"green\"";
  else out << ", fillcolor=\"white\"";
  out << ", style=dashed, flags=\"0x" << std::hex << el.getFlags();
  out << "\"]";
}

void visit(const CmdQMeta& el) const {
  out << " [type=\"QInfo\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"white\"";
  out << ", style=dashed, flags=\"0x" << std::hex << el.getFlags();
  out << "\"]";
}

void visit(const CmdQBuffer& el) const {
  out << " [type=\"QBuf\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"white\"";
  out << ", style=dashed, flags=\"0x" << std::hex << el.getFlags();
  out << "\"]";
}

void visit(const DestList& el) const {
  out << " [type=\"ListDst\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"white\"";
  out << ", style=dashed, flags=\"0x" << std::hex << el.getFlags();
  out << "\"]";
}

