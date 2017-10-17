#include "visitordownloadcrawler.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"



void VisitorDownloadCrawler::setDefDst() const {
  
  uint32_t auxAdr;
  uint32_t tmpAdr;

  auxAdr = writeBeBytesToLeNumber<uint32_t>(b + NODE_DEF_DEST_PTR);
  tmpAdr = at.intAdr2adr(cpu, auxAdr);
  auto x = at.lookupAdr(cpu, tmpAdr);
  if (tmpAdr == LM32_NULL_PTR) return;
  //std::cout << "cpu " << cpu << "InAdr: 0x" << std::hex << auxAdr << " Adr: 0x" << std::hex << tmpAdr <<  std::endl;
  if (!(at.isOk(x))) {
    std::cout << "AtDown Entry not found !" <<  std::endl;
  }  
  else  boost::add_edge(v, x->v, myEdge(sDD), g);

}

void VisitorDownloadCrawler::visit(const Block& el) const {
  Graph::in_edge_iterator in_begin, in_end;
  uint32_t tmpAdr;

  
  tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_ALT_DEST_PTR ));
  //if the block has no destination list, set default destination ourself
  if (tmpAdr != LM32_NULL_PTR) { boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(sDL), g); }
  else setDefDst();  

  tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_IL_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(sQM[PRIO_IL]), g);
  tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_HI_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(sQM[PRIO_HI]), g);
  tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_LO_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(sQM[PRIO_LO]), g);

}

void VisitorDownloadCrawler::visit(const TimingMsg& el) const  {
  uint32_t flags = g[v].np->getFlags();
  uint32_t tmpAdr;

  setDefDst();
  
  //std::cout << "TMSG Flags 0x" << std::hex << flags << std::endl;
  //FIXME do proper null ptr checking for EVERY possibility !!!
  //TODO include possibility to switch between intern / extern addresses as dynamic parameter

  if (flags & NFLG_TMSG_DYN_ID_SMSK) {
    tmpAdr = at.extAdr2adr(cpu, (uint32_t)writeBeBytesToLeNumber<uint64_t>(b + TMSG_ID ));
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(sDID),          g);
  }
  if (flags & NFLG_TMSG_DYN_PAR0_SMSK) {
    //std::cout << "original 0x" << std::hex << (uint32_t)writeBeBytesToLeNumber<uint64_t>(b + TMSG_PAR ) << " , intbase 0x" << m.intBaseAdr + extBaseAdr std::endl;
    tmpAdr = at.extAdr2adr(cpu, (uint32_t)writeBeBytesToLeNumber<uint64_t>(b + TMSG_PAR ));
   // std::cout << "found 0x" << std::hex << tmpAdr << std::endl;
    //parserMeta* lookupPtr = (parserMeta*)(m.lookupAdr(tmpAdr));
    //if (lookupPtr == NULL) std::cout << "Parsermeta Lookup returned Null " << std::endl;
    
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(sDPAR0),          g);
  }
  if (flags & NFLG_TMSG_DYN_PAR1_SMSK) {
    tmpAdr = at.extAdr2adr(cpu, (uint32_t)(writeBeBytesToLeNumber<uint64_t>(b + TMSG_PAR ) >> 32));
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(sDPAR1),          g);
  }
}

void VisitorDownloadCrawler::visit(const Flow& el) const  {
  uint32_t targetCpu = (el.getAct() >> ACT_TCPU_POS) & ACT_TCPU_MSK;
  uint32_t tmpAdr;  
  setDefDst();


  if( targetCpu != cpu) tmpAdr = at.peerAdr2adr(targetCpu, writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  else                  tmpAdr = at.intAdr2adr(targetCpu,  writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));


  


  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(sTG),          g);

  tmpAdr = at.intAdr2adr(targetCpu,  writeBeBytesToLeNumber<uint32_t>(b + CMD_FLOW_DEST ));
  //std::cout << g[v].name << "Caller " << (int)cpu << " callee " << (int)targetCpu << std::hex << " 0x" << writeBeBytesToLeNumber<uint32_t>(b + CMD_FLOW_DEST ) << std::hex << " 0x" << tmpAdr << std::endl;
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(sFD),          g);

}

void VisitorDownloadCrawler::visit(const Flush& el) const {
  uint32_t targetCpu = (el.getAct() >> ACT_TCPU_POS) & ACT_TCPU_MSK;
  uint32_t tmpAdr;
  setDefDst();



  if( targetCpu != cpu) tmpAdr = at.peerAdr2adr(targetCpu, writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  else                  tmpAdr = at.intAdr2adr(cpu,  writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(sTG),          g);

}

void VisitorDownloadCrawler::visit(const Noop& el) const {
  uint32_t targetCpu = (el.getAct() >> ACT_TCPU_POS) & ACT_TCPU_MSK;
  uint32_t tmpAdr;
  setDefDst();

  if( targetCpu != cpu) tmpAdr = at.peerAdr2adr(targetCpu, writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  else                  tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(sTG),          g);

}

void VisitorDownloadCrawler::visit(const Wait& el) const {
  uint32_t tmpAdr; 
  uint32_t targetCpu = (el.getAct() >> ACT_TCPU_POS) & ACT_TCPU_MSK;
  
  setDefDst();
  if( targetCpu != cpu) tmpAdr = at.peerAdr2adr(targetCpu, writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  else                  tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((AllocMeta*)&(*(at.lookupAdr(cpu, tmpAdr))))->v, myEdge(sTG),          g);

}

void VisitorDownloadCrawler::visit(const CmdQMeta& el) const {
  uint32_t tmpAdr;  
  for (ptrdiff_t offs = CMDQ_BUF_ARRAY; offs < CMDQ_BUF_ARRAY_END; offs += _32b_SIZE_) {
    tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + offs ));
    if (tmpAdr != LM32_NULL_PTR) {
      auto x = at.lookupAdr(cpu, tmpAdr);
      if (x != NULL) {//std::cout << "found qbuf!" << std::endl; 
        boost::add_edge(v, x->v, (myEdge){"meta"}, g);
      }
    }  
  }
}

void VisitorDownloadCrawler::visit(const CmdQBuffer& el) const {
}

void VisitorDownloadCrawler::visit(const DestList& el) const {
  vertex_t vPblock;
  Graph::in_edge_iterator in_begin, in_end;
  uint32_t tmpAdr, defAdr; 

   

  //std::cout << "Trying to find parent of " << g[v].name << std::endl;
  boost::tie(in_begin, in_end) = in_edges(v,g);
  if(in_begin != in_end) {

    //get the parent Block. there shall be only one, a block (no check for that right now, sorry)
    //std::cout << "Found parent: " << g[source(*in_begin,g)].name << std::endl;

    vPblock = source(*in_begin,g);

    //add all destination (including default destination (defDstPtr might have changed during runtime) connections from the dest list to the parent block

    bool defaultValid = false;
    std::string sType = sAD;
    defAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(g[vPblock].np->getB() + NODE_DEF_DEST_PTR));
    std::cerr << "def  " << std::hex << "0x" << defAdr << std::endl;   

    for (ptrdiff_t offs = DST_ARRAY; offs < DST_ARRAY_END; offs += _32b_SIZE_) {
      tmpAdr = at.intAdr2adr(cpu, writeBeBytesToLeNumber<uint32_t>(b + offs ));

      //if tmpAdr it is the default address, change edge type to sDD (defdst)
      std::cerr << "current alt " << std::hex << "0x" << tmpAdr << std::endl;
      if (tmpAdr == defAdr) { sType = sDD; defaultValid = true;}
      else sType = sAD;

      if (tmpAdr != LM32_NULL_PTR) {
        auto x = at.lookupAdr(cpu, tmpAdr);
        if (x != NULL) {
          boost::add_edge(vPblock, x->v, (myEdge){sType}, g);
        }
      }  
    }
    if (!defaultValid) { //default destination was not in alt dest list. that shouldnt happen ... draw it in
      std::cerr << "!!! DefDest not in AltDestList. Means someone set an arbitrary pointer for DefDest !!!" << std::endl;
      if (defAdr != LM32_NULL_PTR) {
        auto x = at.lookupAdr(cpu, defAdr);
        if (x != NULL) {
          boost::add_edge(vPblock, x->v, (myEdge){sBD}, g);
        } else boost::add_edge(vPblock, vPblock, (myEdge){sBD}, g);
      }
    }
  } else {
    std::cerr << "!!! No parent found !!!" << std::endl;
  }  

}





