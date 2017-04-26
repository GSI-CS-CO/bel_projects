#include "visitor.h"
#include "node.h"
#include "meta.h"
#include "event.h"


//"struct0 [label=\"<f0> " << name[v] << " | <f1> " << tPeriod[v] << "\"];"; go for structs ...



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
  out << ", t_tPeriod=" << el.getTPeriod();
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

void VisitorVertexWriter::visit(const CmdQueue& el) const {
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

void VisitorVertexWriter::visit(const AltDestList& el) const {
  out << " [shape=\"oval\"";
  if(el.isPainted()) out << ", fillcolor=\"green\"";
  else out << ", fillcolor=\"grey\"";
  out << ", style=dashed, flags=" << el.getFlags();
  out << "]";
}

/*
void VisitorCreateMemBlock::visit(const Block& el) const {
  
  //does the allocator know me?
  //yes
    //am I an updated block?
  //no
    //create allocator map entry
  //call serialise functions with referenec to allocator map 
  
  g[n].np->vB.clear();

  //what about all the destructors in node class? do it right!



  el.show(0, g[n].name.c_str()); 
  npBuf npB;

  // get all children, don't add yourself
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  boost::tie(out_begin, out_end) = out_edges(n,g);
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
     //std::cout << g[target(*out_cur,g)].name << " x " << (vertex_t)target(*out_cur,g) << std::endl;
     g[target(*out_cur,g)].np->accept(VisitorAddEvtChildren((vertex_t)target(*out_cur,g), g, npB));       
  }
  //sort events by time offset
  std::sort(npB.begin(), npB.end(), NodeSortPredicate);

  g[n].np->vBuf.reserve( (npB.end() - npB.begin()) * _EVT_SIZE + _BLOCK_HDR_SIZE ); // preallocate memory
  
  vBuf vBtmp(_BLOCK_HDR_SIZE); //create and fill temp buffer
  itBuf ser = vBtmp.begin();
  g[n].np->serialise(ser); 

  g[n].np->vB.insert( g[n].np->vB.end(), vBtmp.begin(), vBtmp.end() );   // insert block hdr 

    //serialise / show all found evt descendants
  boost::container::vector<node_ptr>::iterator it;
  for(it=npB.begin() ; it < npB.end(); it++) {
    (*it)->show(it - npB.begin(), "  ");
    g[n].np->vB.clear();
    //reserve size?
    g[n].np->serialise((*it)->vB.begin()); //serialise into local node buffer

    g[n].np->vBuf.insert( g[n].np->vB.end(), (*it)->vB.begin(), (*it)->vB.end() ); // insert all events
  }

}

void VisitorAddEvtChildren::visit(const Event& el) const {
 
  

  Graph::out_edge_iterator out_begin, out_end, out_cur;
  
  //add yourself, call for all children
  npB.push_back(g[n].np);

  boost::tie(out_begin, out_end) = out_edges(n,g);
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
      //std::cout << g[target(*out_cur,g)].name << " x " << (vertex_t)target(*out_cur,g) << std::endl;
     g[target(*out_cur,g)].np->accept(VisitorAddEvtChildren((vertex_t)target(*out_cur,g), g, npB));       
  }

}

void VisitorEdgeWriter::visit(const Block& el) const { std::cout << ("Hello!\n"); out << "[shape=\"rectangle\"]";  }
void VisitorEdgeWriter::visit(const TimingMsg& el) const { std::cout << "Visited a TimingMsg!";  out << "[shape=\"oval\", color=\"black\"]"; }//, label=\"" << el.getId() << "\"]"; }
void VisitorEdgeWriter::visit(const Flow& el) const { std::cout << "Visited a Flow!";    out << "[shape=\"oval\", color=\"blue\"]";}
void VisitorEdgeWriter::visit(const Flush& el) const { std::cout << "Visited a Flush!";  out << "[shape=\"oval\", color=\"red\"]";}
void VisitorEdgeWriter::visit(const Noop& el) const { std::cout << "Visited a Noop!";  out << "[shape=\"oval\", color=\"green\"]";}
*/


