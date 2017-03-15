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



  typedef struct {
    const char* name;
  } myNode;

  typedef struct {
    int type;
    uint64_t offs;  
  } myEdge;


  typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::directedS, myNode, myEdge > Graph;
  typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;













int main() {

  


  boost::graph_traits < boost::adjacency_list <> >::vertex_iterator i, end;

  Graph g;


  vertex_t Aa = boost::add_vertex((myNode){"A_pre"}, g);
  vertex_t Ab = boost::add_vertex((myNode){"A_main0"}, g);
  vertex_t Ac = boost::add_vertex((myNode){"A_main1a"}, g);
  vertex_t Ad = boost::add_vertex((myNode){"A_main1b"}, g);
  vertex_t Ae = boost::add_vertex((myNode){"A_main2"}, g);
  vertex_t Af = boost::add_vertex((myNode){"A_post"}, g);
  vertex_t Ba = boost::add_vertex((myNode){"B_pre"}, g);
  vertex_t Bb = boost::add_vertex((myNode){"B_main0"}, g);
  vertex_t Bc = boost::add_vertex((myNode){"B_main1a"}, g);
  vertex_t Bd = boost::add_vertex((myNode){"B_main1b"}, g);
  vertex_t Be = boost::add_vertex((myNode){"B_main1c"}, g);
  vertex_t Bf = boost::add_vertex((myNode){"B_main2"}, g);
  vertex_t Bg = boost::add_vertex((myNode){"B_post"}, g);
  vertex_t Ca = boost::add_vertex((myNode){"C_pre"}, g);
  vertex_t Cb = boost::add_vertex((myNode){"C_main0"}, g);
  vertex_t Cc = boost::add_vertex((myNode){"C_post"}, g);  

  boost::add_edge(Aa, Ab, (myEdge){DEFAULT, 0}, g);
  boost::add_edge(Ab, Ac, (myEdge){DEFAULT, 0}, g);
  boost::add_edge(Ab, Ad, (myEdge){STANDARD, 0}, g);
  boost::add_edge(Ac, Ae, (myEdge){DEFAULT, 0}, g);
  boost::add_edge(Ad, Ae, (myEdge){STANDARD, 0},  g);
  boost::add_edge(Ae, Af, (myEdge){DEFAULT, 0}, g);
  boost::add_edge(Ae, Ab, (myEdge){STANDARD, 0}, g);

  boost::add_edge(Ba, Bb, (myEdge){DEFAULT, 0}, g);
  boost::add_edge(Bb, Bc, (myEdge){DEFAULT, 0}, g);
  boost::add_edge(Bb, Bd, (myEdge){STANDARD, 0}, g);
  boost::add_edge(Bb, Be, (myEdge){STANDARD, 0}, g);
  boost::add_edge(Bc, Bf, (myEdge){DEFAULT, 0}, g);
  boost::add_edge(Bd, Bf, (myEdge){STANDARD, 0},  g);
  boost::add_edge(Be, Bf, (myEdge){STANDARD, 0}, g);
  boost::add_edge(Bf, Bg, (myEdge){STANDARD, 0}, g);
  boost::add_edge(Bf, Bb, (myEdge){DEFAULT, 0}, g);

  boost::add_edge(Ca, Cb, (myEdge){DEFAULT, 0}, g);
  boost::add_edge(Cb, Cc, (myEdge){DEFAULT, 0}, g);

  boost::add_edge(Aa, Ba, (myEdge){SYNC, 110}, g);
  boost::add_edge(Ab, Bb, (myEdge){SYNC, 130}, g);
  boost::add_edge(Af, Bg, (myEdge){SYNC, 130}, g);
  boost::add_edge(Ba, Ca, (myEdge){SYNC, 200}, g);
  boost::add_edge(Bb, Cb, (myEdge){SYNC, 210}, g);
  boost::add_edge(Bg, Cc, (myEdge){SYNC, 200}, g);

  for (i = boost::vertices(g).first;  i != boost::vertices(g).second; i++) {
    printf("Name of node %s \n", g[*i].name);
  }

  std::ofstream out("./test.dot"); 



  boost::write_graphviz(out, g, make_label_writer(boost::get(&myNode::name, g)),  make_edge_writer(boost::get(&myEdge::type, g), boost::get(&myEdge::offs, g)), sample_graph_writer());

  return 0;	
}





