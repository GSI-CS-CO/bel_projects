#include "visitor.h"
#include "node.h"
#include "timeblock.h"
#include "event.h"


//"struct0 [label=\"<f0> " << name[v] << " | <f1> " << period[v] << "\"];"; go for structs ...

void Visitor::eventString(Event& el) {
  out << " [shape=\"oval\"";
  out << ", t_offs=" << el.getTOffs();
  out << ", flags=" << el.getFlags();
}

void Visitor::commandString(Command& el) {
  printf("Hello?\n");
  out << ", t_valid=" << el.getTValid();
}

void Visitor::visitVertex(TimeBlock& el) { 
  out << " [shape=\"rectangle\"";
  out << ", t_period=" << el.getPeriod();
  if(el.hasCmdQ()) out << ", hasCmdQ=" << el.hasCmdQ();
  //out << ", cpu=" << el.getCpu();
  out << ", idx=" << el.getIdx();
  out << ", adr=" << el.getAdr();
  out << "]";
}

void Visitor::visitVertex(TimingMsg& el) {
  eventString((Event&)el);
  out << ", type=\"TMsg\", color=\"black\"";
  out << ", id=" << el.getId();
  out << ", par=" << el.getPar();
  out << ", tef=" << el.getTef();
  out << "]";
}

void Visitor::visitVertex(Noop& el) { 
  eventString((Event&)el);
  out << ", type=\"Noop\", color=\"green\"";
  commandString((Command&) el);
  out << ", qty=" << el.getQty();
  out << "]";
}

void Visitor::visitVertex(Flow& el) { 
  eventString((Event&)el);
  out << ", type=\"Flow\", color=\"blue\"";
  commandString((Command&) el);
  out << ", qty=" << el.getQty();
  out << ", dest=\""; 
  if (el.getNext() != NULL) out << "aName"; //el.blNext->name;
  else out << "None";
  out << "\"]";
}

void Visitor::visitVertex(Flush& el) { 
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


void Visitor::visitEdge(TimeBlock& el) { std::cout << ("Hello!\n"); out << "[shape=\"rectangle\"]";	}
void Visitor::visitEdge(TimingMsg& el) { std::cout << "Visited a TimingMsg!";	out << "[shape=\"oval\", color=\"black\"]"; }//, label=\"" << el.getId() << "\"]"; }
void Visitor::visitEdge(Flow& el) { std::cout << "Visited a Flow!";		out << "[shape=\"oval\", color=\"blue\"]";}
void Visitor::visitEdge(Flush& el) { std::cout << "Visited a Flush!";	out << "[shape=\"oval\", color=\"red\"]";}
void Visitor::visitEdge(Noop& el) { std::cout << "Visited a Noop!";	out << "[shape=\"oval\", color=\"green\"]";}
