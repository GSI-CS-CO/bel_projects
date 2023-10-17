#include "visitoruploadcrawler.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"
#include "dotstr.h"
#include <boost/range/combine.hpp>

namespace dnp = DotStr::Node::Prop;
namespace dnt = DotStr::Node::TypeVal;
namespace dep = DotStr::Edge::Prop;
namespace det = DotStr::Edge::TypeVal;

const std::string VisitorUploadCrawler::exIntro = "VisitorUploadCrawler: ";

//FIXME Dear future self, the code duplication in here is appalling. Create some proper helper functions for crying out loud !

void VisitorUploadCrawler::visit(const Block& el) const {
  el.serialise(getDefDst() + getQInfo(), b);
}

void VisitorUploadCrawler::visit(const TimingMsg& el) const  {
  el.serialise(getDefDst() + getDynSrc(), b);
}

void VisitorUploadCrawler::visit(const Flow& el) const  {
  el.serialise( getDefDst() + getCmdTarget((Command&)el) + getFlowDst(), b);
}

void VisitorUploadCrawler::visit(const Switch& el) const {
  el.serialise( getDefDst() + getSwitchTarget() + getSwitchDst(), b);
}

void VisitorUploadCrawler::visit(const Flush& el) const {
  el.serialise( getDefDst() + getCmdTarget((Command&)el) + getFlushOvr(), b);
}

void VisitorUploadCrawler::visit(const Noop& el) const {
  el.serialise( getDefDst() + getCmdTarget((Command&)el), b);
}

void VisitorUploadCrawler::visit(const Wait& el) const {
  el.serialise( getDefDst() + getCmdTarget((Command&)el), b);
}

void VisitorUploadCrawler::visit(const CmdQMeta& el) const {
  el.serialise( getDefDst() + getQBuf(), b);
}

void VisitorUploadCrawler::visit(const CmdQBuffer& el) const {
  el.serialise(getDefDst(), b);
}

void VisitorUploadCrawler::visit(const DestList& el) const {
  el.serialise(getListDst(), b);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private helper functions

vertex_set_t VisitorUploadCrawler::getChildrenByEdgeType(vertex_t vStart, const std::string edgeType) const {
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  vertex_set_t ret;
  boost::tie(out_begin, out_end) = out_edges(vStart,g);

  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {
    if (g[target(*out_cur,g)].np == nullptr) throw std::runtime_error( exIntro + "Node " + g[target(*out_cur,g)].name + " of type " + g[target(*out_cur,g)].type + " has not data object\n");
    if (g[*out_cur].type == edgeType) { ret.insert(target(*out_cur, g));}
  }

  return ret;
}

vertex_t getOnlyChildByEdgeType(vertex_t vStart, const std::string edgeType) const {

  vertex_set_t vs = getChildrenByEdgeType(vStart, edgeType);
  if vs.size() == 0 return null_vertex;
  if vs.size() == 1 return *vs.begin();
  if vs.size()  > 1 throw std::runtime_error( exIntro + "Node " + g[vStart].name + "has more than one child of that edge type, result is ambiguous\n");
}

//get the adress of a dst node as perceived from a given src node (considers differing RAMs)
  const uint32_t getEdgeTargetAdr(vertex_t vSrc, vertex_t vDst, const std::string& exMsg = "") {
    uint32_t ret = LM32_NULL_PTR;
    if (vSrc != null_vertex && vDst != null_vertex) {
      auto src = at.lookupVertex(vSrc);
      auto dst = at.lookupVertex(vDst);
      ret = at.adrConv(AdrType::MGMT, (dst->cpu == src->cpu ? AdrType::INT : AdrType::PEER), dst->cpu, dst->adr);
    }
    return ret;
  }  

//Use this if the order of vertices does not matter
vAdr& VisitorUploadCrawler::childrenAdrs(vertex_set_t vs, vAdr& ret, const unsigned int minResults, const unsigned int maxResults, const bool allowPeers, const uint32_t resultPadData) const {
  unsigned int results = ret.size();

  for (auto& itVs : vs ) {
    if (ret.size() >= results + maxResults) break;
    auto x = at.lookupVertex(itVs);
    if (!allowPeers && (x->cpu != cpu)) throw std::runtime_error( exIntro + "Child " + g[x->v].name + "'s CPU must not differ from parent " + g[v].name + "'s CPU\n");

    AdrType aTmp = (x->cpu == cpu ? AdrType::INT : AdrType::PEER);
    ret.push_back(at.adrConv(AdrType::MGMT, aTmp, x->cpu, x->adr));
  }
  for (unsigned int i = ret.size(); i < (results + minResults); i++ ) ret.push_back(resultPadData);

  return ret;
}


  mVal VisitorUploadCrawler::getDefDst() const {
    mVal ret;
    ret.insert({ NODE_DEF_DEST_PTR, getEdgeTargetAdr(v, getChildrenByEdgeType(v, det::sDefDst)[0]) });
    return ret;
  }

  vAdr VisitorUploadCrawler::getDynSrc() const {

    vAdr ret;
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(v,g);

    uint32_t aId   = LM32_NULL_PTR;
    uint32_t aPar0 = LM32_NULL_PTR;
    uint32_t aPar1 = LM32_NULL_PTR;
    uint32_t aTef  = LM32_NULL_PTR;
    uint32_t aRes  = LM32_NULL_PTR;
    uint32_t hashXor = 0;


    for (out_cur = out_begin; out_cur != out_end; ++out_cur)
    {
      if (g[target(*out_cur,g)].np == nullptr) sErr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
      else {

        if (g[*out_cur].type == det::sDynId) {
          if (aId != LM32_NULL_PTR) {sErr << "Found more than one dynamic id source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            aId = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_ID_SMSK);
            hashXor ^= x->hash;
          }
        }
        if (g[*out_cur].type == det::sDynPar1) {
          if (aPar1 != LM32_NULL_PTR) {sErr << "Found more than one dynamic par1 source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            aPar1 = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_PAR1_SMSK);
            //sLog << "DynAdr 1 0x" << std::hex << aPar1 << std::endl;
            hashXor ^= x->hash;
          }
        }
        if (g[*out_cur].type == det::sDynPar0) {
          if (aPar0 != LM32_NULL_PTR) {sErr << "Found more than one dynamic par0 source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            aPar0 = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_PAR0_SMSK);
            //sLog << "DynAdr 0 0x" << std::hex << aPar0 << std::endl;
            hashXor ^= x->hash;
          }
        }
        if (g[*out_cur].type == det::sDynTef) {
          if (aTef != LM32_NULL_PTR) {sErr << "Found more than one dynamic tef source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            aTef = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_TEF_SMSK);
            hashXor ^= x->hash;
          }
        }

        /*
        if (g[*out_cur].type == det::sDynRes) {
          if (aRes != LM32_NULL_PTR) {sErr << "Found more than one dynamic res source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            aRes = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_RES_SMSK);
          }
        }
        */
      }
    }

    // Use Res field for checksum. Exor the hashes of all dyndata children and add here
    aTef = hashXor;

    //FIXME this ought to be reserve + indexes, push_back order is too error prone

    ret.push_back(aId);
    ret.push_back(aPar1);
    ret.push_back(aPar0);
    ret.push_back(aTef);
    ret.push_back(aRes);

    return ret;
  }

  mVal VisitorUploadCrawler::getQInfo() const {
    mVal ret;

    //search for dst list
    ret.insert({BLOCK_ALT_DEST_PTR, getEdgeTargetAdr(v, getOnlyChildByEdgeType(v, det::sDstList)) });

    const uint32_t ptrOffs[3] = {BLOCK_CMDQ_LO_PTR, BLOCK_CMDQ_HI_PTR, BLOCK_CMDQ_IL_PTR};

    //search for q lists
    for (unsigned prio=PRIO_LO; prio <= PRIO_IL; prio++) {
      vertex_t vQ = getOnlyChildByEdgeType(v, det::sQPrio[prio]);
      ret.insert({ptrOffs[prio], getEdgeTargetAdr(v, vQ) });
      if (vQ != null_vertex) { g[v].np->setFlags(1 << (NFLG_BLOCK_QS_POS + prio)) }
    }

    return ret;
  }

//FIXME - How do we know the order of edges is the same through all ser/des operations? if it is not, then the rd/wr indices will be mismatched and can cause trouble
vAdr VisitorUploadCrawler::getQBuf() const {
  vAdr ret;

  vertex_set_t vsTmp = getChildrenByEdgeType(v, det::sMeta);
  unsigned bufCnt;
  for (auto it : vsTmp) { 
    if (g[it].type == dnt::sQBuf) vsTmp.erase(it); }
  childrenAdrs(vsTmp, ret, 2, 2);

  return ret;
}

 mVal  VisitorUploadCrawler::getCmdTarget(Command& el) const {

  mVal ret;
  //search for cmd target
  ret.insert({CMD_TARGET, getEdgeTargetAdr(v, getOnlyChildByEdgeType(v, det::sCmdTarget)) });
  return ret;
}

 mVal  VisitorUploadCrawler::getSwitchTarget() const {

  mVal  ret;
  //search for cmd target
  ret.insert({SWITCH_TARGET, getEdgeTargetAdr(v, getOnlyChildByEdgeType(v, det::sSwitchTarget)) });
  

  return ret;
}

//TODO cleanup the dst function redundancies

vAdr VisitorUploadCrawler::getFlowDst() const {
  vAdr ret;

  //this will return exactly one target, otherwise neighbourhood check would have detected the misshapen schedule
  vertex_set_t vsTgt = getChildrenByEdgeType(v, det::sCmdTarget);
  //command cross over to other CPUs is okay. Find out what Cpu the command target is on
  auto tgt = at.lookupVertex(*vsTgt.begin());

  vertex_set_t vsDst = getChildrenByEdgeType(v, det::sCmdFlowDst);
  if((vsTgt.size() == 0) || (vsDst.size() == 0)) { ret.push_back(LM32_NULL_PTR); return ret;}// if this command is not connected, return a null pointer as flowdst

  auto x = at.lookupVertex(*vsDst.begin());

  if (x->cpu != tgt->cpu) throw std::runtime_error(  exIntro + "Target " + g[*vsTgt.begin()].name + "'s CPU must not differ from Dst " + g[*vsDst.begin()].name + "'s CPU\n");
  ret.push_back(at.adrConv(AdrType::MGMT, AdrType::INT , x->cpu, x->adr));

  return ret;
}

vAdr VisitorUploadCrawler::getSwitchDst() const {
  vAdr ret;

  //this will return exactly one target, otherwise neighbourhood check would have detected the misshapen schedule
  vertex_set_t vsTgt = getChildrenByEdgeType(v, det::sSwitchTarget);
  //command cross over to other CPUs is okay. Find out what Cpu the command target is on
  auto tgt = at.lookupVertex(*vsTgt.begin());

  vertex_set_t vsDst = getChildrenByEdgeType(v, det::sSwitchDst);
  if((vsTgt.size() == 0) || (vsDst.size() == 0)) { ret.push_back(LM32_NULL_PTR); return ret;}// if this command is not connected, return a null pointer as flowdst

  auto x = at.lookupVertex(*vsDst.begin());

  if (x->cpu != tgt->cpu) throw std::runtime_error(  exIntro + "Target " + g[*vsTgt.begin()].name + "'s CPU must not differ from Dst " + g[*vsDst.begin()].name + "'s CPU\n");
  ret.push_back(at.adrConv(AdrType::MGMT, AdrType::INT , x->cpu, x->adr));

  return ret;
}

vAdr VisitorUploadCrawler::getFlushOvr() const {
  vAdr ret;

  //this will return exactly one target, otherwise neighbourhood check would have detected the misshapen schedule
  vertex_set_t vsTgt = getChildrenByEdgeType(v, det::sCmdTarget);
  //command cross over to other CPUs is okay. Find out what Cpu the command target is on
  auto tgt = at.lookupVertex(*vsTgt.begin());

  vertex_set_t vsDst = getChildrenByEdgeType(v, det::sCmdFlushOvr);
  if((vsTgt.size() == 0) || (vsDst.size() == 0)) { ret.push_back(LM32_NULL_PTR); return ret;}// if this command is not connected, return a null pointer as flowdst

  auto x = at.lookupVertex(*vsDst.begin());

  if (x->cpu != tgt->cpu) throw std::runtime_error(  exIntro + "Target " + g[*vsTgt.begin()].name + "'s CPU must not differ from Dst " + g[*vsDst.begin()].name + "'s CPU\n");
  ret.push_back(at.adrConv(AdrType::MGMT, AdrType::INT , x->cpu, x->adr));

  return ret;
}

vAdr VisitorUploadCrawler::getListDst() const {
  vAdr ret;
  Graph::in_edge_iterator in_begin, in_end;
  vertex_t vp;

  //get the parent. there shall be only one, a block (no check for that right now, sorry)
  boost::tie(in_begin, in_end) = in_edges(v,g);
  vp = source(*in_begin,g);

  //search for default destination
  childrenAdrs(getChildrenByEdgeType(vp, det::sDefDst), ret);

  //search parent blocks alternative destinations
  childrenAdrs(getChildrenByEdgeType(vp, det::sAltDst), ret, 0, 9);

  return ret;

}