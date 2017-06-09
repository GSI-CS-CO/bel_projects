#include "visitordownloadcrawler.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"

void VisitorDownloadCrawler::setDefDst() const {
  
  uint32_t tmpAdr, auxAdr;
  Graph& g = mmu.getDownGraph();
  uint8_t* b = (uint8_t*)&g[v].np->getB();

  auxAdr = writeBeBytesToLeNumber<uint32_t>(b + NODE_DEF_DEST_PTR);
  tmpAdr = mmu.intAdr2adr(auxAdr);
  parserMeta* tmpParser = mmu.lookupAdr(tmpAdr);

  //std::cout << "InAdr: 0x" << std::hex << auxAdr << " Adr: 0x" << std::hex << tmpAdr <<  std::endl;
  if (tmpParser == NULL) {
    //std::cout << "Parser Entry not found !" <<  std::endl;
  }  
  else if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, tmpParser->v, myEdge(sDD), g);

}

void VisitorDownloadCrawler::visit(const Block& el) const {
    
  
  uint32_t tmpAdr;
  Graph& g = mmu.getDownGraph();
  uint8_t* b = (uint8_t*)&g[v].np->getB();
   Graph::in_edge_iterator in_begin, in_end;

  setDefDst();
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + BLOCK_ALT_DEST_PTR ));
  if (tmpAdr != LM32_NULL_PTR) { boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, myEdge(sDL),          g);
  //std::cout << "Node " << g[v].name << " has destlist " << g[((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v].name << std::endl; 
  /*
  for (boost::tie(in_begin, in_end) = in_edges(((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v,g); in_begin != in_end; ++in_begin)
{   
    std::cout << "Parent of " << g[((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v].name << " is " << g[source(*in_begin,g)].name << std::endl;
}
*/
  }
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_IL_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, myEdge(sQM[PRIO_IL]), g);
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_HI_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, myEdge(sQM[PRIO_HI]), g);
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_LO_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, myEdge(sQM[PRIO_LO]), g);

}

void VisitorDownloadCrawler::visit(const TimingMsg& el) const  {
  uint32_t tmpAdr;
  Graph& g = mmu.getDownGraph();
  uint8_t* b = (uint8_t*)&g[v].np->getB();

  setDefDst();
  uint32_t flags = g[v].np->getFlags();

  //std::cout << "TMSG Flags 0x" << std::hex << flags << std::endl;
  //FIXME do proper null ptr checking for EVERY possibility !!!
  //TODO include possibility to switch between intern / extern addresses as dynamic parameter

  if (flags & NFLG_TMSG_DYN_ID_SMSK) {
    tmpAdr = mmu.extAdr2adr((uint32_t)writeBeBytesToLeNumber<uint64_t>(b + TMSG_ID ));
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, myEdge(sDID),          g);
  }
  if (flags & NFLG_TMSG_DYN_PAR0_SMSK) {
    //std::cout << "original 0x" << std::hex << (uint32_t)writeBeBytesToLeNumber<uint64_t>(b + TMSG_PAR ) << " , intbase 0x" << mmu.intBaseAdr + extBaseAdr std::endl;
    tmpAdr = mmu.extAdr2adr((uint32_t)writeBeBytesToLeNumber<uint64_t>(b + TMSG_PAR ));
   // std::cout << "found 0x" << std::hex << tmpAdr << std::endl;
    //parserMeta* lookupPtr = (parserMeta*)(mmu.lookupAdr(tmpAdr));
    //if (lookupPtr == NULL) std::cout << "Parsermeta Lookup returned Null " << std::endl;
    
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, myEdge(sDPAR0),          g);
  }
  if (flags & NFLG_TMSG_DYN_PAR1_SMSK) {
    tmpAdr = mmu.extAdr2adr((uint32_t)(writeBeBytesToLeNumber<uint64_t>(b + TMSG_PAR ) >> 32));
    if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, myEdge(sDPAR1),          g);
  }
}

void VisitorDownloadCrawler::visit(const Flow& el) const  {
  
  uint32_t tmpAdr;
  Graph& g = mmu.getDownGraph();
  uint8_t* b = (uint8_t*)&g[v].np->getB();

  setDefDst();
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, myEdge(sTG),          g);
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + CMD_FLOW_DEST ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, myEdge(sFD),          g);

}

void VisitorDownloadCrawler::visit(const Flush& el) const {
  
  uint32_t tmpAdr;
  Graph& g = mmu.getDownGraph();
  uint8_t* b = (uint8_t*)&g[v].np->getB();

  setDefDst();
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, myEdge(sTG),          g);

}

void VisitorDownloadCrawler::visit(const Noop& el) const {
  
  uint32_t tmpAdr;
  Graph& g = mmu.getDownGraph();
  uint8_t* b = (uint8_t*)&g[v].np->getB();

  setDefDst();
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, myEdge(sTG),          g);

}

void VisitorDownloadCrawler::visit(const Wait& el) const {
  
  uint32_t tmpAdr;
  Graph& g = mmu.getDownGraph();
  uint8_t* b = (uint8_t*)&g[v].np->getB();

  setDefDst();
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, myEdge(sTG),          g);

}

void VisitorDownloadCrawler::visit(const CmdQMeta& el) const {
  
  uint32_t tmpAdr;
  Graph& g = mmu.getDownGraph();
  uint8_t* b = (uint8_t*)&g[v].np->getB();
  parserMeta* pMeta;

  for (ptrdiff_t offs = CMDQ_BUF_ARRAY; offs < CMDQ_BUF_ARRAY_END; offs += _32b_SIZE_) {
    tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + offs ));
    if (tmpAdr != LM32_NULL_PTR) {
      pMeta = ((parserMeta*)(mmu.lookupAdr(tmpAdr)));
      if (pMeta != NULL) {//std::cout << "found qbuf!" << std::endl; 
        boost::add_edge(v, pMeta->v, (myEdge){"meta"}, g);
      }
    }  
  }
}

void VisitorDownloadCrawler::visit(const CmdQBuffer& el) const {
}

void VisitorDownloadCrawler::visit(const DestList& el) const {
  vertex_t vPblock;
  uint32_t tmpAdr;
  Graph& g = mmu.getDownGraph();
  uint8_t* b = (uint8_t*)&g[v].np->getB();
  Graph::in_edge_iterator in_begin, in_end;
  parserMeta* pMeta;

  //std::cout << "Trying to find parent of " << g[v].name << std::endl;
  boost::tie(in_begin, in_end) = in_edges(v,g);
  if(in_begin != in_end) {

    //get the parent Block. there shall be only one, a block (no check for that right now, sorry)
    //std::cout << "Found parent: " << g[source(*in_begin,g)].name << std::endl;



    vPblock = source(*in_begin,g);

    //add all destination (including default destination (defDstPtr might have changed during runtime) connections from the dest list to the parent block
    for (ptrdiff_t offs = DST_ARRAY; offs < DST_ARRAY_END; offs += _32b_SIZE_) {
      tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + offs ));
      if (tmpAdr != LM32_NULL_PTR) {
        pMeta = ((parserMeta*)(mmu.lookupAdr(tmpAdr)));
        if (pMeta != NULL) {//std::cout << "found altdest!" << std::endl; 
          boost::add_edge(vPblock, pMeta->v, (myEdge){sAD}, g);
        }
      }  
    }
  } else {
    std::cerr << "!!! No parent found !!!" << std::endl;
  }  

}





