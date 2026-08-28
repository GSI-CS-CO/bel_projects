#include "visitoruploadcrawler.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "global.h"
#include "event.h"
#include "dotstr.h"
#include "validation.h"
#include <boost/range/combine.hpp>
#include "log.h"

namespace dnp = DotStr::Node::Prop;
namespace dnt = DotStr::Node::TypeVal;
namespace dnmg = DotStr::Node::MetaGen;
namespace dep = DotStr::Edge::Prop;
namespace det = DotStr::Edge::TypeVal;

const std::string VisitorUploadCrawler::exIntro = "VisitorUploadCrawler: ";

//FIXME Dear future self, the code duplication in here is appalling. Create some proper helper functions for crying out loud !

void VisitorUploadCrawler::visit(const Block& el) const {
  el.serialise(getDefDst() + getQInfo() + getRefLinks(), b);
}

void VisitorUploadCrawler::visit(const TimingMsg& el) const  {
  el.serialise(getDefDst() + getRefLinks() + getDynSrc(), b);
}

void VisitorUploadCrawler::visit(const Flow& el) const  {
  el.serialise( getDefDst() + getCmdTarget((Command&)el) + getFlowDst(), b);
}

void VisitorUploadCrawler::visit(const Switch& el) const {
  el.serialise( getDefDst() + getSwitchTarget() + getSwitchDst(), b);
}

void VisitorUploadCrawler::visit(const Origin& el) const {
  el.serialise( getDefDst() + getOriginDst(), b);
}

void VisitorUploadCrawler::visit(const StartThread& el) const {
  el.serialise( getDefDst(), b);
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

void VisitorUploadCrawler::visit(const Global& el) const {
  mVal emptyM;

  el.serialise(emptyM, b);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private helper functions

vertex_vec_t VisitorUploadCrawler::getChildrenByEdgeType(vertex_t vStart, const std::string edgeType) const {
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  vertex_vec_t ret;
  boost::tie(out_begin, out_end) = out_edges(vStart,g);

  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {
    if (g[target(*out_cur,g)].np == nullptr) throw std::runtime_error( exIntro + "Node " + g[target(*out_cur,g)].name + " of type " + g[target(*out_cur,g)].type + " has not data object\n");
    if (g[*out_cur].type == edgeType) { ret.push_back(target(*out_cur, g));}
  }

  return ret;
}
/*
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
*/
 vertex_t VisitorUploadCrawler::getOnlyChildByEdgeType(vertex_t vStart, const std::string edgeType) const {
  vertex_t ret = null_vertex;

  vertex_vec_t vs = getChildrenByEdgeType(vStart, edgeType);
  if (vs.size()  > 1) {
    throw std::runtime_error( exIntro + "Node '" + g[vStart].name + "' has more than one child of edge type '" + edgeType + "', result is ambiguous\n");
  }
  //if (vs.size() == 0) ret = null_vertex;
  if (vs.size() == 1) ret = *vs.begin();

  return ret;
}

//get the adress of a dst node as perceived from a given src node (considers differing RAMs)
  uint32_t VisitorUploadCrawler::getEdgeTargetAdr(vertex_t vSrc, vertex_t vDst) const {
    uint32_t ret = LM32_NULL_PTR;
    if (vSrc != null_vertex && vDst != null_vertex) {
      auto src = at.lookupVertex(vSrc);
      auto dst = at.lookupVertex(vDst);
      log<DEBUG_LVL2>(L"edgeTargetAdr %1% -> %2%, dst adr b4 conv %3$#08x") % g[src->v].name.c_str() % g[dst->v].name.c_str() % dst->adr;
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

    const vertex_t vc = getOnlyChildByEdgeType(v, det::sDefDst);
    const uint32_t a = getEdgeTargetAdr(v, vc);

  ret.insert( { NODE_DEF_DEST_PTR, a} );
  return ret;
}

mVal VisitorUploadCrawler::getRefLinks() const {
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  mVal t;
  uint32_t dynOps = 0;

  log<DEBUG_LVL0>(L"Entered Reflink handling for Node %1%") % g[v].name.c_str();

  boost::tie(out_begin, out_end) = out_edges(v,g);

  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {


    if ((g[*out_cur].type == det::sAdr) || (g[*out_cur].type == det::sRef) || (g[*out_cur].type == det::sRef2)) {
      log<DEBUG_LVL2>(L"Found Reflink to TargetNode %1%. Offset Source: %3% Offset Target: %2% Width: %4%") % g[target(*out_cur,g)].name.c_str() % g[*out_cur].fhead.c_str() % g[*out_cur].ftail.c_str() % g[*out_cur].bwidth.c_str();
      uint32_t oTarget  = s2u<uint32_t>(g[*out_cur].fhead);
      uint32_t oSource  = s2u<uint32_t>(g[*out_cur].ftail);
      uint32_t width    = s2u<uint32_t>(g[*out_cur].bwidth) == 64 ? 1 : 0;
      

      unsigned bIdx = oSource / 4 * 3; //idx of descriptor
      uint32_t dynMode = DYN_MODE_REF2;
      if (g[*out_cur].type == det::sRef)      { dynMode = DYN_MODE_REF; }
      else { if (g[*out_cur].type == det::sAdr) dynMode = DYN_MODE_ADR; }

      uint32_t bDesc = ((width & DYN_WIDTH64_MSK) << DYN_WIDTH64_POS) | ((dynMode & DYN_MODE_MSK) << DYN_MODE_POS); //descriptor bits for this ref
      dynOps |= bDesc << bIdx; // add to dynops
      log<DEBUG_LVL2>(L"Descriptor: idx %1% bshift %2% dynOps Slice %3$#02x shifted slice %4$#08x") % (oSource>>2) % bIdx % bDesc % (bDesc << bIdx);

      //we create a map entry, adress offset to adress, that will contain our refPtr
      //key is offset oSource (e.g. TMSG_RES)
      //value is the address of the target node + offset oTarget
      
      if (oTarget % 4 || oSource % 4) {throw std::runtime_error( exIntro + "Reflink Offsets are not 4B aligned: Offset Source " + std::to_string((unsigned)oSource) + " Offset Target " + std::to_string((unsigned)oTarget) + "\n");} // check if adress is 32b aligned. if not, throw ex
      uint32_t tmpAdr = getEdgeTargetAdr(v, target(*out_cur, g));
      log<DEBUG_LVL2>(L"Inserting: @Offset Source: %1$#02x edge target Adr %2$#08x + Offset Target %3$#02x = %4$#08x") % oSource % tmpAdr % oTarget % (tmpAdr + oTarget);
      t.insert({oSource, tmpAdr + oTarget});
    }
  }
  
  t.insert({NODE_OPT_DYN, dynOps});
  log<DEBUG_LVL2>(L"Passing Dynops to Offs %1$#08x Value: %2$#08x") % NODE_OPT_DYN % dynOps;
  return t;
}


/*
  mVal VisitorUploadCrawler::getValLinks() const {
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    mVal t;
    
    boost::tie(out_begin, out_end) = out_edges(v,g);

    for (out_cur = out_begin; out_cur != out_end; ++out_cur)
    {
      if (g[*out_cur].type == edgeType) {
        uint32_t oTarget  = s2u(g[*out_cur].fHead);
        uint32_t oSource  = s2u(g[*out_cur].fTail);
        uint32_t width    = s2u(g[*out_cur].bWidth) == 64 ? 1 : 0;
        //we create a map entry, adress offset to adress, that will contain our refPtr
        //key is offset oSource (e.g. TMSG_RES)
        //value is the address of the target node + offset oTarget
        t.insert(oSource, getEdgeTargetAdr(v, target(*out_cur, g)) );
        if (oTarget / )
        t.insert(NODE_OPT_DYN, )
      }
    }
    
    return t;
  }    
*/

  mVal VisitorUploadCrawler::getDynSrc() const {

    mVal ret;
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(v,g);

    uint32_t hashXor = 0;


    for (out_cur = out_begin; out_cur != out_end; ++out_cur)
    {
      if (g[target(*out_cur,g)].np == nullptr) sErr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
      else {

        if (g[*out_cur].type == det::sDynId) {
          if (ret.find(TMSG_ID_LO) != ret.end()) {sErr << "Found more than one dynamic id source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            uint32_t aId = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_ID_SMSK);
            ret.insert({TMSG_ID_LO, aId });

            hashXor ^= x->hash;
          }
        }
        if (g[*out_cur].type == det::sDynPar1) {
          if (ret.find(TMSG_PAR_HI) != ret.end()){sErr << "Found more than one dynamic par1 source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            uint32_t aPar1 = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_PAR1_SMSK);
            ret.insert({TMSG_PAR_HI, aPar1 });
            hashXor ^= x->hash;
          }
        }
        if (g[*out_cur].type == det::sDynPar0) {
          if (ret.find(TMSG_PAR_LO) != ret.end()) {sErr << "Found more than one dynamic par0 source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            uint32_t aPar0 = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_PAR0_SMSK);
            ret.insert({TMSG_PAR_LO, aPar0 });
            hashXor ^= x->hash;
          }
        }

        if (g[*out_cur].type == det::sDynRes) {
          if (ret.find(TMSG_RES) != ret.end()) {sErr << "Found more than one dynamic res source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            uint32_t aRes = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_RES_SMSK);
            ret.insert({TMSG_RES, aRes});
            hashXor ^= x->hash;
          }
        }

      }
    }

    // Use Res field for checksum. Exor the hashes of all dyndata children and add here if hashXor was modified
    if (hashXor) ret.insert({TMSG_TEF, hashXor });

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
      if (vQ != null_vertex) { g[v].np->setFlags(1 << (NFLG_BLOCK_QS_POS + prio)); }
    }

    return ret;
  }




mVal VisitorUploadCrawler::getQBuf() const {
  mVal ret;

  vertex_vec_t vsTmp = getChildrenByEdgeType(v, det::sMeta);

  for (auto it : vsTmp) {
    if (g[it].type == dnt::sQBuf) {
      //check name suffix 0 and 1. We must place the buffer node addresses in correct order so they alway match rd/wr indices
      //Dunno if it has a practical effect, we should not write back command buffers anyway but zero them IMO
      //However, it is the clean way to keep the order of buffers.

      if (hasEnding(g[it].name, dnmg::s1stQBufSuffix)) { ret.insert({0*_32b_SIZE_, getEdgeTargetAdr(v, it) }); }
      if (hasEnding(g[it].name, dnmg::s2ndQBufSuffix)) { ret.insert({1*_32b_SIZE_, getEdgeTargetAdr(v, it) }); }
    }
  }
  return ret;
}

 mVal  VisitorUploadCrawler::getCmdTarget(Command& el) const {
  log<DEBUG_LVL0>(L"cmdTarget: Entering");
  mVal ret;
  //search for cmd target, returns NULL PTR if there is no target
  ret.insert({CMD_TARGET, getEdgeTargetAdr(v, getOnlyChildByEdgeType(v, det::sCmdTarget)) });

  log<DEBUG_LVL1>(L"cmdTarget: Done. Target Adr = %1$#08x") % (unsigned)getEdgeTargetAdr(v, getOnlyChildByEdgeType(v, det::sCmdTarget));
  return ret;
}

mVal  VisitorUploadCrawler::getSwitchTarget() const {

  mVal  ret;
  //search for switch target, returns NULL PTR if there is no target
  ret.insert({SWITCH_TARGET, getEdgeTargetAdr(v, getOnlyChildByEdgeType(v, det::sSwitchTarget)) });


  return ret;
}

//TODO cleanup the dst function redundancies

mVal VisitorUploadCrawler::getFlowDst() const {
  mVal ret;
  log<DEBUG_LVL0>(L"flowDst: Entering");
  //this will return exactly one target, otherwise neighbourhood check would have detected the misshapen schedule
  vertex_vec_t vsTgt = getChildrenByEdgeType(v, det::sCmdTarget);
  //command cross over to other CPUs is okay. Find out what Cpu the command target is on


  vertex_vec_t vsDst = getChildrenByEdgeType(v, det::sCmdFlowDst);
  if((vsTgt.size() == 0) || (vsDst.size() == 0)) { ret.insert({(unsigned)CMD_FLOW_DEST, (unsigned)LM32_NULL_PTR}); log<DEBUG_LVL0>(L"flowDst: Done, unconnected"); return ret;}// if this command is not connected, return a null pointer as flowdst

  auto tgt = at.lookupVertex(*vsTgt.begin());
  auto dst = at.lookupVertex(*vsDst.begin());

  if (dst->cpu != tgt->cpu) throw std::runtime_error(  exIntro + "Target " + g[*vsTgt.begin()].name + "'s CPU must not differ from Dst " + g[*vsDst.begin()].name + "'s CPU\n");
  ret.insert({(unsigned)CMD_FLOW_DEST, (unsigned)at.adrConv(AdrType::MGMT, AdrType::INT , dst->cpu, dst->adr)});
  log<DEBUG_LVL1>(L"flowDst: Done. Dst Adr = %1$#08x") % (unsigned)at.adrConv(AdrType::MGMT, AdrType::INT , dst->cpu, dst->adr);
  return ret;
}

mVal VisitorUploadCrawler::getSwitchDst() const {
  mVal ret;

  //this will return exactly one target, otherwise neighbourhood check would have detected the misshapen schedule
  vertex_vec_t vsTgt = getChildrenByEdgeType(v, det::sSwitchTarget);
  //command cross over to other CPUs is okay. Find out what Cpu the command target is on

  vertex_vec_t vsDst = getChildrenByEdgeType(v, det::sSwitchDst);
  if((vsTgt.size() == 0) || (vsDst.size() == 0)) { ret.insert({SWITCH_DEST, LM32_NULL_PTR}); return ret;}// if this command is not connected, return a null pointer as flowdst

  auto tgt = at.lookupVertex(*vsTgt.begin());
  auto dst = at.lookupVertex(*vsDst.begin());

  if (dst->cpu != tgt->cpu) throw std::runtime_error(  exIntro + "Target " + g[*vsTgt.begin()].name + "'s CPU must not differ from Dst " + g[*vsDst.begin()].name + "'s CPU\n");
  ret.insert({(unsigned)SWITCH_DEST, (unsigned)at.adrConv(AdrType::MGMT, AdrType::INT , dst->cpu, dst->adr)});

  return ret;
}

mVal VisitorUploadCrawler::getOriginDst() const {
  mVal ret;

  vertex_vec_t vsDst = getChildrenByEdgeType(v, det::sOriginDst);
  //TODO: does it matter if the CPU is the one of v or can it be arbitrary in case of null?
  if(vsDst.size() == 0) {
    ret.insert({(unsigned)ORIGIN_DEST, LM32_NULL_PTR});
    ret.insert({(unsigned)ORIGIN_CPU , (unsigned)0});
    return ret;
  }// if this command is not connected, return a null pointer as Origindst

  auto dst = at.lookupVertex(*vsDst.begin());

  ret.insert({(unsigned)ORIGIN_DEST, (unsigned)at.adrConv(AdrType::MGMT, AdrType::INT , dst->cpu, dst->adr)});
  ret.insert({(unsigned)ORIGIN_CPU , (unsigned)dst->cpu});

  return ret;
}

mVal VisitorUploadCrawler::getFlushOvr() const {
  mVal ret;

  //this will return exactly one target, otherwise neighbourhood check would have detected the misshapen schedule
  vertex_vec_t vsTgt = getChildrenByEdgeType(v, det::sCmdTarget);
  //command cross over to other CPUs is okay. Find out what Cpu the command target is on

  vertex_vec_t vsDst = getChildrenByEdgeType(v, det::sCmdFlushOvr);
  if((vsTgt.size() == 0) || (vsDst.size() == 0)) { ret.insert({CMD_FLUSH_DEST_OVR, LM32_NULL_PTR}); return ret;}// if this command is not connected, return a null pointer as flowdst

  auto tgt = at.lookupVertex(*vsTgt.begin());
  auto dst = at.lookupVertex(*vsDst.begin());

  if (dst->cpu != tgt->cpu) throw std::runtime_error(  exIntro + "Target " + g[*vsTgt.begin()].name + "'s CPU must not differ from Dst " + g[*vsDst.begin()].name + "'s CPU\n");
  ret.insert({(unsigned)CMD_FLUSH_DEST_OVR, (unsigned)at.adrConv(AdrType::MGMT, AdrType::INT , dst->cpu, dst->adr)});

  return ret;
}

mVal VisitorUploadCrawler::getListDst() const {
  mVal ret;

  // scratch all that. We'll turn it around and create scores of destlists, all pointing to a block as their acnestor (or child? whats better, logically?)
  // this means single pass/independent reconstroction of altdsts - each dstlst by its own. process all, and you get all the altdsts back.

  /*--- find origin block and parent node (can be the same). Count number of hops necessary to reach original block ---*/
  log<DEBUG_LVL0>(L"dstLst: Entering");
  vertex_t va;

  Graph::out_edge_iterator out_begin, out_end;
  boost::tie(out_begin, out_end) = out_edges(v,g);
  va = target(*out_begin,g);
  if (g[va].np == nullptr) throw std::runtime_error( exIntro + "Node " + g[va].name + " of type " + g[va].type + " has not data object\n");
  if (!g[va].np->isBlock()) { throw std::runtime_error(  exIntro + "DstList " + g[v].name + "is childless!\n");}


  log<DEBUG_LVL0>(L"dstLst: childblock <%1%>/<%2%> <---- <%3%>/<%4%>(thisListnode)")  % g[va].name.c_str() % g[va].type.c_str() % g[v].name.c_str() % g[v].type.c_str();

  /*--- Get us the vector of all altDst nodes ---*/
  vertex_vec_t altVec = getChildrenByEdgeType(va, det::sAltDst); //get all known altdst nodes
  vertex_t       vDef = getOnlyChildByEdgeType(va, det::sDefDst);//search for defdst node
  //add the defdst node if existent
  if(vDef != null_vertex) {
    //there is only "dst children" in terms of pointers. We need a way to know edges were alt and which one was def when downloading. Place def at begin so we can find out by order.
    //defdst must be listed in altVec as element zero. Push copy of beginning to back of vector, overwrite first element with def.
    if(altVec.size() > 0)   {altVec.push_back(altVec[0]); altVec[0] = vDef;}
    else                    {altVec.push_back(vDef);}
  }
  //find the slice of altVec for this dstLst node
  //we do it he lazy way and look at the numerical suffix of this node's name

  //who am I? get the number from name suffix
  unsigned idx = s2u<unsigned>(g[v].name.substr(g[v].name.find_last_not_of("0123456789") + 1));
  unsigned slice_begin, slice_end;
  slice_begin = idx * DST_MAX;
  slice_end   = std::min((unsigned)altVec.size(), slice_begin + DST_MAX); // handle the end of altVec / slice not being full
  //insert into this node's adress map. Keys are word offsets 0..DST_MAX-1, values are altVec elements in this slice
  unsigned offs = DST_ARRAY; // offset for first slice element is zero
  for(unsigned i = slice_begin; i < slice_end; i++) {
    ret.insert({offs, getEdgeTargetAdr(v, altVec[i])});
    offs += _PTR_SIZE_;
  }
  log<DEBUG_LVL0>(L"dstLst: done");
  //insert pointer to our parent block as this node's defdst (lazy, but it kinda is, even if the firmware does not traverse it)
  ret.insert({(unsigned)DST_NXTPTR, getEdgeTargetAdr(v, va)});

  return ret;

}
