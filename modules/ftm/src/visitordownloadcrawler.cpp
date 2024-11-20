#include "visitordownloadcrawler.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "global.h"
#include "event.h"
#include "dotstr.h"

#include "log.h"

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
    else {log<WARNING>(L"setDefDst: Node %1% has an invalid def dst, ignoring because of active covenant")  % g[v].name.c_str();}
  }

}

void VisitorDownloadCrawler::visit(const Block& el) const {
  Graph::in_edge_iterator in_begin, in_end;
  uint32_t tmpAdr;

  try {
    
  

  tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_ALT_DEST_PTR ));
  //if the block has no destination list, set default destination ourself
  if (tmpAdr != LM32_NULL_PTR) { boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sDstList), g); }
  else setDefDst();

  tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_IL_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sQPrio[PRIO_IL]), g);
  tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_HI_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sQPrio[PRIO_HI]), g);
  tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_LO_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sQPrio[PRIO_LO]), g);
  
  } catch (std::runtime_error const& err) {
    log<ERROR>(L"visitBlock: Failed to create Block %1% edges: %2%") % g[v].name.c_str() % err.what();
  }

  setRefLinks();
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

  setRefLinks();
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

void VisitorDownloadCrawler::visit(const DestList& el) const {
  vertex_t vPblock;
  Graph::out_edge_iterator out_begin, out_end;
  uint32_t tmpAdr, defAdr;

  setDefDst();

  //sLog << "Trying to find parent of " << g[v].name << std::endl;
  boost::tie(out_begin, out_end) = out_edges(v,g);
  if(out_begin != out_end) {
    unsigned idx = s2u<unsigned>(g[v].name.substr(g[v].name.find_last_not_of("0123456789") + 1));
    //get the parent Block. there is only one, neighbourhood validation function made sure of this
    vPblock = target(*out_begin,g);
    log<DEBUG_LVL0>(L"Download dstLst: childblock <%1%>/<%2%> <---- <%3%>/<%4%>(thisListnode) at idx %5%")  % g[vPblock].name.c_str() % g[vPblock].type.c_str() % g[v].name.c_str() % g[v].type.c_str() % idx;

    //add all destination (including default destination (defDstPtr might have changed during runtime) connections from the dest list to the parent block

    bool defaultValid = false;
    std::string sType = det::sAltDst;

    //find out if there alreay is a defdst from parentblock to somewhere else
    //getting defdst from parent block for comparison
    defAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>((uint8_t*)(at.lookupVertex(vPblock))->b + NODE_DEF_DEST_PTR));
    
    //create edges from addresses
    for (ptrdiff_t offs = DST_ARRAY; offs < DST_ARRAY_END; offs += _32b_SIZE_) {
      tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT,cpu, writeBeBytesToLeNumber<uint32_t>(b + offs ));
      if ((tmpAdr == defAdr) && (idx=0)) defaultValid=true;

      //we draw altdst edges, but skip if the adr is the same as the defdst. we never draw a defdst, the block will do that. 
      if ((tmpAdr != LM32_NULL_PTR) && (tmpAdr != defAdr)) {
        try {
          auto x = at.lookupAdr(cpu, tmpAdr);
          boost::add_edge(vPblock, x->v, (myEdge){sType}, g);
        } catch (...) {
          if (!ct.isOk(ct.lookup(g[vPblock].name))) { throw; }
          else {
            log<ERROR>(L"visitDestList: Node %1% has an invalid def dst, ignoring because of active covenant") % g[vPblock].name.c_str();
          }

        }
      }
    }
    if (!defaultValid == (idx=0)) { //default destination was not in alt dest list. that shouldnt happen ... draw it in
      log<ERROR>(L"visitDestList: Adr %1$#x not in AltDestList. Someone set an arbitrary pointer for a DefDest, following this edge might crash the DM") % defAdr;
      if (defAdr != LM32_NULL_PTR) {
        log<DEBUG_LVL0>(L"Download dstLst: defAdr %1$#x will be drawn in")  % defAdr;
        try {
          auto x = at.lookupAdr(cpu, defAdr);
          boost::add_edge(vPblock, x->v, (myEdge){det::sBadDefDst}, g);
        } catch(...) {
          boost::add_edge(vPblock, vPblock, (myEdge){det::sBadDefDst}, g);
        }
      }
    }
  } else {
    throw std::runtime_error( exIntro + "Node " + g[v].name + " of type " + g[v].type + " cannot be childless\n");
  }
}

void VisitorDownloadCrawler::setRefLinks() const {
  log<DEBUG_LVL0>(L"Entered Reflink recreation for Node %1%") % g[v].name.c_str();

  uint32_t dynOps = writeBeBytesToLeNumber<uint32_t>(b + NODE_OPT_DYN);
  log<DEBUG_LVL2>(L"Total Dynops found %1$#08x") % dynOps;
  for (uint8_t idx=0; idx < 9; idx++) {
    uint32_t ds = dynOps >> (idx * 3);
    unsigned mode = ds & DYN_MODE_SMSK;
    
    if(mode >= DYN_MODE_ADR) {

      uint32_t width    = (ds & DYN_WIDTH64_SMSK) ? 64 : 32;
      uint32_t oSource  = idx << 2;
      log<DEBUG_LVL2>(L"Descriptor: idx %1% dynOps Slice %2$#02x Osource %3$#08x") % idx % (ds & 0x7) % oSource;
      //This is an adress with offset to a word inside a node. To create an edge, we need the address of the node.
      //get the tmpAdr from the indexed 32b word
      uint32_t dbgAdr = writeBeBytesToLeNumber<uint32_t>(b + (idx << 2));
      log<DEBUG_LVL2>(L"Got Adr %1$#08x") % dbgAdr;
      uint32_t tmpAdr = at.adrConv(AdrType::INT, AdrType::MGMT, cpu, dbgAdr);
      log<DEBUG_LVL2>(L"Found tmpAdr %1$#08x at Source offset %2$#08x, Startoffs %3$#08x, diff %4$#08x") % tmpAdr % oSource % at.getStartOffs(cpu) % (tmpAdr - at.getStartOffs(cpu));
      //Subtract the beginning of the mem pool. The result modulo nodesize gives us the target word offset in Byte (we need it later)
      uint32_t oTarget;
      if((tmpAdr < at.getStartOffs(cpu)) || (tmpAdr > at.getEndOffs(cpu))) { // this is a global. 
        
        oTarget = tmpAdr - at.rl->getLocVal(at.rl->getLocName(tmpAdr));

      } else {
        oTarget = (tmpAdr - at.getStartOffs(cpu)) % _MEM_BLOCK_SIZE;
      }
      if (oTarget % 4) {throw std::runtime_error( exIntro + "Reflink Offset is not 4B aligned: Offset Target " + std::to_string((unsigned)oTarget) + "\n");} // check if adress is 32b aligned. if not, throw ex
      
      try {
        //Subtract the Target word offset from tmpAdr and we get the node adress
        uint32_t nodeAdr = tmpAdr - oTarget;
        log<DEBUG_LVL2>(L"Trying lookup for tmpAdr %1$#08x Split: node adr: %2$#08x Offset Target: %3$#08x") % tmpAdr % nodeAdr % oTarget;
        auto x = at.lookupAdr(cpu, nodeAdr);

        boost::add_edge(v, x->v, myEdge(det::sDyn[mode], std::to_string((unsigned)oTarget), std::to_string((unsigned)oSource), std::to_string((unsigned)width)), g);
        log<DEBUG_LVL2>(L"Found Reflink to TargetNode %1%. Offset Source: %2$#08x Offset Target: %3$#08x Width: %4%") % g[x->v].name.c_str() % oSource % oTarget % width;
      } catch(exception& err) {
        throw std::runtime_error( "Error when recreating a byReference/byValue edge from node binary <" + g[v].name + ">. Cause: " + err.what());
      }
    }
  }

}

void VisitorDownloadCrawler::visit(const Global& el) const {
}

