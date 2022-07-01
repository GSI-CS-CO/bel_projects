#include "visitordownloadcrawler.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"
#include "dotstr.h"

namespace dnp = DotStr::Node::Prop;
namespace dnt = DotStr::Node::TypeVal;
namespace dep = DotStr::Edge::Prop;
namespace det = DotStr::Edge::TypeVal;


const std::string VisitorDownloadCrawler::exIntro = "VisitorDownloadCrawler: ";


void VisitorDownloadCrawler::setDefDst() const {

  uint32_t auxAdr;
  uint32_t tmpAdr;

  auxAdr = writeBeBytesToLeNumber<uint32_t>(b + NODE_DEF_DEST_PTR);
  tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, auxAdr);

  if (tmpAdr == LM32_NULL_PTR) return;
  // try to lookup address and create edge
  try {
    auto x = at.lookupAdr(cpu, tmpAdr);
    boost::add_edge(v, x->v, myEdge(det::sDefDst), g);
  } catch(...) {
    // it is possible that a covenant will cause carpeDM to intentionally leave an orphaned def dst to avoid contesting the DM's future changes
    // So if there is a covenant registered for the node we're just processing, ignore the exception. Otherwise rethrow
    if (!ct.isOk(ct.lookup(g[v].name))) { throw; }
    else {sLog << "setDefDst: Node <" << g[v].name << "> has an invalid def dst, ignoring because of active covenant" << std::endl;}
  }

}

bool VisitorDownloadCrawler::setAltDsts(const uint32_t defAdr)  const { 
  unsigned hops = 0;
  vertex_t v_parent = v, v_child, v_Block = v;
  bool defaultListed = false;
  
    // iterate over dstList LL
  while(hops <= ((altDstMax + dstListCapacity -1) / dstListCapacity) ) { // max number of hops must be less than max depth of dstLinked List
    Graph::out_edge_iterator out_begin, out_end;
    boost::tie(out_begin, out_end) = out_edges(v_parent,g);
    if( (out_begin == out_end) || (g[*out_begin].type != det::sDstList) ){break;} // no edges or bad edge type found
    v_child = target(*out_begin,g);

    //add all found altDst edges to parent Block
    for (ptrdiff_t offs = DST_ARRAY; offs < DST_ARRAY_END; offs += _32b_SIZE_) {
      uint32_t tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>( ((AllocMeta*)&(*(at.lookupVertex(v_child))))->b + offs ));
  
      if (tmpAdr != LM32_NULL_PTR) { //if the address is not null, add anything altDst edge
        try {
          auto x = at.lookupAdr(cpu, tmpAdr);
          if (tmpAdr != defAdr) { boost::add_edge(v_Block, x->v, (myEdge){det::sAltDst}, g); } //if altDst address it matches defdest address, don't add again
          else                  {defaultListed = true;}
        } catch (...) {
          if (!ct.isOk(ct.lookup(g[v_Block].name)) || (tmpAdr != defAdr)) { throw; }
          else {sLog << "visitDstList: Node <" << g[v_Block].name << "> has an invalid def dst, ignoring because of active covenant" << std::endl;}
  
        }
      } 
    }
    
    v_parent = v_child;
    hops++; // count the LL hops
  }
 
  return defaultListed;

}

void VisitorDownloadCrawler::visit(const Block& el) const {
  Graph::in_edge_iterator in_begin, in_end;
  uint32_t tmpAdr;
  uint32_t defAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>((uint8_t*)(b + NODE_DEF_DEST_PTR)));
  bool defaultListed;

  try {
    
    //if the block has a altDst list, process it and set altDsts
    tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_ALT_DEST_PTR ));
    if (tmpAdr != LM32_NULL_PTR) { 
      boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sDstList), g); 
      defaultListed = setAltDsts(defAdr);
    } else {
      defaultListed = true;
    }

    // set defdst
    if (defAdr != LM32_NULL_PTR) {
        std::string sType = defaultListed ? det::sDefDst : det::sBadDefDst;
      try {
        auto x = at.lookupAdr(cpu, defAdr);
        boost::add_edge(v, x->v, (myEdge){sType}, g);
      } catch(...) {
        boost::add_edge(v, v, (myEdge){det::sBadDefDst}, g);
      }
    } 

    tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_IL_PTR ));
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sQPrio[PRIO_IL]), g);
    tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_HI_PTR ));
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sQPrio[PRIO_HI]), g);
    tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_LO_PTR ));
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sQPrio[PRIO_LO]), g);
  
  } catch (std::runtime_error const& err) {
   std::cerr << "Failed to create Block <" << g[v].name << " edges: " << err.what() << std::endl;
  } 
}

void VisitorDownloadCrawler::visit(const TimingMsg& el) const  {
  uint32_t flags = g[v].np->getFlags();
  uint32_t tmpAdr;

  setDefDst();

  if (flags & NFLG_TMSG_DYN_ID_SMSK) {
    tmpAdr = at.adrConv(AdrType::EXT, AdrType::MGMT,cpu, (uint32_t)writeBeBytesToLeNumber<uint64_t>(b + TMSG_ID ));
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sDynId),          g);
  }
  if (flags & NFLG_TMSG_DYN_PAR0_SMSK) {
    tmpAdr = at.adrConv(AdrType::EXT, AdrType::MGMT,cpu, (uint32_t)writeBeBytesToLeNumber<uint32_t>(b + TMSG_PAR_LO));
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sDynPar0),          g);
  }
  if (flags & NFLG_TMSG_DYN_PAR1_SMSK) {
    tmpAdr = at.adrConv(AdrType::EXT, AdrType::MGMT,cpu, (uint32_t)writeBeBytesToLeNumber<uint32_t>(b + TMSG_PAR_HI));
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sDynPar1),          g);
  }
}

std::pair<uint8_t, AdrType> VisitorDownloadCrawler::createSwitch(const Switch& el) const {
  uint8_t targetCpu;
  AdrType adrT;
  uint32_t tmpAdr, auxAdr;
  auxAdr = writeBeBytesToLeNumber<uint32_t>(b + SWITCH_TARGET );

  std::tie(targetCpu, adrT) = at.adrClassification(auxAdr);
  targetCpu = (adrT == AdrType::PEER ? targetCpu : cpu); // Internal address type does not know which cpu it belongs to

  setDefDst();
  tmpAdr = at.adrConv(adrT, AdrType::MGMT, targetCpu, auxAdr);
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(targetCpu, tmpAdr))))->v, myEdge(det::sSwitchTarget),    g);

  return std::make_pair(targetCpu, adrT);
}

void VisitorDownloadCrawler::visit(const Origin& el) const  {
  uint8_t targetCpu;
  AdrType adrT;
  uint32_t tmpAdr;

  uint32_t  auxAdr = writeBeBytesToLeNumber<uint32_t>(b + ORIGIN_DEST );

  std::tie(targetCpu, adrT) = at.adrClassification(auxAdr);
  targetCpu = (adrT == AdrType::PEER ? targetCpu : cpu); // Internal address type does not know which cpu it belongs to



  setDefDst();
  tmpAdr = at.adrConv(adrT, AdrType::MGMT, targetCpu, auxAdr);
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(targetCpu, tmpAdr))))->v, myEdge(det::sOriginDst),    g);

}

void VisitorDownloadCrawler::visit(const StartThread& el) const  {
  setDefDst();
}

std::pair<uint8_t, AdrType> VisitorDownloadCrawler::createCmd(const Command& el) const {
  uint8_t targetCpu;
  AdrType adrT;
  uint32_t tmpAdr, auxAdr;
  auxAdr = writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET );

  std::tie(targetCpu, adrT) = at.adrClassification(auxAdr);
  targetCpu = (adrT == AdrType::PEER ? targetCpu : cpu); // Internal address type does not know which cpu it belongs to

  setDefDst();
  tmpAdr = at.adrConv(adrT, AdrType::MGMT, targetCpu, auxAdr);
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(targetCpu, tmpAdr))))->v, myEdge(det::sCmdTarget),    g);

  return std::make_pair(targetCpu, adrT);
}

void VisitorDownloadCrawler::visit(const Flow& el) const  {
  uint8_t targetCpu;
  AdrType adrT;
  uint32_t tmpAdr;

  std::tie(targetCpu, adrT) = createCmd((Command&)el);
  uint32_t rawAdr = writeBeBytesToLeNumber<uint32_t>(b + CMD_FLOW_DEST );


  tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT, targetCpu, rawAdr);


  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(targetCpu, tmpAdr))))->v, myEdge(det::sCmdFlowDst),   g);

}

void VisitorDownloadCrawler::visit(const Switch& el) const  {
  uint8_t targetCpu;
  AdrType adrT;
  uint32_t tmpAdr;

  std::tie(targetCpu, adrT) = createSwitch(el);
  uint32_t rawAdr = writeBeBytesToLeNumber<uint32_t>(b + SWITCH_DEST );


  tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT, targetCpu, rawAdr);


  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(targetCpu, tmpAdr))))->v, myEdge(det::sSwitchDst),   g);

}

void VisitorDownloadCrawler::visit(const Flush& el) const {
  uint8_t targetCpu;
  AdrType adrT;
  uint32_t tmpAdr;

  std::tie(targetCpu, adrT) = createCmd((Command&)el);
  uint32_t rawAdr = writeBeBytesToLeNumber<uint32_t>(b + CMD_FLUSH_DEST_OVR );


  tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT, targetCpu, rawAdr);


  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(targetCpu, tmpAdr))))->v, myEdge(det::sCmdFlushOvr),   g);
}

void VisitorDownloadCrawler::visit(const Noop& el) const {
  createCmd((Command&)el);
}

void VisitorDownloadCrawler::visit(const Wait& el) const {
  createCmd((Command&)el);
}

void VisitorDownloadCrawler::visit(const CmdQMeta& el) const {
  uint32_t tmpAdr;
  for (ptrdiff_t offs = CMDQ_BUF_ARRAY; offs < CMDQ_BUF_ARRAY_END; offs += _32b_SIZE_) {
    tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>(b + offs ));
    if (tmpAdr != LM32_NULL_PTR) {
      auto x = at.lookupAdr(cpu, tmpAdr);
      boost::add_edge(v, x->v, (myEdge){det::sMeta}, g);
    }
  }
}

void VisitorDownloadCrawler::visit(const CmdQBuffer& el) const {
}
/*
void VisitorDownloadCrawler::visit(const DestList& el) const {
  vertex_t vPblock;
  Graph::in_edge_iterator in_begin, in_end;
  uint32_t tmpAdr, defAdr;

  //sLog << "Trying to find parent of " << g[v].name << std::endl;
  boost::tie(in_begin, in_end) = in_edges(v,g);
  if(in_begin != in_end) {

    //get the parent Block. there is only one, neighbourhood validation function made sure of this
    vPblock = source(*in_begin,g);

    //add all destination (including default destination (defDstPtr might have changed during runtime) connections from the dest list to the parent block

    bool defaultValid = false;
    std::string sType = det::sAltDst;

    defAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>((uint8_t*)(at.lookupVertex(vPblock))->b + NODE_DEF_DEST_PTR));

    for (ptrdiff_t offs = DST_ARRAY; offs < DST_ARRAY_END; offs += _32b_SIZE_) {
      tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>(b + offs ));

      //if tmpAdr it is the default address, change edge type to det::sDefDst (defdst)
      if (tmpAdr == defAdr) { sType = det::sDefDst; defaultValid = true;}
      else sType = det::sAltDst;

      if (tmpAdr != LM32_NULL_PTR) {
        try {
          auto x = at.lookupAdr(cpu, tmpAdr);
          boost::add_edge(vPblock, x->v, (myEdge){sType}, g);
        } catch (...) {
          if (!ct.isOk(ct.lookup(g[vPblock].name))) { throw; }
          else {sLog << "visitDstList: Node <" << g[vPblock].name << "> has an invalid def dst, ignoring because of active covenant" << std::endl;}

        }
      }
    }
    if (!defaultValid) { //default destination was not in alt dest list. that shouldnt happen ... draw it in
      sErr << "!!! DefDest Adr " << std::hex << "0x" << defAdr << " not in AltDestList. Means someone set an arbitrary pointer for DefDest !!!" << std::endl;
      if (defAdr != LM32_NULL_PTR) {
        try {
          auto x = at.lookupAdr(cpu, defAdr);
          boost::add_edge(vPblock, x->v, (myEdge){det::sBadDefDst}, g);
        } catch(...) {
          boost::add_edge(vPblock, vPblock, (myEdge){det::sBadDefDst}, g);
        }
      }
    }
  } else {
    throw std::runtime_error( exIntro + "Node " + g[v].name + " of type " + g[v].type + " must have a parent\n");
  }

}
*/

void VisitorDownloadCrawler::visit(const DestList& el) const {
  //create edge to own possible child
  uint32_t auxAdr = writeBeBytesToLeNumber<uint32_t>(b + NODE_DEF_DEST_PTR);
  uint32_t tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT, cpu, auxAdr);

  if (tmpAdr != LM32_NULL_PTR) {
    auto x = at.lookupAdr(cpu, tmpAdr);
    boost::add_edge(v, x->v, myEdge(det::sDstList), g);

  }

}




