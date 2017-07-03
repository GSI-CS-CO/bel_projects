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



#include "visitorvertexwriter.h"
#include "common.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"




  template <class MetaMap>
  struct non_meta {
    non_meta() { }
    non_meta(MetaMap meta) : meta(meta) { }
    template <class Vertex>
    bool operator()(const Vertex& v) const {
      if (meta[v] != NULL) return !(meta[v]->isMeta());
      else return true;
    }
    MetaMap meta;
  };

  template <class MetaMap>
  inline non_meta<MetaMap> 
  make_non_meta(MetaMap meta) {
    return non_meta<MetaMap>(meta);
  }


  struct sample_graph_writer {
    const std::string& sroot;
    void operator()(std::ostream& out) const {
      out << "graph [root=\"" << sroot << "\", rankdir=TB, nodesep=0.6, mindist=1.0, ranksep=1.0, overlap=false]" << std::endl;
      out << "node [shape=\"rectangle\", style=\"filled\"]" << std::endl;
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
      if (om[v] != NULL) om[v]->accept(VisitorVertexWriter(out));
      else std::cerr << "Vertex " << v << " has no node !!!" << std::endl;
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






  template <class typeMap>
  class edge_writer {
  public:
    edge_writer(typeMap type) : type(type) {}
    template <class Edge>
    void operator()(std::ostream& out, const Edge& v) const {
      out <<  "[type=\"" << type[v] << "\", color=\""; 
      if      (type[v] == sDD) out << "red";
      else if (type[v] == sAD) out << "black";
      else if (type[v] == sTG) out << "blue";
      else if (type[v] == sFD) out << "magenta";
      else out << "grey\", label=\"" << type[v];
      out <<  "\"]";   
    }
  private:
    typeMap type;
  };

  template <class typeMap>
  inline edge_writer<typeMap> 
  make_edge_writer(typeMap type) {
    return edge_writer<typeMap>(type);
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
        out << "struct0 [label=\"<f0> " << name[v] << " | <f1> " << tPeriod[v] << "\"];";
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


