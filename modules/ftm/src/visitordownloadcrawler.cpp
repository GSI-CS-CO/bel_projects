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
  tmpAdr = at.intAdr2adr(cpu, auxAdr);
  auto x = at.lookupAdr(cpu, tmpAdr);
  if (tmpAdr == LM32_NULL_PTR) return;
  //sLog << "cpu " << cpu << "InAdr: 0x" << std::hex << auxAdr << " Adr: 0x" << std::hex << tmpAdr <<  std::endl;
  if (!(at.isOk(x))) throw std::runtime_error( exIntro + "Node " + g[v].name + " of type " + g[v].type + " was found unallocated\n");
  boost::add_edge(v, x->v, myEdge(det::sDefDst), g);

}

void VisitorDownloadCrawler::visit(const Block& el) const {
  Graph::in_edge_iterator in_begin, in_end;
  uint32_t tmpAdr;

  
  tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_ALT_DEST_PTR ));
  //if the block has no destination list, set default destination ourself
  if (tmpAdr != LM32_NULL_PTR) { boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sDstList), g); }
  else setDefDst();  

  tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_IL_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sQPrio[PRIO_IL]), g);
  tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_HI_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sQPrio[PRIO_HI]), g);
  tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_LO_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sQPrio[PRIO_LO]), g);

}

void VisitorDownloadCrawler::visit(const TimingMsg& el) const  {
  uint32_t flags = g[v].np->getFlags();
  uint32_t tmpAdr;

  setDefDst();

  if (flags & NFLG_TMSG_DYN_ID_SMSK) {
    tmpAdr = at.extAdr2adr(cpu, (uint32_t)writeBeBytesToLeNumber<uint64_t>(b + TMSG_ID ));
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sDynId),          g);
  }
  if (flags & NFLG_TMSG_DYN_PAR0_SMSK) {
    tmpAdr = at.extAdr2adr(cpu, (uint32_t)writeBeBytesToLeNumber<uint64_t>(b + TMSG_PAR ));
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sDynPar0),          g);
  }
  if (flags & NFLG_TMSG_DYN_PAR1_SMSK) {
    tmpAdr = at.extAdr2adr(cpu, (uint32_t)(writeBeBytesToLeNumber<uint64_t>(b + TMSG_PAR ) >> 32));
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sDynPar1),          g);
  }
}

void VisitorDownloadCrawler::visit(const Flow& el) const  {
  uint32_t targetCpu = (el.getAct() >> ACT_TCPU_POS) & ACT_TCPU_MSK;
  uint32_t tmpAdr;  
  setDefDst();


  if( targetCpu != cpu) tmpAdr = at.peerAdr2adr(targetCpu, writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  else                  tmpAdr = at.intAdr2adr(targetCpu,  writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));

  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sCmdTarget),          g);

  tmpAdr = at.intAdr2adr(targetCpu,  writeBeBytesToLeNumber<uint32_t>(b + CMD_FLOW_DEST ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sCmdFlowDst),          g);

}

void VisitorDownloadCrawler::visit(const Flush& el) const {
  uint32_t targetCpu = (el.getAct() >> ACT_TCPU_POS) & ACT_TCPU_MSK;
  uint32_t tmpAdr;
  setDefDst();
  
  if( targetCpu != cpu) tmpAdr = at.peerAdr2adr(targetCpu, writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  else                  tmpAdr = at.intAdr2adr(cpu,  writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sCmdTarget),          g);

}

void VisitorDownloadCrawler::visit(const Noop& el) const {
  uint32_t targetCpu = (el.getAct() >> ACT_TCPU_POS) & ACT_TCPU_MSK;
  uint32_t tmpAdr;
  setDefDst();

  if( targetCpu != cpu) tmpAdr = at.peerAdr2adr(targetCpu, writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  else                  tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sCmdTarget),          g);

}

void VisitorDownloadCrawler::visit(const Wait& el) const {
  uint32_t tmpAdr; 
  uint32_t targetCpu = (el.getAct() >> ACT_TCPU_POS) & ACT_TCPU_MSK;
  
  setDefDst();
  if( targetCpu != cpu) tmpAdr = at.peerAdr2adr(targetCpu, writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  else                  tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(det::sCmdTarget),          g);

}

void VisitorDownloadCrawler::visit(const CmdQMeta& el) const {
  uint32_t tmpAdr;  
  for (ptrdiff_t offs = CMDQ_BUF_ARRAY; offs < CMDQ_BUF_ARRAY_END; offs += _32b_SIZE_) {
    tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + offs ));
    if (tmpAdr != LM32_NULL_PTR) {
      auto x = at.lookupAdr(cpu, tmpAdr);
      if (!(at.isOk(x))) throw std::runtime_error( exIntro + "Node " + g[v].name + " of type " + g[v].type + " was found unallocated\n");
      boost::add_edge(v, x->v, (myEdge){det::sMeta}, g);
    }  
  }
}

void VisitorDownloadCrawler::visit(const CmdQBuffer& el) const {
}

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
    defAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(g[vPblock].np->getB() + NODE_DEF_DEST_PTR));

    for (ptrdiff_t offs = DST_ARRAY; offs < DST_ARRAY_END; offs += _32b_SIZE_) {
      tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + offs ));

      //if tmpAdr it is the default address, change edge type to det::sDefDst (defdst)
      if (tmpAdr == defAdr) { sType = det::sDefDst; defaultValid = true;}
      else sType = det::sAltDst;

      if (tmpAdr != LM32_NULL_PTR) {
        auto x = at.lookupAdr(cpu, tmpAdr);
        if (at.isOk(x)) {
          boost::add_edge(vPblock, x->v, (myEdge){sType}, g);
        }
      }  
    }
    if (!defaultValid) { //default destination was not in alt dest list. that shouldnt happen ... draw it in
      sErr << "!!! DefDest not in AltDestList. Means someone set an arbitrary pointer for DefDest !!!" << std::endl;
      if (defAdr != LM32_NULL_PTR) {
        auto x = at.lookupAdr(cpu, defAdr);
        if (at.isOk(x)) {
          boost::add_edge(vPblock, x->v, (myEdge){det::sBadDefDst}, g);
        } else boost::add_edge(vPblock, vPblock, (myEdge){det::sBadDefDst}, g);
      }
    }
  } else {
    throw std::runtime_error( exIntro + "Node " + g[v].name + " of type " + g[v].type + " must have a parent\n");
  }  

}






