#include "visitor.h"
#include "node.h"
#include "timeblock.h"
#include "event.h"


//"struct0 [label=\"<f0> " << name[v] << " | <f1> " << period[v] << "\"];"; go for structs ...

void Visitor::visitVertex(TimeBlock& el) { std::cout << ("Hello!\n"); out << "[shape=\"rectangle\"]";	}
void Visitor::visitVertex(TimingMsg& el) { std::cout << "Visited a TimingMsg!";	out << "[shape=\"oval\", color=\"black\"]"; }//, label=\"" << el.getId() << "\"]"; }
void Visitor::visitVertex(Flow& el) { std::cout << "Visited a Flow!";		out << "[shape=\"oval\", color=\"blue\"]";}
void Visitor::visitVertex(Flush& el) { std::cout << "Visited a Flush!";	out << "[shape=\"oval\", color=\"red\"]";}
void Visitor::visitVertex(Noop& el) { std::cout << "Visited a Noop!";	out << "[shape=\"oval\", color=\"green\"]";}

void Visitor::visitEdge(TimeBlock& el) { std::cout << ("Hello!\n"); out << "[shape=\"rectangle\"]";	}
void Visitor::visitEdge(TimingMsg& el) { std::cout << "Visited a TimingMsg!";	out << "[shape=\"oval\", color=\"black\"]"; }//, label=\"" << el.getId() << "\"]"; }
void Visitor::visitEdge(Flow& el) { std::cout << "Visited a Flow!";		out << "[shape=\"oval\", color=\"blue\"]";}
void Visitor::visitEdge(Flush& el) { std::cout << "Visited a Flush!";	out << "[shape=\"oval\", color=\"red\"]";}
void Visitor::visitEdge(Noop& el) { std::cout << "Visited a Noop!";	out << "[shape=\"oval\", color=\"green\"]";}
