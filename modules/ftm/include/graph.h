#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/graph/graphviz.hpp>
#include "ftm_common.h"
#include "common.h"


class Node;

class  myVertex {
public:
  std::string name;
  std::string cpu;
  uint32_t hash;
  node_ptr np;

  //FIXME
  //now follows a list of all possible properties graphviz_read can assign, to copy to concrete Node objects later
  //dirty business. this will have to go in the future
  // Option 1 (easy and clean, still needs ALL possible properties to be present in myVertex):
  //    pass Node constructor a reference to hosting myVertex Struct, Node can then use all relevent myVertex properties.
  //    Will make Node constructors VERY simple
  // Option 2 (hard, but very clean): 
  //    overload graphviz_read subfunctions so the parser evaluates typefield first, then have a 
  //    Node class factory and directly reference derived class members in property map.
  std::string type;

  std::string flags;

  //Meta

  //Block 
  std::string tPeriod = "0xD15EA5EDDEADBEEF";
  std::string rdIdxIl = "0", rdIdxHi = "0", rdIdxLo = "0";
  std::string wrIdxIl = "0", wrIdxHi = "0", wrIdxLo = "0";

  //Event
  std::string tOffs = "0xD15EA5EDDEADBEEF";

  //Timing Message
  std::string id = "0xD15EA5EDDEADBEEF";
  std::string id_fid    = "0";
  std::string id_gid    = "0";
  std::string id_evtno  = "0";
  std::string id_sid    = "0";
  std::string id_bpid   = "0";
  std::string id_res    = "0";

  std::string par = "0xD15EA5EDDEADBEEF";
  std::string tef = "0";
  std::string res = "0";

  //Command

  std::string tValid = "0";


  // Flush

  std::string qIl = "0", qHi = "0", qLo = "0";

  std::string frmIl = "0", toIl = "0";
  std::string frmHi = "0", toHi = "0";
  std::string frmLo = "0", toLo = "0"; 

  //Flow, Noop
  std::string prio = "0";
  std::string qty = "1";

  //Wait
  std::string tWait = "0xD15EA5EDDEADBEEF";

  std::string flowDest = "UNDEFINED";
  std::string flowTarget = "UNDEFINED";

  myVertex() : name("UNDEFINED"), cpu("0"), hash(0xDEADBEEF), np(NULL), type("UNDEFINED"), flags("0xDEADBEEF") {}
  
  myVertex(std::string name, std::string cpu, uint32_t hash, node_ptr np, std::string type, std::string flags) : name(name), cpu(cpu), hash(hash), np(np), type(type), flags(flags) {}

  //deep copy
  myVertex(myVertex const &aSource);

};


class myEdge {
public:
  std::string type;
  myEdge() : type("UNDEFINED") {}
  myEdge(std::string type) : type(type) {}
};




typedef boost::property<boost::graph_name_t, std::string > graph_p;
typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::bidirectionalS, myVertex, myEdge, graph_p  > Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;
typedef boost::container::vector<vertex_t> vVertices;


#endif