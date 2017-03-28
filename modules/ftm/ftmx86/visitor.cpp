#include "visitor.h"
#include "node.h"
#include "timeblock.h"
#include "event.h"


//"struct0 [label=\"<f0> " << name[v] << " | <f1> " << period[v] << "\"];"; go for structs ...

Graph Visitor::defaultGraph;
vertex_t Visitor::defaultVertex;

void Visitor::eventString(const Event& el) const {
  out << " [shape=\"oval\"";
  out << ", t_offs=" << el.getTOffs();
  out << ", flags=" << el.getFlags();
}

void Visitor::commandString(const Command& el) const {
  out << ", t_valid=" << el.getTValid();
}

void Visitor::visitVertex(const TimeBlock& el) const  { 
  out << " [shape=\"rectangle\"";
  out << ", t_period=" << el.getTPeriod();
  if(el.hasCmdQ()) out << ", color=\"red\", hasCmdQ=" << el.hasCmdQ();
  //out << ", cpu=" << el.getCpu();
  out << ", idx=" << el.getIdx();
  out << ", adr=" << el.getAdr();
  out << "]";
}

void Visitor::visitVertex(const TimingMsg& el) const {
  eventString((Event&)el);
  out << ", type=\"TMsg\", color=\"black\"";
  out << ", id=" << el.getId();
  out << ", par=" << el.getPar();
  out << ", tef=" << el.getTef();
  out << "]";
}

void Visitor::visitVertex(const Noop& el) const { 
  eventString((Event&)el);
  out << ", type=\"Noop\", color=\"green\"";
  commandString((Command&) el);
  out << ", qty=" << el.getQty();
  out << "]";
}

void Visitor::visitVertex(const Flow& el) const  { 
  eventString((Event&)el);
  out << ", type=\"Flow\", color=\"blue\"";
  commandString((Command&) el);
  out << ", qty=" << el.getQty();
  out << ", dest=\""; 
  if (el.getNext() != NULL) out << "aName"; //el.blNext->name;
  else out << "None";
  out << "\"]";
}

void Visitor::visitVertex(const Flush& el) const { 
  eventString((Event&)el);
  out << ", type=\"Flush\", color=\"red\"";
  commandString((Command&) el);
  out << ", flushIlQ=" << el.getFlushQil();
  out << ", flushHiQ=" << el.getFlushQhi();
  out << ", flushHiupTo=" << el.getFlushUpToHi();
  out << ", flushLoQ=" << el.getFlushQlo();
  out << ", flushLoupTo=" << el.getFlushUpToLo();
  out << "]";
}


void Visitor::visitSerialiser(const TimeBlock& el) const {std::cout <<  "TB  Ser for " << g[n].name << "\n"; }
void Visitor::visitSerialiser(const Event& el) const     {std::cout << "Evt Ser for " << g[n].name << "\n";}

void Visitor::visitEdge(const TimeBlock& el) const { std::cout << ("Hello!\n"); out << "[shape=\"rectangle\"]";	}
void Visitor::visitEdge(const TimingMsg& el) const { std::cout << "Visited a TimingMsg!";	out << "[shape=\"oval\", color=\"black\"]"; }//, label=\"" << el.getId() << "\"]"; }
void Visitor::visitEdge(const Flow& el) const { std::cout << "Visited a Flow!";		out << "[shape=\"oval\", color=\"blue\"]";}
void Visitor::visitEdge(const Flush& el) const { std::cout << "Visited a Flush!";	out << "[shape=\"oval\", color=\"red\"]";}
void Visitor::visitEdge(const Noop& el) const { std::cout << "Visited a Noop!";	out << "[shape=\"oval\", color=\"green\"]";}



