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
  std::string tPeriod;
  std::string rdIdxIl, rdIdxHi, rdIdxLo;
  std::string wrIdxIl, wrIdxHi, wrIdxLo;

  //Event
  std::string tOffs;

  //Timing Message
  std::string id;
  std::string id_fid;
  std::string id_gid;
  std::string id_evtno;
  std::string id_sid;
  std::string id_bpid;
  std::string id_res;

  std::string par;
  std::string tef;
  std::string res;

  //Command

  std::string tValid;


  // Flush

  std::string qIl, qHi, qLo;

  std::string frmIl, toIl;
  std::string frmHi, toHi;
  std::string frmLo, toLo; 

  //Flow, Noop
  std::string prio;
  std::string qty;

  //Wait
  std::string tWait;

  myVertex() : name("UNDEFINED"), cpu("0"), hash(0xDEADBEEF), np(NULL), type("UNDEFINED"), flags("0xDEADBEEF"), tPeriod("0xD15EA5EDDEADBEEF"), rdIdxIl("0"), rdIdxHi("0"), rdIdxLo("0"), 
  wrIdxIl("0"), wrIdxHi("0"), wrIdxLo("0"), tOffs("0xD15EA5EDDEADBEEF"), id("0xD15EA5EDDEADBEEF"), id_fid("0"), id_gid("0"), id_evtno("0"), id_sid("0"), id_bpid("0"), id_res("0"),
  par("0xD15EA5EDDEADBEEF"), tef("0"), res("0"), tValid("0xD15EA5EDDEADBEEF"),
  qIl("0"), qHi("0"), qLo("0"), frmIl("0"), toIl("0"), frmHi("0"), toHi("0"), frmLo("0"), toLo("0"), prio("0"), qty("1"), tWait("0xD15EA5EDDEADBEEF") {}
  
  myVertex(std::string name, std::string cpu, uint32_t hash, node_ptr np, std::string type, std::string flags) : name(name), cpu(cpu), hash(hash), np(np), type(type), flags(flags), tPeriod("0xD15EA5EDDEADBEEF"),
  rdIdxIl("0"), rdIdxHi("0"), rdIdxLo("0"), wrIdxIl("0"), wrIdxHi("0"), wrIdxLo("0"), tOffs("0xD15EA5EDDEADBEEF"), id("0xD15EA5EDDEADBEEF"), id_fid("0"), id_gid("0"), id_evtno("0"), id_sid("0"), id_bpid("0"), id_res("0"),
  par("0xD15EA5EDDEADBEEF"), tef("0"), res("0"), tValid("0xD15EA5EDDEADBEEF"), qIl("0"), qHi("0"), qLo("0"), frmIl("0"), toIl("0"), frmHi("0"), toHi("0"), frmLo("0"), toLo("0"), prio("0"), qty("1"), tWait("0xD15EA5EDDEADBEEF") {}

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