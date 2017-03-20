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
    int type;
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



  vertex_t Ca = boost::add_vertex((myNode){BLOCK, "C_pre"}, g);
  vertex_t Ca0 = boost::add_vertex((myNode){TIMING_EVT, "C_pre_evt0"}, g);
  vertex_t Ca1 = boost::add_vertex((myNode){TIMING_EVT, "C_pre_evt1"}, g);

  vertex_t Cb = boost::add_vertex((myNode){BLOCK, "C_main0"}, g);
  vertex_t Cb0 = boost::add_vertex((myNode){TIMING_EVT, "C_main_evt0"}, g);
  vertex_t Cb1 = boost::add_vertex((myNode){TIMING_EVT, "C_main_evt1"}, g);
  vertex_t Cb2 = boost::add_vertex((myNode){COMMAND_EVT"C_main_evt2"}, g);
  vertex_t Cb3 = boost::add_vertex((myNode){TIMING_EVT, "C_main_evt3"}, g);

  vertex_t Cc = boost::add_vertex((myNode){BLOCK, "C_post"}, g);  
  vertex_t Cc0 = boost::add_vertex((myNode){COMMAND_EVT, "C_post_evt0"}, g);
  vertex_t Cc1 = boost::add_vertex((myNode){TIMING_EVT, "C_post_evt1"}, g);
  vertex_t Cc2 = boost::add_vertex((myNode){TIMING_EVT, "C_post_evt2"}, g);


  boost::add_edge(Ca, Cb, (myEdge){DEFAULT, 0}, g);
  boost::add_edge(Ca, Ca0, (myEdge){SYNC, 100}, g);
  boost::add_edge(Ca, Ca1, (myEdge){SYNC, 150}, g);
  

  boost::add_edge(Cb, Cc, (myEdge){DEFAULT, 0}, g);
  boost::add_edge(Cb, Cb0, (myEdge){SYNC, 25}, g);
  boost::add_edge(Cb, Cb1, (myEdge){SYNC, 90}, g);
  boost::add_edge(Cb, Cb2, (myEdge){SYNC, 90}, g);
  boost::add_edge(Cb, Cb3, (myEdge){SYNC, 1000}, g);

  

  boost::add_edge(Cc, Cc0, (myEdge){SYNC, 0}, g);
  boost::add_edge(Cc, Cc1, (myEdge){SYNC, 500}, g);
  boost::add_edge(Cc, Cc2, (myEdge){SYNC, 1000}, g);


  for (i = boost::vertices(g).first;  i != boost::vertices(g).second; i++) {
    printf("Name of node %s \n", g[*i].name);
  }

  std::ofstream out("./test_sub.dot"); 



  boost::write_graphviz(out, g, make_vertex_writer(boost::get(&myNode::type, g), boost::get(&myNode::name, g)),  make_edge_writer(boost::get(&myEdge::type, g), boost::get(&myEdge::offs, g)), sample_graph_writer());

 

  return 0;	
}





