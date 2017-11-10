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

void VisitorVertexWriter::pushPair(const std::string& p, int v, int format) const {
  out << ", " << p << "=\"";
  switch (format) {
    case FORMAT_HEX   : out << std::hex << v; break;
    case FORMAT_DEC   : out << std::dec << v; break;
    case FORMAT_BIN   : out << std::dec << (v & 1); break;
    case FORMAT_BOOL  : out << ((v & 1) ? sTrue : sFalse); break;
    default           : out << std::hex << v;
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

  if (el.getPattern() != sUndefined) {
    pushPair(dnp::Base::sPatName, el.getPattern());
    pushPair(dnp::Base::sPatEntry, (int)el.isPatEntry(), FORMAT_BOOL);
    pushPair(dnp::Base::sPatExit, (int)el.isPatExit(), FORMAT_BOOL);
  }  
  if (el.getBeamProc() != sUndefined) {
    pushPair(dnp::Base::sBpName, el.getBeamProc());
    pushPair(dnp::Base::sBpEntry, (int)el.isBpEntry(), FORMAT_BOOL);
    pushPair(dnp::Base::sBpExit, (int)el.isBpExit(), FORMAT_BOOL);
  }
}  

void VisitorVertexWriter::pushNodeInfo(const Node& el) const {
  pushStart();
  //can't use our helper for first property, as we cannot have a comma
  out << dnp::Base::sCpu   << "=\"" <<  (int)el.getCpu() << "\"";
  pushPair(dnp::Base::sFlags, el.getFlags(), FORMAT_HEX);

}

void VisitorVertexWriter::pushEventInfo(const Event& el) const {
  pushPair(dnp::TMsg::sTimeOffs, el.getTOffs(), FORMAT_DEC);
  pushMembershipInfo((Node&)el); 

}

void VisitorVertexWriter::pushCommandInfo(const Command& el) const {
  pushPair(dnp::Cmd::sTimeValid, el.getTValid(), FORMAT_DEC);
}

void VisitorVertexWriter::pushPaintedInfo(const Node& el) const {
  pushSingle((el.isPainted() ? ec::Node::Base::sLookPaint0 : ec::Node::Base::sLookPaintNone));
}




void VisitorVertexWriter::visit(const Block& el) const  {
  pushNodeInfo((Node&)el); 
  pushPair(dnp::Base::sType, dnt::sBlock);
  pushPair(dnp::Block::sTimePeriod, el.getTPeriod(), FORMAT_DEC);
  pushMembershipInfo((Node&)el); 
  pushSingle(ec::Node::Block::sLookDef);
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
  vPf& vF = idFormats[fid];
  //std::cout << "Output Node " << el.getName();
  for(auto& it : vF) { pushPair(it.s, ((id >> it.pos) &  ((1 << it.bits ) - 1) ), FORMAT_DEC); 
    //std::cout << ", " << it.s << " = " << std::dec << ((id >> it.pos) &  ((1 << it.bits ) - 1) ); 
  }
  //std::cout << " ID = 0x" << std::hex << id << std::endl;
  pushPair(dnp::TMsg::sPar, el.getPar(), FORMAT_HEX);
  pushPair(dnp::TMsg::sTef, el.getTef(), FORMAT_DEC);
  pushSingle(ec::Node::TMsg::sLookDef);
  pushPaintedInfo((Node&)el);
  pushEnd();
}

void VisitorVertexWriter::visit(const Noop& el) const {
  pushNodeInfo((Node&)el); 
  pushPair(dnp::Base::sType, dnt::sCmdNoop);
  pushEventInfo((Event&)el);
  pushCommandInfo((Command&) el);
  pushPair(dnp::Cmd::sQty, el.getQty(), FORMAT_DEC);
  pushSingle(ec::Node::Cmd::sLookDef);
  //pushSingle(ec::Node::Cmd::sLookNoop);
  pushPaintedInfo((Node&)el);
  pushEnd();
}

void VisitorVertexWriter::visit(const Flow& el) const  { 
  pushNodeInfo((Node&)el);
  pushPair(dnp::Base::sType, dnt::sCmdFlow);
  pushCommandInfo((Command&) el);
  pushEventInfo((Event&)el);
  pushPair(dnp::Cmd::sQty, el.getQty(), FORMAT_DEC);
  pushSingle(ec::Node::Cmd::sLookDef);
  //pushSingle(ec::Node::Cmd::sLookFlow);
  pushPaintedInfo((Node&)el);
  pushEnd();
}

void VisitorVertexWriter::visit(const Flush& el) const { 
  pushNodeInfo((Node&)el);
  pushPair(dnp::Base::sType, dnt::sCmdFlush);
  pushEventInfo((Event&)el);
  pushCommandInfo((Command&) el);
  pushPair(dnp::Cmd::sPrio,  el.getPrio(), FORMAT_DEC);
  pushSingle(ec::Node::Cmd::sLookDef);
  //pushSingle(ec::Node::Cmd::sLookFlush);
  pushPaintedInfo((Node&)el);
  pushEnd();
}

void VisitorVertexWriter::visit(const Wait& el) const {
  pushNodeInfo((Node&)el);
  pushPair(dnp::Base::sType, dnt::sCmdWait);
  pushEventInfo((Event&)el);
  pushCommandInfo((Command&) el);
  pushSingle(ec::Node::Cmd::sLookDef);
  //pushSingle(ec::Node::Cmd::sLookWait);
  pushPaintedInfo((Node&)el);
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

