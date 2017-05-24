#include "visitor.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"

const std::string sNodeType[] = {"prioil", "priohi", "priolo"};
const std::string sQM[] = {"prioil", "priohi", "priolo"};
const std::string sDL  = "listdst";
const std::string sDD  = "defdst";
const std::string sAD  = "altdst";
const std::string sTG  = "target";
const std::string sFD  = "flowdst";

//"struct0 [label=\"<f0> " << name[v] << " | <f1> " << tPeriod[v] << "\"];"; go for structs ...



/////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex Writer Visitor

void VisitorVertexWriter::eventString(const Event& el) const {
  out << " [shape=\"oval\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"grey\"";
  out << ", t_offs=" << el.getTOffs() << ", flags=" << el.getFlags();
}

void VisitorVertexWriter::commandString(const Command& el) const {
  out << ", t_valid=" << el.getTValid();
}

void VisitorVertexWriter::visit(const Block& el) const  { 
  out << " [shape=\"rectangle\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"grey\"";
  out << ", t_Period=" << el.getTPeriod();
  out << "]";
}

void VisitorVertexWriter::visit(const TimingMsg& el) const {
  eventString((Event&)el);
  out << ", type=\"TMsg\", color=\"black\"";
  out << ", id=" << el.getId();
  out << ", par=" << el.getPar();
  out << ", tef=" << el.getTef();
  out << ", res=" << el.getRes();
  out << "]";
}

void VisitorVertexWriter::visit(const Noop& el) const { 
  eventString((Event&)el);
  out << ", type=\"Noop\", color=\"green\"";
  commandString((Command&) el);
  out << ", qty=" << el.getQty();
  out << "]";
}

void VisitorVertexWriter::visit(const Flow& el) const  { 
  eventString((Event&)el);
  out << ", type=\"Flow\", color=\"blue\"";
  commandString((Command&) el);
  out << ", qty=" << el.getQty();
  out << "\"]";
}

void VisitorVertexWriter::visit(const Flush& el) const { 
  eventString((Event&)el);
  out << ", type=\"Flush\", color=\"red\"";
  commandString((Command&) el);
  out << ", prio=" << el.getPrio();
  out << "]";
}

void VisitorVertexWriter::visit(const Wait& el) const {
  out << " [shape=\"oval\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"grey\"";
  out << ", style=dashed, flags=" << el.getFlags();
  out << "]";
}

void VisitorVertexWriter::visit(const CmdQMeta& el) const {
  out << " [shape=\"oval\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"grey\"";
  out << ", style=dashed, flags=" << el.getFlags();
  out << "]";
}

void VisitorVertexWriter::visit(const CmdQBuffer& el) const {
  out << " [shape=\"oval\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"grey\"";
  out << ", style=dashed, flags=" << el.getFlags();
  out << "]";
}

void VisitorVertexWriter::visit(const DestList& el) const {
  out << " [shape=\"oval\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"grey\"";
  out << ", style=dashed, flags=" << el.getFlags();
  out << "]";
}




/////////////////////////////////////////////////////////////////////////////////////////////////
// Node Crawler Visitor


// Upload Crawler

void VisitorNodeUploadCrawler::visit(const Block& el) const {
  vAdr vA, tmpDD, tmpQM;
  tmpDD = getDefDst();
  tmpQM = getQInfo();

  vA.reserve( tmpDD.size() + tmpQM.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpQM.begin(), tmpQM.end() );

  el.serialise(vA);
}

void VisitorNodeUploadCrawler::visit(const TimingMsg& el) const  {
  el.serialise(getDefDst());
}

void VisitorNodeUploadCrawler::visit(const Flow& el) const  {
  vAdr vA, tmpDD, tmpCT, tmpFD;
  tmpDD = getDefDst();
  tmpCT = getCmdTarget();

  vA.reserve( tmpDD.size() + tmpCT.size() + tmpFD.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpCT.begin(), tmpCT.end() );
  vA.insert( vA.end(), tmpFD.begin(), tmpFD.end() );

  el.serialise(vA);

}

void VisitorNodeUploadCrawler::visit(const Flush& el) const {
  vAdr vA, tmpDD, tmpCT;
  tmpDD = getDefDst();
  tmpCT = getCmdTarget();

  vA.reserve( tmpDD.size() + tmpCT.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpCT.begin(), tmpCT.end() );

  el.serialise(vA);

}

void VisitorNodeUploadCrawler::visit(const Noop& el) const {
  vAdr vA, tmpDD, tmpCT;
  tmpDD = getDefDst();
  tmpCT = getCmdTarget();

  vA.reserve( tmpDD.size() + tmpCT.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpCT.begin(), tmpCT.end() );

  el.serialise(vA);

}

void VisitorNodeUploadCrawler::visit(const Wait& el) const {
  vAdr vA, tmpDD, tmpCT;
  tmpDD = getDefDst();
  tmpCT = getCmdTarget();

  vA.reserve( tmpDD.size() + tmpCT.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpCT.begin(), tmpCT.end() );

  el.serialise(vA);

}

void VisitorNodeUploadCrawler::visit(const CmdQMeta& el) const {
  vAdr vA, tmpDD, tmpQB;
  tmpDD = getDefDst();
  tmpQB = getQBuf();

  vA.reserve( tmpDD.size() + tmpQB.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpQB.begin(), tmpQB.end() );

  el.serialise(vA);
}

void VisitorNodeUploadCrawler::visit(const CmdQBuffer& el) const {
  el.serialise(getDefDst());
}

void VisitorNodeUploadCrawler::visit(const DestList& el) const {
  vAdr vA, tmpDD, tmpDL;
  tmpDD = getDefDst();
  tmpDL = getListDst();

  vA.reserve( tmpDD.size() + tmpDL.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpDL.begin(), tmpDL.end() );

  el.serialise(vA);
}

////////////////////////////////////////////////////////////////////
// Download Crawler
/*
   //find default destination
      uint32_t childAdr = writeBeBytesToLeNumber<uint32_t>((uint8_t*)&it.second.b[NODE_DEF_DEST_PTR]);
      if ( childAdr != LM32_NULL_PTR ) childAdr = intAdr2adr(childAdr);
    
      if (parserMap.count(childAdr) > 0) {
        auto& child = parserMap.at(childAdr);
        std::cout << gDown[it.second.v].name << "'s defDest is " << gDown[child.v].name << std::endl;
        boost::add_edge(it.second.v, child.v, (myEdge){"defDest"}, gDown);
      } else {
        std::cout << gDown[it.second.v].name << "has no defDest." << std::endl;
*/

void VisitorNodeDownloadCrawler::setDefDst() const {
  
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
  else if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, tmpParser->v, (myEdge){sDD}, g);

}




void VisitorNodeDownloadCrawler::visit(const Block& el) const {
    
  
  uint32_t tmpAdr;
  Graph& g = mmu.getDownGraph();
  uint8_t* b = (uint8_t*)&g[v].np->getB();
   Graph::in_edge_iterator in_begin, in_end;

  setDefDst();
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + BLOCK_ALT_DEST_PTR ));
  if (tmpAdr != LM32_NULL_PTR) { boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, (myEdge){sDL},          g);
  //std::cout << "Node " << g[v].name << " has destlist " << g[((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v].name << std::endl; 
  /*
  for (boost::tie(in_begin, in_end) = in_edges(((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v,g); in_begin != in_end; ++in_begin)
{   
    std::cout << "Parent of " << g[((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v].name << " is " << g[source(*in_begin,g)].name << std::endl;
}
*/
  }
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_IL_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, (myEdge){sQM[PRIO_IL]}, g);
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_HI_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, (myEdge){sQM[PRIO_HI]}, g);
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + BLOCK_CMDQ_LO_PTR ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, (myEdge){sQM[PRIO_LO]}, g);

}

void VisitorNodeDownloadCrawler::visit(const TimingMsg& el) const  {
  setDefDst();
}

void VisitorNodeDownloadCrawler::visit(const Flow& el) const  {
  
  uint32_t tmpAdr;
  Graph& g = mmu.getDownGraph();
  uint8_t* b = (uint8_t*)&g[v].np->getB();

  setDefDst();
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, (myEdge){sTG},          g);
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + CMD_FLOW_DEST ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, (myEdge){sFD},          g);

}

void VisitorNodeDownloadCrawler::visit(const Flush& el) const {
  
  uint32_t tmpAdr;
  Graph& g = mmu.getDownGraph();
  uint8_t* b = (uint8_t*)&g[v].np->getB();

  setDefDst();
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, (myEdge){sTG},          g);

}

void VisitorNodeDownloadCrawler::visit(const Noop& el) const {
  
  uint32_t tmpAdr;
  Graph& g = mmu.getDownGraph();
  uint8_t* b = (uint8_t*)&g[v].np->getB();

  setDefDst();
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, (myEdge){sTG},          g);

}

void VisitorNodeDownloadCrawler::visit(const Wait& el) const {
  
  uint32_t tmpAdr;
  Graph& g = mmu.getDownGraph();
  uint8_t* b = (uint8_t*)&g[v].np->getB();

  setDefDst();
  tmpAdr = mmu.intAdr2adr(writeBeBytesToLeNumber<uint32_t>(b + CMD_TARGET ));
  if (tmpAdr != LM32_NULL_PTR) boost::add_edge(v, ((parserMeta*)(mmu.lookupAdr(tmpAdr)))->v, (myEdge){sTG},          g);

}

void VisitorNodeDownloadCrawler::visit(const CmdQMeta& el) const {
  
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

void VisitorNodeDownloadCrawler::visit(const CmdQBuffer& el) const {
}

void VisitorNodeDownloadCrawler::visit(const DestList& el) const {
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

    //add all the alternative destination connections from the dest list to the parent block
    for (ptrdiff_t offs = DST_ARRAY + ADR_ALT_DST_ARRAY * _32b_SIZE_; offs < DST_ARRAY_END; offs += _32b_SIZE_) {
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private helper functions

 vAdr VisitorNodeUploadCrawler::getDefDst() const {
    bool found = false;
    Graph& g = mmu.getUpGraph();
    vAdr ret;
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(v,g);
 
    std::cout << g[v].name << ": " << std::endl;

    for (out_cur = out_begin; out_cur != out_end; ++out_cur)
    {   
      if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
      else {

        if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == sDD) {
          if (found) {std::cerr << "!!! Found more than one default destination !!!" << std::endl; break;
          } else {
            auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
            if (x != NULL) {
              ret.push_back(mmu.adr2intAdr(x->adr));
              found = true; 
              //std::cout << "defDst: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << x->adr << std::endl;
            }
          }
        }
      }  
    }
    if (!(found)) ret.push_back(LM32_NULL_PTR);

    return ret;
  }

  vAdr VisitorNodeUploadCrawler::getQInfo() const {
    int idx;
    bool found;
    Graph& g = mmu.getUpGraph();
    vAdr ret;
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(v,g);


    
    found = false;
    for (out_cur = out_begin; out_cur != out_end; ++out_cur)
    {   
      if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
      else {
        if (g[target(*out_cur,g)].np->isMeta() && g[target(*out_cur,g)].type == sDL) {
          if (found) {std::cerr << "!!! Found more than one Destination List !!!" << std::endl; break;
          } else {
            auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
            if (x != NULL) {
              ret.push_back(mmu.adr2intAdr(x->adr));
              found = true;
              //std::cout << g[target(*out_cur,g)].name << " @ 0x" << std::hex << mmu.adr2intAdr(x->adr) << std::endl;
            }
          }
        }
      }  
    }
    if (!(found)) ret.push_back(LM32_NULL_PTR);

    for (idx=0; idx < 3; idx++) {
      found = false;
      for (out_cur = out_begin; out_cur != out_end; ++out_cur)
      {   
        if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
        else {

          if (g[target(*out_cur,g)].np->isMeta() && g[*out_cur].type == sQM[idx]) {
            if (found) {std::cerr << "!!! Found more than one queue info of type " << sQM[idx] << " !!!" << std::endl; break;}
            else {
              auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
              if (x != NULL) {
                ret.push_back(mmu.adr2intAdr(x->adr));
                found = true;
                //std::cout << "qMeta: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << mmu.adr2intAdr(x->adr) << std::endl;
              }
            }  
          }
        }  
      }
      if (!(found)) ret.push_back(LM32_NULL_PTR);
    }

    return ret;
  }


vAdr VisitorNodeUploadCrawler::getQBuf() const {
  bool found;
  Graph& g = mmu.getUpGraph();
  vAdr ret;
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  boost::tie(out_begin, out_end) = out_edges(v,g);
  
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (g[target(*out_cur,g)].np->isMeta()) {
        auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
        if (x != NULL) {
          ret.push_back(mmu.adr2intAdr(x->adr));
          found = true;
          //std::cout << "qBuf: " <<  g[target(*out_cur,g)].name << " @ 0x" << std::hex << mmu.adr2intAdr(x->adr) << std::endl;
        }
      }
    }  
  }

  if (!(found)) ret.push_back(LM32_NULL_PTR);

  return ret;
}

vAdr VisitorNodeUploadCrawler::getCmdTarget() const {
  bool found;
  Graph& g = mmu.getUpGraph();
  vAdr ret;
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  boost::tie(out_begin, out_end) = out_edges(v,g);
  
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == sTG) {
        if (found) {std::cerr << "!!! Found more than one target !!!" << std::endl; break;
        } else {
          auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
          if (x != NULL) {
            ret.push_back(mmu.adr2intAdr(x->adr));
            found = true;
            //std::cout << "Target: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << mmu.adr2intAdr(x->adr) << std::endl;
          }
        }
      }
    }  
  }

  if (!(found)) ret.push_back(LM32_NULL_PTR);

  return ret;
}

vAdr VisitorNodeUploadCrawler::getFlowDst() const {
  bool found;
  Graph& g = mmu.getUpGraph();
  vAdr ret;


  Graph::out_edge_iterator out_begin, out_end, out_cur;
  boost::tie(out_begin, out_end) = out_edges(v,g);
  
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == sFD) {
        if (found) {std::cerr << "!!! Found more than one flow destination !!!" << std::endl; break;
        } else {
          auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
          if (x != NULL) {
            ret.push_back(mmu.adr2intAdr(x->adr));
            found = true; 
            //std::cout << "flowDst: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << mmu.adr2intAdr(x->adr) << std::endl;
          }
        }
      }
    }  
  }

  if (!(found)) ret.push_back(LM32_NULL_PTR);

  return ret;
}

vAdr VisitorNodeUploadCrawler::getListDst() const {
  bool found;
  Graph& g = mmu.getUpGraph();
  vAdr ret;
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  Graph::in_edge_iterator in_begin, in_end;
  vertex_t vp;

  //get the parent. there shall be only one, a block (no check for that right now, sorry)
  boost::tie(in_begin, in_end) = in_edges(v,g);
  vp = source(*in_begin,g);
  
  
  boost::tie(out_begin, out_end) = out_edges(vp,g);
  
  //search parent blocks default destination
  
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == sDD) {
        if (found) {std::cerr << "!!! Found more than one default destination !!!" << std::endl; break;
        } else {
          auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
          if (x != NULL) {
            ret.push_back(mmu.adr2intAdr(x->adr));
            found = true;
            //std::cout << "defDst: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << mmu.adr2intAdr(x->adr) << std::endl;
          }
        }
      }
    }  
  }
  if (!(found)) ret.push_back(LM32_NULL_PTR);

  //search parent blocks alternative destinations
  found = false;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
    else {

      if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == sAD) {
        auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
        if (x != NULL) {
          ret.push_back(mmu.adr2intAdr(x->adr));
          found = true;
          //std::cout << "altDst: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << mmu.adr2intAdr(x->adr) << std::endl;
        }
      }
    }  
  }
  if (!(found)) ret.push_back(LM32_NULL_PTR);

  
  return ret;

}

/*
void VisitorEdgeWriter::visit(const Block& el) const { std::cout << ("Hello!\n"); out << "[shape=\"rectangle\"]";  }
void VisitorEdgeWriter::visit(const TimingMsg& el) const { std::cout << "Visited a TimingMsg!";  out << "[shape=\"oval\", color=\"black\"]"; }//, label=\"" << el.getId() << "\"]"; }
void VisitorEdgeWriter::visit(const Flow& el) const { std::cout << "Visited a Flow!";    out << "[shape=\"oval\", color=\"blue\"]";}
void VisitorEdgeWriter::visit(const Flush& el) const { std::cout << "Visited a Flush!";  out << "[shape=\"oval\", color=\"red\"]";}
void VisitorEdgeWriter::visit(const Noop& el) const { std::cout << "Visited a Noop!";  out << "[shape=\"oval\", color=\"green\"]";}
*/


