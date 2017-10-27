#include "visitorvertexwriter.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"

namespace ec  = DotStr::EyeCandy;
namespace dgp = DotStr::Graph::Prop;
namespace dnp = DotStr::Node::Prop;
namespace dnt = DotStr::Node::TypeVal;
namespace dep = DotStr::Edge::Prop;
namespace det = DotStr::Edge::TypeVal;
namespace ssi = DotStr::Node::Prop::TMsg::SubId;

void VisitorVertexWriter::pushPair(const std::string& p, int v, bool hex) const {
  out << ", " << p << "=\"";
  if (hex)  out << std::hex;
  else      out << std::dec;
  out << v << "\"";
}

void VisitorVertexWriter::pushPair(const std::string& p, const std::string& v) const {
  out << ", " << p << "=\"" << v << "\"";
}

void VisitorVertexWriter::pushSingle(const std::string& p) const  {
  out << ", " << p;
}


void VisitorVertexWriter::pushNodeInfo(const Node& el) const {
  pushStart();
  //can't use our helper for first property, as we cannot have a comma
  out << dnp::Base::sCpu   << "=\"" <<  (int)el.getCpu() << "\"";
  pushPair(dnp::Base::sFlags, el.getFlags(), F_HEX);
}

void VisitorVertexWriter::pushEventInfo(const Event& el) const {
  pushPair(dnp::TMsg::sTimeOffs, el.getTOffs(), F_DEC);
}

void VisitorVertexWriter::pushCommandInfo(const Command& el) const {
  pushPair(dnp::Cmd::sTimeValid, el.getTValid(), F_DEC);
}

void VisitorVertexWriter::pushPaintedInfo(const Node& el) const {
  pushSingle((el.isPainted() ? ec::Node::Base::sLookPaint0 : ec::Node::Base::sLookPaintNone));
}


void VisitorVertexWriter::visit(const Block& el) const  {
  pushNodeInfo((Node&)el); 
  pushPair(dnp::Base::sType, dnt::sBlock);
  pushPair(dnp::Block::sTimePeriod, el.getTPeriod(), F_DEC);
  pushSingle(ec::Node::Block::sLookDef);
  pushEnd();
}

void VisitorVertexWriter::visit(const TimingMsg& el) const {
  pushNodeInfo((Node&)el);
  pushPair(dnp::Base::sType, dnt::sTMsg);
  pushEventInfo((Event&)el);  
  
  uint64_t id = el.getId();

  pushPair(ssi::sFid, ((id >> ID_FID_POS)   & ID_FID_MSK), F_DEC);
  //pushSingle(formatId(id));
  pushPair(dnp::TMsg::sPar, el.getPar(), F_HEX);
  pushPair(dnp::TMsg::sTef, el.getTef(), F_DEC);
  //pushPair(dnp::sTMsg::sRes, << el.getRes(), F_DEC);
  pushSingle(ec::Node::TMsg::sLookDef);
  pushPaintedInfo((Node&)el);
  pushEnd();
}

void VisitorVertexWriter::visit(const Noop& el) const {
  pushNodeInfo((Node&)el); 
  pushPair(dnp::Base::sType, dnt::sCmdNoop);
  pushEventInfo((Event&)el);
  pushCommandInfo((Command&) el);
  pushPair(dnp::Cmd::sQty, el.getQty(), F_DEC);
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
  pushPair(dnp::Cmd::sQty, el.getQty(), F_DEC);
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
  pushPair(dnp::Cmd::sPrio,  el.getPrio(), F_DEC);
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

