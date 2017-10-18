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
#include "dotstr.h"

using namespace DotStr;


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
  std::string tPeriod = tUndefined64;
  std::string rdIdxIl = tZero, rdIdxHi = tZero, rdIdxLo = tZero;
  std::string wrIdxIl = tZero, wrIdxHi = tZero, wrIdxLo = tZero;

  //Event
  std::string tOffs = tUndefined64;

  //Timing Message
  std::string id = tUndefined64;
  std::string id_fid    = tZero;
  std::string id_gid    = tZero;
  std::string id_evtno  = tZero;
  std::string id_sid    = tZero;
  std::string id_bpid   = tZero;
  std::string id_res    = tZero;

  std::string par = tUndefined64;
  std::string tef = tZero;
  std::string res = tZero;

  //Command

  std::string tValid = tZero;


  // Flush

  std::string qIl = tZero, qHi = tZero, qLo = tZero;

  std::string frmIl = tZero, toIl = tZero;
  std::string frmHi = tZero, toHi = tZero;
  std::string frmLo = tZero, toLo = tZero; 

  //Flow, Noop
  std::string prio = tZero;
  std::string qty = "1";

  //Wait
  std::string tWait = tUndefined64;

  std::string flowDest = tUndefined;
  std::string flowTarget = tUndefined;

  myVertex() : name(tUndefined), cpu(tZero), hash(uUndefined32), np(NULL), type(tUndefined), flags(tUndefined32) {}
  
  myVertex(std::string name, std::string cpu, uint32_t hash, node_ptr np, std::string type, std::string flags) : name(name), cpu(cpu), hash(hash), np(np), type(type), flags(flags) {}

  //deep copy
  myVertex(myVertex const &aSource);

};


class myEdge {
public:
  std::string type;
  myEdge() : type(tUndefined) {}
  myEdge(std::string type) : type(type) {}
};




typedef boost::property<boost::graph_name_t, std::string > graph_p;
typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::bidirectionalS, myVertex, myEdge, graph_p  > Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;
typedef boost::container::vector<vertex_t> vVertices;


#endif