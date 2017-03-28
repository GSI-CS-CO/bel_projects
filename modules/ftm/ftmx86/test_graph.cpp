#include <boost/shared_ptr.hpp>
#include <stdio.h>
#include <iostream>
#include <inttypes.h>
#include "common.h"
#include "event.h"
#include "timeblock.h"
#include <boost/graph/graphviz.hpp>

#include "propwrite.h"
#include "node.h"
#include "timeblock.h"
#include "event.h"
#include "visitor.h"
















vertex_t add_child_at(std::string name, node_ptr node, vertex_t parent, Graph& g) {
  vertex_t child = boost::add_vertex((myVertex) {name, node}, g);
  boost::add_edge(parent, child, (myEdge) {SYNC, boost::bind(&Node::getTPeriod, g[parent].np), boost::bind(&Node::getTOffs, g[child].np)}, g);
  


  return child;
}

vertex_t connect(vertex_t child, vertex_t parent, Graph& g) {
  boost::add_edge(parent, child, (myEdge) {SYNC, boost::bind(&Node::getTPeriod, g[parent].np), boost::bind(&Node::getTOffs, g[child].np)}, g);

  // parent is an event, traverse parents until we find a block. Connect.
  

  return child;
}

void show_children(vertex_t v, Graph& g) {
Graph::out_edge_iterator out_begin, out_end;
for (boost::tie(out_begin, out_end) = out_edges(v,g); out_begin != out_end; ++out_begin)
{   
    std::cout << g[target(*out_begin,g)].name << std::endl;
}
std::cout << std::endl;
}


void show_parents(vertex_t v, Graph& g) {

Graph::in_edge_iterator in_begin, in_end;
for (boost::tie(in_begin, in_end) = in_edges(v,g); in_begin != in_end; ++in_begin)
{   
    std::cout << g[source(*in_begin,g)].name << std::endl;
}
std::cout << std::endl;
}

/*
vertex_t& getParents(vertex_t v, Graph& g) {

Graph::in_edge_iterator in_begin, in_end;
for (boost::tie(in_begin, in_end) = in_edges(v,g); in_begin != in_end; ++in_begin)
{   
    std::cout << g[source(*in_begin,g)].name << std::endl;
}
std::cout << std::endl;
}
*/


int main() {

  

  boost::graph_traits < boost::adjacency_list <> >::vertex_iterator i, end;

  Graph g;
/*
  vertex_t root = boost::add_vertex((myVertex) {"A", (node_ptr)new TimeBlock(0xcafebabedeadbee7ULL, true)}, g);
  vertex_t Aa = boost::add_vertex((myVertex) {"Evt_A_0", (node_ptr)new TimingMsg(0xcafebabedeadbee6ULL, 0, 42, 2, 4)} , g);
  vertex_t Ab = boost::add_vertex((myVertex) {"Evt_A_1", (node_ptr)new Noop(5000, 1, 10000, 10)} , g);

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

*/
  vertex_t root = boost::add_vertex((myVertex) {"A", (node_ptr) new TimeBlock(5000, true)}, g);
  vertex_t tmp, tmp1, tmp2, tmp3;
  tmp =  add_child_at("Evt_A_0", (node_ptr)new TimingMsg(300, 0, 42, 2, 4), root, g);
  tmp = add_child_at("B0", (node_ptr)new TimeBlock(6000, true), root, g);
  add_child_at("Evt_B_0", (node_ptr)new TimingMsg(300, 0, 42, 2, 4), tmp, g);
  add_child_at("Evt_B_1", (node_ptr)new TimingMsg(400, 0, 42, 2, 4), tmp, g);
  add_child_at("Evt_B_2", (node_ptr)new TimingMsg(0, 0, 42, 2, 4), tmp, g);
  tmp1 = add_child_at("C0", (node_ptr)new TimeBlock(2000, false), tmp, g);
  tmp2 = add_child_at("C1", (node_ptr)new TimeBlock(2000, false), tmp, g);
  tmp = add_child_at("D0", (node_ptr)new TimeBlock(1000, true), tmp2, g);
  tmp3 = add_child_at("Evt_D_0", (node_ptr)new Noop(8, 1, 10000, 10), tmp, g);
  tmp2 = add_child_at("Evt_D_1", (node_ptr)new TimingMsg(100, 0, 42, 2, 4), tmp, g);
  connect(root, tmp2, g);
  connect(tmp, tmp1, g);
  connect(root, tmp, g);
  tmp = add_child_at("E0", (node_ptr)new TimeBlock(9000, false), tmp, g);
  
  std::ofstream out("./test.dot"); 

  
  

  //g[Aa].np->accept(v);
  //g[Ab].np->accept(v);


  


  boost::write_graphviz(out, g, make_vertex_writer(boost::get(&myVertex::np, g)), make_edge_writer(boost::get(&myEdge::getTimeParent, g), boost::get(&myEdge::getTimeChild, g)), sample_graph_writer{g[root].name}, boost::get(&myVertex::name, g));

  show_parents(root, g);
  show_children(root, g);

  //Visitor v = Visitor(out, root, g);

  g[root].np->acceptSerialiser(Visitor(out, root, g)); 


  g[tmp3].np->acceptSerialiser(Visitor(out, tmp3, g)); 

  return 0;	
}





