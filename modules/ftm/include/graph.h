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

using namespace DotStr::Misc;


class Node;

class  myVertex {
public:
  std::string name = sUndefined;
  std::string patName   = sUndefined;
  std::string bpName    = sUndefined;
  std::string cpu       = sUndefined;
  std::string thread    = sUndefined;
  uint32_t hash = uUndefined32;
  node_ptr np = nullptr;

  
  std::string patEntry  = sZero;
  std::string patExit   = sZero;

  
  std::string bpEntry   = sZero;
  std::string bpExit    = sZero;

  //FIXME
  //now follows a list of all possible properties graphviz_read can assign, to copy to concrete Node objects later
  //dirty business. this will have to go in the future
  // Option 1 (easy and clean, still needs ALL possible properties to be present in myVertex):
  //    pass Node constructor a reference to hosting myVertex Struct, Node can then use all relevent myVertex properties.
  //    Will make Node constructors VERY simple
  // Option 2 (hard, but very clean): 
  //    overload graphviz_read subfunctions so the parser evaluates typefield first, then have a 
  //    Node class factory and directly reference derived class members in property map.
  std::string type = sUndefined;

  std::string flags = sUndefined32;

  //Meta

  //Block 
  std::string tPeriod = sUndefined64;
  std::string rdIdxIl = sZero, rdIdxHi = sZero, rdIdxLo = sZero;
  std::string wrIdxIl = sZero, wrIdxHi = sZero, wrIdxLo = sZero;

  //Event
  std::string tOffs = sUndefined64;

  //Timing Message
  std::string id = sUndefined64;
  std::string id_fid    = sZero;
  std::string id_gid    = sZero;
  std::string id_evtno  = sZero;
  std::string id_sid    = sZero;
  std::string id_bpid   = sZero;
  std::string id_res    = sZero;
  std::string id_bin    = sZero;
  std::string id_reqnob = sZero;
  std::string id_vacc   = sZero;
  std::string par = sUndefined64;
  std::string tef = sZero;
  std::string res = sZero;

  //Command

  std::string tValid = sZero;


  // Flush

  std::string qIl = sZero, qHi = sZero, qLo = sZero;

  std::string frmIl = sZero, toIl = sZero;
  std::string frmHi = sZero, toHi = sZero;
  std::string frmLo = sZero, toLo = sZero; 

  //Flow, Noop
  std::string prio = sZero;
  std::string qty = "1";
  std::string perma = sZero;

  //Wait
  std::string tWait = sUndefined64;

  std::string cmdTarget   = sUndefined;
  std::string cmdDest     = sUndefined;
  std::string cmdDestBp   = sUndefined;
  std::string cmdDestPat  = sUndefined;

  myVertex() {}
  
  myVertex(std::string name, std::string cpu, uint32_t hash, node_ptr np, std::string type, std::string flags) : name(name), cpu(cpu), hash(hash), np(np), type(type), flags(flags) {}
  myVertex(std::string name, std::string pattern, std::string beamproc, std::string cpu, uint32_t hash, node_ptr np, std::string type, std::string flags) : name(name), patName(pattern), bpName(beamproc), cpu(cpu), hash(hash), np(np), type(type), flags(flags) {}

  //deep copy
  myVertex(myVertex const &aSource);

};


class myEdge {
public:
  std::string type;
  myEdge() : type(sUndefined) {}
  myEdge(std::string type) : type(type) {}
};

//TODO change to aliases C++14 Style?
typedef boost::property<boost::graph_name_t, std::string > graph_p;
typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::bidirectionalS, myVertex, myEdge, graph_p  > Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;
typedef boost::container::vector<vertex_t> vVertices;
typedef std::map<vertex_t, vertex_t> vertex_map_t;
typedef std::set<vertex_t> vertex_set_t;

typedef boost::property_map<Graph, std::string myVertex::*>::type NameMap;

template <class NameMap >
struct nameEqualityFilter {
    nameEqualityFilter(NameMap n, NameMap nc) : names(n), compNames(nc) { }
  template <typename Vertex>
  bool operator()(const Vertex& v) const {
    bool ret = false;
    for(auto& it : compNames ) { ret |= (names[v] == it.second); } //true if name is known. Hate property maps for not having a test method...
    return ret;  
  }
  NameMap names;
  NameMap compNames;
};
/*

template <class TypeMap >
struct staticEdgeFilter {
    staticEdgeFilter(TypeMap n, vStrC t) : types(n), allowedTypes(t) { }
  template <typename Vertex>
  bool operator()(const Vertex& v) const {
    
    for(auto& it : allowedTypes ) { if (types[v] == it) return true; } //true if edge is of allowed type
    return false;  
  }
  TypeMap types;
  vStrC allowedTypes;
};
 
 vStrC allowedTypes = {sDefDst, sCmdFlowDst, sDynFlowDst};
*/
#endif