#include "visitorvertexwriter.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"
#include "idformat.h"

namespace ec  = DotStr::EyeCandy;
namespace dgp = DotStr::Graph::Prop;
namespace dnp = DotStr::Node::Prop;
namespace dnt = DotStr::Node::TypeVal;
namespace dep = DotStr::Edge::Prop;
namespace det = DotStr::Edge::TypeVal;
namespace ssi = DotStr::Node::Prop::TMsg::SubId;

using namespace DotStr::Misc;

void VisitorVertexWriter::pushPair(const std::string& p, uint64_t v, int format) const {
  out << ", " << p << "=\"";
  switch (format) {
    case formatnum::HEX   : out << "0x" << std::hex << v; break;
    case formatnum::HEX16   : out << "0x" << std::hex  << std::setfill('0') << std::setw(4) << v; break;
    case formatnum::HEX32   : out << "0x" << std::hex  << std::setfill('0') << std::setw(8) << v; break;
    case formatnum::HEX64   : out << "0x" << std::hex  << std::setfill('0') << std::setw(16) << v; break;
    case formatnum::DEC   : out << std::dec << v; break;
    case formatnum::BIT   : out << std::dec << (v & 1); break;
    case formatnum::BOOL  : out << ((v & 1) ? sTrue : sFalse); break;
    default           : out << std::dec << v;
  }  
  out << "\"";
}

void VisitorVertexWriter::pushPair(const std::string& p, const std::string& v) const {
  out << ", " << p << "=\"" << v << "\"";
}

void VisitorVertexWriter::pushSingle(const std::string& p) const  {
  out << ", " << p;
}

void VisitorVertexWriter::pushMembershipInfo(const Node& el) const {

  //if (el.getPattern() != sUndefined) {
    pushPair(dnp::Base::sPatName, el.getPattern());
    pushPair(dnp::Base::sPatEntry, (int)el.isPatEntry(), formatnum::BOOL);
    pushPair(dnp::Base::sPatExit, (int)el.isPatExit(), formatnum::BOOL);
  //}  
  //if (el.getBeamproc() != sUndefined) {
    pushPair(dnp::Base::sBpName, el.getBeamproc());
    pushPair(dnp::Base::sBpEntry, (int)el.isBpEntry(), formatnum::BOOL);
    pushPair(dnp::Base::sBpExit, (int)el.isBpExit(), formatnum::BOOL);
  //}
}  

void VisitorVertexWriter::pushNodeInfo(const Node& el) const {
  pushStart();
  //can't use our helper for first property, as we cannot have a comma
  out << dnp::Base::sCpu   << "=\"" <<  (int)el.getCpu() << "\"";
  pushPair(dnp::Base::sFlags, el.getFlags(), formatnum::HEX32);

}

void VisitorVertexWriter::pushEventInfo(const Event& el) const {
  pushPair(dnp::TMsg::sTimeOffs, el.getTOffs(), formatnum::DEC);
  pushMembershipInfo((Node&)el); 

}

void VisitorVertexWriter::pushCommandInfo(const Command& el) const {
  pushPair(dnp::Cmd::sTimeValid, el.getTValid(), formatnum::DEC);
}

void VisitorVertexWriter::pushPaintedEyecandy(const Node& el) const {
  pushSingle((el.isPainted() ? ec::Node::Base::sLookPaint0 : ec::Node::Base::sLookPaintNone));
}

void VisitorVertexWriter::pushStartEyecandy(const Node& el) const {
  if(el.isPatEntry()) pushSingle(ec::Node::Base::sLookPatEntry);
}

void VisitorVertexWriter::pushStopEyecandy(const Node& el) const {
  if(el.isPatExit()) pushSingle(ec::Node::Base::sLookPatExit);
}



void VisitorVertexWriter::visit(const Block& el) const  {
  pushNodeInfo((Node&)el); 
  pushPair(dnp::Base::sType, dnt::sBlock);
  pushPair(dnp::Block::sTimePeriod, el.getTPeriod(), formatnum::DEC);
  pushMembershipInfo((Node&)el); 
  pushSingle(ec::Node::Block::sLookDef);
  pushPaintedEyecandy((Node&)el);
  pushStopEyecandy((Node&)el);
  pushEnd();
}

void VisitorVertexWriter::visit(const TimingMsg& el) const {
  pushNodeInfo((Node&)el);
  pushPair(dnp::Base::sType, dnt::sTMsg);
  pushEventInfo((Event&)el);  
  
  // ID output depends on FID field
  uint64_t id = el.getId();
  uint8_t fid = ((id >> ID_FID_POS) & ID_FID_MSK); 
  if (fid >= idFormats.size()) throw std::runtime_error("bad format id (FID) within ID field of Node '" + el.getName() + "'");
  //ouput ID subfields
  vPf& vF = idFormats[fid];
  for(auto& it : vF) { pushPair(it.s, ((id >> it.pos) &  ((1 << it.bits ) - 1) ), formatnum::DEC); }
  pushPair(dnp::TMsg::sId, el.getId(), formatnum::HEX64);
  pushPair(dnp::TMsg::sPar, el.getPar(), formatnum::HEX64);
  pushPair(dnp::TMsg::sTef, el.getTef(), formatnum::DEC);
  pushSingle(ec::Node::TMsg::sLookDef);
  pushPaintedEyecandy((Node&)el);
  pushStartEyecandy((Node&)el);
  pushEnd();
}

void VisitorVertexWriter::visit(const Noop& el) const {
  pushNodeInfo((Node&)el); 
  pushPair(dnp::Base::sType, dnt::sCmdNoop);
  pushEventInfo((Event&)el);
  pushCommandInfo((Command&) el);
  pushPair(dnp::Cmd::sQty, el.getQty(), formatnum::DEC);
  pushSingle(ec::Node::Cmd::sLookDef);
  //pushSingle(ec::Node::Cmd::sLookNoop);
  pushPaintedEyecandy((Node&)el);
  pushStartEyecandy((Node&)el);
  pushEnd();
}

void VisitorVertexWriter::visit(const Flow& el) const  { 
  pushNodeInfo((Node&)el);
  pushPair(dnp::Base::sType, dnt::sCmdFlow);
  pushCommandInfo((Command&) el);
  pushEventInfo((Event&)el);
  pushPair(dnp::Cmd::sQty, el.getQty(), formatnum::DEC);
  pushSingle(ec::Node::Cmd::sLookDef);
  //pushSingle(ec::Node::Cmd::sLookFlow);
  pushPaintedEyecandy((Node&)el);
  pushStartEyecandy((Node&)el);
  pushEnd();
}

void VisitorVertexWriter::visit(const Flush& el) const { 
  pushNodeInfo((Node&)el);
  pushPair(dnp::Base::sType, dnt::sCmdFlush);
  pushEventInfo((Event&)el);
  pushCommandInfo((Command&) el);
  pushPair(dnp::Cmd::sPrio,  el.getPrio(), formatnum::DEC);
  pushSingle(ec::Node::Cmd::sLookDef);
  //pushSingle(ec::Node::Cmd::sLookFlush);
  pushPaintedEyecandy((Node&)el);
  pushStartEyecandy((Node&)el);
  pushEnd();
}

void VisitorVertexWriter::visit(const Wait& el) const {
  pushNodeInfo((Node&)el);
  pushPair(dnp::Base::sType, dnt::sCmdWait);
  pushEventInfo((Event&)el);
  pushCommandInfo((Command&) el);
  pushSingle(ec::Node::Cmd::sLookDef);
  //pushSingle(ec::Node::Cmd::sLookWait);
  pushPaintedEyecandy((Node&)el);
  pushStartEyecandy((Node&)el);
  pushEnd();
}

void VisitorVertexWriter::visit(const CmdQMeta& el) const {
  pushNodeInfo((Node&)el);
  pushPair(dnp::Base::sType, dnt::sQInfo);
  pushSingle(ec::Node::Meta::sLookDef);
  pushEnd();
}

void VisitorVertexWriter::visit(const CmdQBuffer& el) const {
  pushNodeInfo((Node&)el);
  pushPair(dnp::Base::sType, dnt::sQBuf);
  pushSingle(ec::Node::Meta::sLookDef);
  pushEnd();
}

void VisitorVertexWriter::visit(const DestList& el) const {
  pushNodeInfo((Node&)el);
  pushPair(dnp::Base::sType, dnt::sDstList);
  pushSingle(ec::Node::Meta::sLookDef);
  pushEnd();
}

