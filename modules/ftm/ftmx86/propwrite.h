#ifndef _PROP_WRITE_H_
#define _PROP_WRITE_H_
#include <string>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

#define STANDARD 0  
#define DEFAULT 1
#define SYNC    2

#define BLOCK 0
#define TIMING_EVT 1
#define COMMAND_EVT 2



#include "visitor.h"
typedef boost::shared_ptr<Node> node_ptr;

  struct sample_graph_writer {
    const std::string& sroot;
    void operator()(std::ostream& out) const {
      out << "graph [root=" << sroot << ", rankdir=LR, nodesep=0.6, mindist=1.0, ranksep=1.0, overlap=false]" << std::endl;
      out << "node [shape=\"rectangle\"]" << std::endl;
      //out << "node [shape=circle color=white]" << std::endl;
      //out << "edge [style=dashed]" << std::endl;
    }
  };

  template <class objMap >
  class vertex_writer {
  public:
    vertex_writer(objMap om) : om(om) {}
    template <class Vertex>
    void operator()(std::ostream& out, const Vertex& v) const {
      Visitor vs = Visitor(out);
      om[v]->acceptVertex(vs);
    }
  private:
    objMap om;
  };

  template <class objMap>
  inline vertex_writer<objMap> 
  make_vertex_writer(objMap om) {
    return vertex_writer<objMap>(om);
  }

template <class Name>
  class label_writer {
  public:
    label_writer(Name _name) : name(_name) {}
    template <class VertexOrEdge>
    void operator()(std::ostream& out, const VertexOrEdge& v) const {
      out << "[label=\"" << name[v] << "\"]";
    }
  private:
    Name name;
  };


  template <class objMap >
  class id_writer {
  public:
    id_writer(objMap om) : om(om) {}
    template <class VertexOrEdge>
    void operator()(std::ostream& out, const VertexOrEdge& v) const {
      out << "node_id=\"" << om[v] << "\""; 
    }
  private:
    objMap om;
  };

  template <class objMap>
  inline id_writer<objMap> 
  make_id_writer(objMap om) {
    return id_writer<objMap>(om);
  }






  template <class typeMap, class offsMap>
  class edge_writer {
  public:
    edge_writer(typeMap type, offsMap offs) : type(type),  offs(offs) {}
    template <class Edge>
    void operator()(std::ostream& out, const Edge& v) const {
      out <<  "[";
      if (type[v] == STANDARD) {out <<  "color=\"black\", label=\"" << offs[v] << "\"";}     
      if (type[v] == DEFAULT) {out <<  "color=\"red\", label=\"" << offs[v] << "\"";}
      if (type[v] == SYNC) {out <<  "  style=\"dashed\", label=\"" << offs[v] << "\" ";} //constraint=false,
      out <<  "]";   
    }
  private:
    typeMap type;
    offsMap offs;
  };

  template <class typeMap, class offsMap>
  inline edge_writer<typeMap, offsMap> 
  make_edge_writer(typeMap type, offsMap offs) {
    return edge_writer<typeMap, offsMap>(type, offs);
  }
/*
  template <class typeMap, class nameMap, >
  class vertex_writer {
  public:
    vertex_writer(typeMap type, nameMap name) : type(type),  name(name) {}
    template <class Vertex>
    void operator()(std::ostream& out, const Vertex& v) const {
      char sep = ' ';      
      if (type[v] == BLOCK) {
        out << "struct0 [label=\"<f0> " << name[v] << " | <f1> " << period[v] << "\"];";
      }
      else {     
        out <<  "[";
        if (type[v] == TIMING_EVT || type[v] == COMMAND_EVT) {out <<  " shape=\"oval\", evtpar=\"test\""; sep = ',';}     
        if (type[v] == COMMAND_EVT) {out <<  " style=\"dotted\""; sep = ',';}
        out <<  sep << " label=\"" << name[v] << "\" ]"; 
      }
    }
  private:
    typeMap type;
    nameMap name;
  };

  template <class typeMap, class nameMap>
  inline vertex_writer<typeMap, nameMap> 
  make_vertex_writer(typeMap type, nameMap name) {
    return vertex_writer<typeMap, nameMap>(type, name);
  }

*/
  /*

   //doesn't work - shitty BGL example is missing the implementation ...

  template <class defMap>
  edge_writer<defMap>
  make_edge_writer(defMap n);

  */



  //works

  





#endif


