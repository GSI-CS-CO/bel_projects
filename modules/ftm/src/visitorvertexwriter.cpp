#include "visitorvertexwriter.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"



void VisitorVertexWriter::eventString(const Event& el) const {
  out << " [shape=\"oval\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"white\"";
  out << ", tOffs=" <<  std::dec << el.getTOffs() << ", flags=\"0x" << std::hex << el.getFlags();
}

void VisitorVertexWriter::commandString(const Command& el) const {
  out << ", tValid=" <<  std::dec << el.getTValid();
}

void VisitorVertexWriter::visit(const Block& el) const  { 
  out << " [type=\"Block\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"white\"";
  out << ", tPeriod=" <<  std::dec << el.getTPeriod();
  out << "]";
}

void VisitorVertexWriter::visit(const TimingMsg& el) const {
  eventString((Event&)el);
  out << "\", type=\"TMsg\", color=\"black\"";
  out << ", id=\"0x" << std::hex << el.getId();
  out << "\", par=\"0x" << std::hex << el.getPar();
  out << "\", tef=\"0x" << std::hex << el.getTef();
  out << "\", res=\"0x" << std::hex << el.getRes();
  out << "\"]";
}

void VisitorVertexWriter::visit(const Noop& el) const { 
  eventString((Event&)el);
  out << "\", type=\"Noop\", color=\"pink\"";
  commandString((Command&) el);
  out << ", qty=" <<  std::dec << el.getQty();
  out << "]";
}

void VisitorVertexWriter::visit(const Flow& el) const  { 
  eventString((Event&)el);
  out << "\", type=\"Flow\", color=\"magenta\"";
  commandString((Command&) el);
  out << ", qty=" <<  std::dec << el.getQty();
  out << "]";
}

void VisitorVertexWriter::visit(const Flush& el) const { 
  eventString((Event&)el);
  out << "\", type=\"Flush\", color=\"red\"";
  commandString((Command&) el);
  out << ", prio=" <<  std::dec << el.getPrio();
  out << "]";
}

void VisitorVertexWriter::visit(const Wait& el) const {
  out << " [shape=\"oval\"";
  if(el.isPainted()) out << ", color=\"green\"";
  else out << ", fillcolor=\"white\"";
  out << ", style=dashed, flags=\"0x" << std::hex << el.getFlags();
  out << "\"]";
}

void VisitorVertexWriter::visit(const CmdQMeta& el) const {
  out << " [type=\"QInfo\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"white\"";
  out << ", style=dashed, flags=\"0x" << std::hex << el.getFlags();
  out << "\"]";
}

void VisitorVertexWriter::visit(const CmdQBuffer& el) const {
  out << " [type=\"QBuf\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"white\"";
  out << ", style=dashed, flags=\"0x" << std::hex << el.getFlags();
  out << "\"]";
}

void VisitorVertexWriter::visit(const DestList& el) const {
  out << " [type=\"ListDst\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"white\"";
  out << ", style=dashed, flags=\"0x" << std::hex << el.getFlags();
  out << "\"]";
}

