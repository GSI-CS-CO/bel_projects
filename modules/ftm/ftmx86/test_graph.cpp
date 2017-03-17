#include <boost/shared_ptr.hpp>
#include <stdio.h>
#include <iostream>
#include <inttypes.h>
#include "common.h"
#include "event.h"
#include "timeblock.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include "propwrite.h"
#include "node.h"
#include "timeblock.h"
#include "event.h"
#include "visitor.h"




  typedef struct {
    int type;
    uint64_t offs;  
  } myEdge;


typedef boost::shared_ptr<Node> node_ptr;



  typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::directedS, node_ptr, myEdge > Graph;
  typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;













int main() {

  


  boost::graph_traits < boost::adjacency_list <> >::vertex_iterator i, end;

  Graph g;


  vertex_t Aa = boost::add_vertex((node_ptr) new TimeBlock(0xcafebabedeadbee7ULL, true), g);
  vertex_t Ab = boost::add_vertex((node_ptr) new TimingMsg(0xcafebabedeadbee6ULL, 0, 42, 2, 4) , g);

  boost::add_edge(Aa, Ab, (myEdge){SYNC, 200}, g);

  std::ofstream out("./test.dot"); 

  
  Visitor v = Visitor(out);

  g[Aa]->accept(v);
  g[Ab]->accept(v);

  //boost::write_graphviz(out, g, node_writer());

  return 0;	
}





