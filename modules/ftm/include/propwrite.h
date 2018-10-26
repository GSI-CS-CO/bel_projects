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
#include "dotstr.h"

namespace ec = DotStr::EyeCandy;
namespace dgp = DotStr::Graph::Prop;
namespace dnp = DotStr::Node::Prop;
namespace dep = DotStr::Edge::Prop;
namespace det = DotStr::Edge::TypeVal;


  //FIXME use graph / node property tag string constants!


  template <class MetaMap>
  struct non_meta {
    non_meta() { }
    non_meta(MetaMap meta) : meta(meta) { }
    template <class Vertex>
    bool operator()(const Vertex& v) const {
      if (meta[v] != nullptr) return !(meta[v]->isMeta());
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
      out << "graph [root=\"" << sroot << "\"," << ec::Graph::sLookVert << "]" << std::endl;
      out << "node [" << ec::Node::Base::sLookDef << "]" << std::endl;
    }
  };

  template <class objMap >
  class vertex_writer {
  public:
    vertex_writer(objMap om) : om(om) {}
    template <class Vertex>
    void operator()(std::ostream& out, const Vertex& v) const {
      if (om[v] != nullptr) om[v]->accept(VisitorVertexWriter(out));
      //else std::cerr << "Vertex " << v << " has no node !!!" << std::endl;
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
      out << dnp::Base::sName << "=\"" << om[v] << "\"";
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
      out <<  "[" << dep::Base::sType << "=\"" << type[v] << "\", ";
      if      (type[v] == det::sBadDefDst)   out << ec::Edge::sLookbad;
      else if (type[v] == det::sDefDst)      out << ec::Edge::sLookDefDst;
      else if (type[v] == det::sAltDst)      out << ec::Edge::sLookAltDst;
      else if (type[v] == det::sCmdTarget)   out << ec::Edge::sLookTarget;
      else if (type[v] == det::sCmdFlowDst)  out << ec::Edge::sLookArgument;
      else if (type[v] == det::sCmdFlushOvr) out << ec::Edge::sLookArgument;
      else if (type[v] == det::sDynId)       out << ec::Edge::sLookArgument;
      else if (type[v] == det::sDynPar0)     out << ec::Edge::sLookArgument;
      else if (type[v] == det::sDynPar1)     out << ec::Edge::sLookArgument;
      else if (type[v] == det::sDynTef)      out << ec::Edge::sLookArgument;
      else if (type[v] == det::sDynRes)      out << ec::Edge::sLookArgument;
      else if (type[v] == det::sDynFlowDst)  out << ec::Edge::sLookDebug0;
      else if (type[v] == det::sResFlowDst)  out << ec::Edge::sLookDebug1;
      else if (type[v] == det::sDomFlowDst)  out << ec::Edge::sLookDebug2;
      else                                   out << ec::Edge::sLookMeta;
      out <<  "]";
    }
  private:
    typeMap type;
  };

  template <class typeMap>
  inline edge_writer<typeMap>
  make_edge_writer(typeMap type) {
    return edge_writer<typeMap>(type);
  }


  template <class typeMap>
  struct static_eq {
    static_eq() { } // necessary ?
    static_eq(typeMap type) : type(type) {}
    template <class Edge>
    bool operator()(const Edge& e) const {
      auto allowedTypes = {det::sDefDst, det::sResFlowDst, det::sDynFlowDst, det::sCmdTarget, det::sCmdFlowDst, det::sCmdFlushOvr};
      for(auto& it : allowedTypes ) { if (type[e] == it) return true; } //true if edge is of allowed types
      return false;
    }
    private:
      typeMap type;
  };


  template <class typeMap>
  inline static_eq<typeMap>
  make_static_eq(typeMap type) {
    return static_eq<typeMap>(type);
  }


#endif


