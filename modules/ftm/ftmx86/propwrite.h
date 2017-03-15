#ifndef _PROP_WRITE_H_
#define _PROP_WRITE_H_

#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

#define STANDARD 0  
#define DEFAULT 1
#define SYNC    2

  struct sample_graph_writer {
    void operator()(std::ostream& out) const {
      out << "graph [rankdir=LR, mindist=3.0]" << std::endl;
      out << "node [shape=rectangle]" << std::endl;
      //out << "node [shape=circle color=white]" << std::endl;
      //out << "edge [style=dashed]" << std::endl;
    }
  };


  template <class typeMap, class offsMap>
  class edge_writer {
  public:
    edge_writer(typeMap type, offsMap offs) : type(type),  offs(offs) {}
    template <class Edge>
    void operator()(std::ostream& out, const Edge& v) const {
      char sep = ' ';      
      out <<  "[";      
      if (type[v] == DEFAULT) {out <<  "color=\"red\""; sep = ',';}
      if (type[v] == SYNC) {out <<  sep << " style=\"dashed\", label=\"" << offs[v] << "\" ";}
      out <<  "]";   
    }
  private:
    typeMap type;
    offsMap offs;
  };


  /*

   //doesn't work - shitty BGL example is missing the implementation ...

  template <class defMap>
  edge_writer<defMap>
  make_edge_writer(defMap n);

  */



  //works

  template <class typeMap, class offsMap>
  inline edge_writer<typeMap, offsMap> 
  make_edge_writer(typeMap type, offsMap offs) {
    return edge_writer<typeMap, offsMap>(type, offs);
  }





#endif


