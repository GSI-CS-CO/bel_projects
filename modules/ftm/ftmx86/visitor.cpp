#include "visitor.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"

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




void VisitorNodeCrawler::visit(const Block& el) const {
  vAdr vA, tmpDD, tmpQM;
  tmpDD = getDefDst();
  tmpQM = getQInfo();

  vA.reserve( tmpDD.size() + tmpQM.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpQM.begin(), tmpQM.end() );

  el.serialise(vA);
}

void VisitorNodeCrawler::visit(const TimingMsg& el) const  {
  el.serialise(getDefDst());
}

void VisitorNodeCrawler::visit(const Flow& el) const  {
  vAdr vA, tmpDD, tmpCT, tmpFD;
  tmpDD = getDefDst();
  tmpCT = getCmdTarget();

  vA.reserve( tmpDD.size() + tmpCT.size() + tmpFD.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpCT.begin(), tmpCT.end() );
  vA.insert( vA.end(), tmpFD.begin(), tmpFD.end() );

  el.serialise(vA);

}

void VisitorNodeCrawler::visit(const Flush& el) const {
  vAdr vA, tmpDD, tmpCT;
  tmpDD = getDefDst();
  tmpCT = getCmdTarget();

  vA.reserve( tmpDD.size() + tmpCT.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpCT.begin(), tmpCT.end() );

  el.serialise(vA);

}

void VisitorNodeCrawler::visit(const Noop& el) const {
  vAdr vA, tmpDD, tmpCT;
  tmpDD = getDefDst();
  tmpCT = getCmdTarget();

  vA.reserve( tmpDD.size() + tmpCT.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpCT.begin(), tmpCT.end() );

  el.serialise(vA);

}

void VisitorNodeCrawler::visit(const Wait& el) const {
  vAdr vA, tmpDD, tmpCT;
  tmpDD = getDefDst();
  tmpCT = getCmdTarget();

  vA.reserve( tmpDD.size() + tmpCT.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpCT.begin(), tmpCT.end() );

  el.serialise(vA);

}

void VisitorNodeCrawler::visit(const CmdQMeta& el) const {
  vAdr vA, tmpDD, tmpQB;
  tmpDD = getDefDst();
  tmpQB = getQBuf();

  vA.reserve( tmpDD.size() + tmpQB.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpQB.begin(), tmpQB.end() );

  el.serialise(vA);
}

void VisitorNodeCrawler::visit(const CmdQBuffer& el) const {
  el.serialise(getDefDst());
}

void VisitorNodeCrawler::visit(const DestList& el) const {
  vAdr vA, tmpDD, tmpDL;
  tmpDD = getDefDst();
  tmpDL = getListDst();

  vA.reserve( tmpDD.size() + tmpDL.size() ); // preallocate memory
  vA.insert( vA.end(), tmpDD.begin(), tmpDD.end() );
  vA.insert( vA.end(), tmpDL.begin(), tmpDL.end() );

  el.serialise(vA);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private helper functions

 vAdr VisitorNodeCrawler::getDefDst() const {
    bool found = false;
    Graph& g = mmu.getGraph();
    vAdr ret;
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(v,g);
 
    
    for (out_cur = out_begin; out_cur != out_end; ++out_cur)
    {   
      if (g[target(*out_cur,g)].np == NULL) std::cerr << g[target(*out_cur,g)].name << " is UNDEFINED" << std::endl;
      else {

        if (!(g[target(*out_cur,g)].np->isMeta()) && g[*out_cur].type == sDD) {
          if (found) {std::cerr << "!!! Found more than one default destination !!!" << std::endl; break;
          } else {
            auto* x = mmu.lookupName(g[target(*out_cur,g)].name);
            if (x != NULL) {ret.push_back(x->adr); found = true; std::cout << "defDst: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << x->adr << std::endl;}
          }
        }
      }  
    }
    if (!(found)) ret.push_back(LM32_NULL_PTR);

    return ret;
  }

  vAdr VisitorNodeCrawler::getQInfo() const {
    int idx;
    bool found;
    Graph& g = mmu.getGraph();
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
            if (x != NULL) {ret.push_back(x->adr); found = true; std::cout << g[target(*out_cur,g)].name << " @ 0x" << std::hex << x->adr << std::endl;}
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
              if (x != NULL) {ret.push_back(x->adr); found = true; std::cout << "qMeta: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << x->adr << std::endl;}
            }  
          }
        }  
      }
      if (!(found)) ret.push_back(LM32_NULL_PTR);
    }

    return ret;
  }


vAdr VisitorNodeCrawler::getQBuf() const {
  bool found;
  Graph& g = mmu.getGraph();
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
        if (x != NULL) {ret.push_back(x->adr); found = true; std::cout << "qBuf: " <<  g[target(*out_cur,g)].name << " @ 0x" << std::hex << x->adr << std::endl;}
      }
    }  
  }

  if (!(found)) ret.push_back(LM32_NULL_PTR);

  return ret;
}

vAdr VisitorNodeCrawler::getCmdTarget() const {
  bool found;
  Graph& g = mmu.getGraph();
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
          if (x != NULL) {ret.push_back(x->adr); found = true; std::cout << "Target: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << x->adr << std::endl;}
        }
      }
    }  
  }

  if (!(found)) ret.push_back(LM32_NULL_PTR);

  return ret;
}

vAdr VisitorNodeCrawler::getFlowDst() const {
  bool found;
  Graph& g = mmu.getGraph();
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
          if (x != NULL) {ret.push_back(x->adr); found = true; std::cout << "flowDst: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << x->adr << std::endl;}
        }
      }
    }  
  }

  if (!(found)) ret.push_back(LM32_NULL_PTR);

  return ret;
}

vAdr VisitorNodeCrawler::getListDst() const {
  bool found;
  Graph& g = mmu.getGraph();
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
          if (x != NULL) {ret.push_back(x->adr); found = true; std::cout << "defDst: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << x->adr << std::endl;}
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
        if (x != NULL) {ret.push_back(x->adr); found = true; std::cout << "altDst: " << g[target(*out_cur,g)].name << " @ 0x" << std::hex << x->adr << std::endl;}
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


