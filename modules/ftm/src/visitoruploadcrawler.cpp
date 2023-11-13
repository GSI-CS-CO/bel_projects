#include "visitoruploadcrawler.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"
#include "dotstr.h"
#include "validation.h"
#include <boost/range/combine.hpp>

namespace dnp = DotStr::Node::Prop;
namespace dnt = DotStr::Node::TypeVal;
namespace dnmg = DotStr::Node::MetaGen;
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

  vertex_vec_t vs = getChildrenByEdgeType(vStart, edgeType);
  if (vs.size() == 0) return null_vertex;
  if (vs.size() == 1) return *vs.begin();
  if (vs.size()  > 1) throw std::runtime_error( exIntro + "Node " + g[vStart].name + "has more than one child of that edge type, result is ambiguous\n");
}

//get the adress of a dst node as perceived from a given src node (considers differing RAMs)
  uint32_t VisitorUploadCrawler::getEdgeTargetAdr(vertex_t vSrc, vertex_t vDst) const {
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

    const vertex_t vc = getOnlyChildByEdgeType(v, det::sDefDst);
    const uint32_t a = getEdgeTargetAdr(v, vc);

    ret.insert(
      { NODE_DEF_DEST_PTR, a}
    );
    return ret;
  }



  mVal VisitorUploadCrawler::getRefLinks() const {
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    mVal t;
    /*
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
    */
    return t;
  }



  mVal VisitorUploadCrawler::getValLinks() const {
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    mVal t;
    /*
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
    */
    return t;
  }    


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
          if (ret.find(TMSG_ID_LO) == ret.end()) {sErr << "Found more than one dynamic id source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            uint32_t aId = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_ID_SMSK);
            ret.insert({TMSG_ID_LO, aId });
            
            hashXor ^= x->hash;
          }
        }
        if (g[*out_cur].type == det::sDynPar1) {
          if (ret.find(TMSG_PAR_HI) == ret.end()){sErr << "Found more than one dynamic par1 source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            uint32_t aPar1 = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_PAR1_SMSK);
            ret.insert({TMSG_PAR_HI, aPar1 });
            hashXor ^= x->hash;
          }
        }
        if (g[*out_cur].type == det::sDynPar0) {
          if (ret.find(TMSG_PAR_LO) == ret.end()) {sErr << "Found more than one dynamic par0 source" << std::endl; break;
          } else {
            auto x = at.lookupVertex(target(*out_cur,g));
            uint32_t aPar0 = at.adrConv(AdrType::MGMT, AdrType::EXT, x->cpu, x->adr); g[v].np->setFlags(NFLG_TMSG_DYN_PAR0_SMSK);
            ret.insert({TMSG_PAR_LO, aPar0 });
            hashXor ^= x->hash;
          }
        }
        
        if (g[*out_cur].type == det::sDynRes) {
          if (ret.find(TMSG_RES) == ret.end()) {sErr << "Found more than one dynamic res source" << std::endl; break;
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

  mVal ret;
  //search for cmd target, returns NULL PTR if there is no target
  ret.insert({CMD_TARGET, getEdgeTargetAdr(v, getOnlyChildByEdgeType(v, det::sCmdTarget)) });
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

  //this will return exactly one target, otherwise neighbourhood check would have detected the misshapen schedule
  vertex_vec_t vsTgt = getChildrenByEdgeType(v, det::sCmdTarget);
  //command cross over to other CPUs is okay. Find out what Cpu the command target is on
  auto tgt = at.lookupVertex(*vsTgt.begin());

  vertex_vec_t vsDst = getChildrenByEdgeType(v, det::sCmdFlowDst);
  if((vsTgt.size() == 0) || (vsDst.size() == 0)) { ret.insert({(unsigned)CMD_FLOW_DEST, (unsigned)LM32_NULL_PTR}); return ret;}// if this command is not connected, return a null pointer as flowdst

  auto dst = at.lookupVertex(*vsDst.begin());

  if (dst->cpu != tgt->cpu) throw std::runtime_error(  exIntro + "Target " + g[*vsTgt.begin()].name + "'s CPU must not differ from Dst " + g[*vsDst.begin()].name + "'s CPU\n");
  ret.insert({(unsigned)CMD_FLOW_DEST, (unsigned)at.adrConv(AdrType::MGMT, AdrType::INT , dst->cpu, dst->adr)});

  return ret;
}

mVal VisitorUploadCrawler::getSwitchDst() const {
  mVal ret;

  //this will return exactly one target, otherwise neighbourhood check would have detected the misshapen schedule
  vertex_vec_t vsTgt = getChildrenByEdgeType(v, det::sSwitchTarget);
  //command cross over to other CPUs is okay. Find out what Cpu the command target is on
  auto tgt = at.lookupVertex(*vsTgt.begin());

  vertex_vec_t vsDst = getChildrenByEdgeType(v, det::sSwitchDst);
  if((vsTgt.size() == 0) || (vsDst.size() == 0)) { ret.insert({SWITCH_DEST, LM32_NULL_PTR}); return ret;}// if this command is not connected, return a null pointer as flowdst

  auto dst = at.lookupVertex(*vsDst.begin());

  if (dst->cpu != tgt->cpu) throw std::runtime_error(  exIntro + "Target " + g[*vsTgt.begin()].name + "'s CPU must not differ from Dst " + g[*vsDst.begin()].name + "'s CPU\n");
  ret.insert({(unsigned)SWITCH_DEST, (unsigned)at.adrConv(AdrType::MGMT, AdrType::INT , dst->cpu, dst->adr)});

  return ret;
}

mVal VisitorUploadCrawler::getFlushOvr() const {
  mVal ret;

  //this will return exactly one target, otherwise neighbourhood check would have detected the misshapen schedule
  vertex_vec_t vsTgt = getChildrenByEdgeType(v, det::sCmdTarget);
  //command cross over to other CPUs is okay. Find out what Cpu the command target is on
  auto tgt = at.lookupVertex(*vsTgt.begin());

  vertex_vec_t vsDst = getChildrenByEdgeType(v, det::sCmdFlushOvr);
  if((vsTgt.size() == 0) || (vsDst.size() == 0)) { ret.insert({CMD_FLUSH_DEST_OVR, LM32_NULL_PTR}); return ret;}// if this command is not connected, return a null pointer as flowdst

  auto dst = at.lookupVertex(*vsDst.begin());

  if (dst->cpu != tgt->cpu) throw std::runtime_error(  exIntro + "Target " + g[*vsTgt.begin()].name + "'s CPU must not differ from Dst " + g[*vsDst.begin()].name + "'s CPU\n");
  ret.insert({(unsigned)CMD_FLUSH_DEST_OVR, (unsigned)at.adrConv(AdrType::MGMT, AdrType::INT , dst->cpu, dst->adr)});

  return ret;
}

mVal VisitorUploadCrawler::getListDst() const {
  mVal ret;
  
  /* To increase the number of altdst connected to a block beyond the capacity of a single dstLst,
     the list of all altdst is to be split and stored into a linked list (LL) of dstList nodes.
     Now the problem for dst LL - we dont have global knowlege, our perspective is local to THIS dstlist node,
     which sits somewhere in the LL.
     
     We must gather the destination list of the original block and then find out which slice of that list must be
     stored in THIS dstList node by finding THIS node's position in the LL. Then we insert the adresses
     corresponding to nodes in the slice into THIS node's return address map.

    defDst     altDst  altDst
       A          A       A
       |__ _______|_______|...
          |
          |          hops                   child  
          |-------|-------|-------|            |
        Block -> dstL -> dstL -> THIS_NODE -> dstL -x
                           |                       |  
                         parent                   end
 
  
  things we can find out from the graph nodes and edges connected to THIS node
   - parent/ancestor node type
   - this node's position in the LL (number of hops until ancestor is the original block)
   - altdst edges outgoing from original block
   - defdst edge outgoing from original block
   - edge outgoing from this node. if there is a child, this is the LL nextPtr, else it is the LL end
  */

  /*--- find origin block and parent node (can be the same). Count number of hops necessary to reach original block ---*/
  vertex_t va, vp;// ancestor (the iterator) and parent
  unsigned countHops;
  bool unknownAncestor = true; // we better check if we successfully found the ancestor block

  // we need to iterate the ancestor backwards over the LL. Once were at the beginning, va will point to the original block.
  va = v;   //Set va to v (this node) to begin traversal. 
  for(countHops = 1; countHops < (Validation::MaxOccurrance::DST + DST_MAX -1) / DST_MAX; ++countHops) { // there can be as many hops as dstList nodes needed to fit MaxOccurrance::DST
    Graph::in_edge_iterator in_begin, in_end;
    boost::tie(in_begin, in_end) = in_edges(va,g);
    va = source(*in_begin,g); // Update va after each hop until va's node type equals block
    
    if (countHops == 1) vp = va; // save first degree ancestor as parent.

    if (g[va].np == nullptr) throw std::runtime_error( exIntro + "Node " + g[va].name + " of type " + g[va].type + " has not data object\n");
    if (g[va].np->isBlock()) { // if the checked ancestor is a block, we're done.
      unknownAncestor = false;
      break;
    }
  }
  if (unknownAncestor) throw std::runtime_error(  exIntro + "DstList " + g[v].name + "is an orphan!\n");
  sLog << "dstLL: ancestor " << g[va].name << " " << g[va].type << ". <- " << countHops << " -> thisNode " << g[v].name << " " << g[v].type << ", this parent is " << g[vp].name << " " << g[vp].type;

  
  /*--- Get us the vector of all altDst nodes ---*/
  vertex_vec_t altVec = getChildrenByEdgeType(va, det::sAltDst); //get all known altdst nodes
  vertex_t       vDef = getOnlyChildByEdgeType(va, det::sDefDst);//search for defdst node
  //add the defdst node if existent
  if(vDef != null_vertex) {
    //there is only "dst children" in terms of pointers. We need a way to know edges were alt and which one was def when downloading. Place def at begin so we can find out by order.
    //defdst must be listed in altVec as element zero. Push copy of beginning to back of vector, overwrite first element with def.
    altVec.push_back(altVec[0]); 
    altVec[0] = vDef;
  }

  //find the slice of altVec for this dstLst node
  unsigned slice_begin, slice_end;
  slice_begin = (countHops-1) * DST_MAX;
  slice_end   = std::min((unsigned)altVec.size(), slice_begin + DST_MAX); // handle the end of altVec / slice not being full

  //insert into this node's adress map. Keys are word offsets 0..DST_MAX-1, values are altVec elements in this slice
  unsigned offs = DST_ARRAY; // offset for first slice element is zero
  for(unsigned i = slice_begin; i < slice_end; i++) {
    ret.insert({offs, getEdgeTargetAdr(v, altVec[i])});
    offs += _PTR_SIZE_;
  }

  //insert LL ptr to next dstLst node into adress map. inserts LM32_NULL_PTR if there is none.
  ret.insert({(unsigned)DST_NXTPTR, getEdgeTargetAdr(v, getOnlyChildByEdgeType(v, det::sDstList))});
  
  
  return ret;

}
