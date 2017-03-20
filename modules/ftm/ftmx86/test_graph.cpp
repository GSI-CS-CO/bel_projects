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

typedef boost::shared_ptr<Node> node_ptr;


  typedef struct {
    std::string name;
    node_ptr np;
  } myVertex;

  typedef struct {
    int type;
    uint64_t offs;  
  } myEdge;





  typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::directedS, myVertex, myEdge > Graph;
  typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;








void addEdge(vertex_t& p, vertex_t& c) {
  //add visitor for 



}




int main() {

  

  boost::graph_traits < boost::adjacency_list <> >::vertex_iterator i, end;

  Graph g;

  vertex_t root = boost::add_vertex((myVertex) {"A000", (node_ptr)new TimeBlock(0xcafebabedeadbee7ULL, true)}, g);
  vertex_t Aa = boost::add_vertex((myVertex) {"Evt_A_0", (node_ptr)new TimingMsg(0xcafebabedeadbee6ULL, 0, 42, 2, 4)} , g);
  vertex_t Ab = boost::add_vertex((myVertex) {"Evt_A_1", (node_ptr)new TimingMsg(0xdeadbeefdeadbee6ULL, 1, 10, 1, 3)} , g);

  boost::add_edge(root, Aa, (myEdge){SYNC, 8}, g);
  boost::add_edge(root, Ab, (myEdge){SYNC, 2000}, g);

  vertex_t B0 = boost::add_vertex((myVertex) {"B0", (node_ptr)new TimeBlock(0xcafebabedeadbee7ULL, true)}, g);
  vertex_t B0a = boost::add_vertex((myVertex) {"Evt_B0_0", (node_ptr)new TimingMsg(0x0000beefdeadbee6ULL, 2, 5, 1, 4)} , g);

  boost::add_edge(root, B0, (myEdge){DEFAULT, 50000}, g);
  boost::add_edge(B0, B0a, (myEdge){SYNC, 7000}, g);


  vertex_t B1 = boost::add_vertex((myVertex) {"B1", (node_ptr)new TimeBlock(0xcafebabedeadbee7ULL, true)}, g);
  vertex_t B1a = boost::add_vertex((myVertex) {"Evt_B1_0", (node_ptr)new TimingMsg(0x0000beefdeadbee6ULL, 2, 5, 1, 4)} , g);

  boost::add_edge(root, B1, (myEdge){STANDARD, 50000}, g);
  boost::add_edge(B1, B1a, (myEdge){SYNC, 300}, g);

  vertex_t C = boost::add_vertex((myVertex) {"C", (node_ptr)new TimeBlock(0xcafebabedeadbee7ULL, true)}, g);
  vertex_t Ca = boost::add_vertex((myVertex) {"Evt_C_0", (node_ptr)new TimingMsg(0xcafebabedeadbee6ULL, 0, 42, 2, 4)} , g);
  vertex_t Cb = boost::add_vertex((myVertex) {"Evt_C_1", (node_ptr)new TimingMsg(0xdeadbeefdeadbee6ULL, 1, 10, 1, 3)} , g);

  boost::add_edge(B0, C, (myEdge){DEFAULT, 8000}, g);
  boost::add_edge(B1, C, (myEdge){STANDARD, 8000}, g);
  boost::add_edge(C, Ca, (myEdge){SYNC, 100}, g);
  boost::add_edge(C, Cb, (myEdge){SYNC, 250}, g);
  boost::add_edge(C, root, (myEdge){STANDARD, 10000}, g);


  vertex_t D = boost::add_vertex((myVertex) {"D", (node_ptr)new TimeBlock(0xcafebabedeadbee7ULL, true)}, g);
  vertex_t Da = boost::add_vertex((myVertex) {"Evt_D_0", (node_ptr)new TimingMsg(0xcafebabedeadbee6ULL, 0, 42, 2, 4)} , g);
  vertex_t Db = boost::add_vertex((myVertex) {"Evt_D_1", (node_ptr)new TimingMsg(0xdeadbeefdeadbee6ULL, 1, 10, 1, 3)} , g);

  boost::add_edge(C, D, (myEdge){DEFAULT, 10000}, g);
  boost::add_edge(D, Da, (myEdge){SYNC, 100}, g);
  boost::add_edge(D, Db, (myEdge){SYNC, 250}, g);

  vertex_t E = boost::add_vertex((myVertex) {"E", (node_ptr)new TimeBlock(0xcafebabedeadbee7ULL, true)}, g);
  vertex_t Ea = boost::add_vertex((myVertex) {"Evt_E_0", (node_ptr)new TimingMsg(0xcafebabedeadbee6ULL, 0, 42, 2, 4)} , g);

  boost::add_edge(D, E, (myEdge){DEFAULT, 100}, g);
  boost::add_edge(E, Ea, (myEdge){SYNC, 100}, g);

  std::ofstream out("./test.dot"); 

  
  Visitor v = Visitor(out);

  //g[Aa].np->accept(v);
  //g[Ab].np->accept(v);


  


  boost::write_graphviz(out, g, make_vertex_writer(boost::get(&myVertex::np, g)), make_edge_writer(boost::get(&myEdge::type, g), boost::get(&myEdge::offs, g)), sample_graph_writer{g[root].name}, boost::get(&myVertex::name, g));

  return 0;	
}





