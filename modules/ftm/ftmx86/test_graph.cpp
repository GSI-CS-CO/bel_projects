#include <boost/shared_ptr.hpp>
#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <boost/graph/graphviz.hpp>

#include "common.h"
#include "propwrite.h"
#include "node.h"
#include "meta.h"
#include "event.h"
#include "visitor.h"
#include "memunit.h"




static void hexDump (const char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
       printf ("\n%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
               printf ("  %s\n", buff);

            // Output the offset.
           printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
       printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
}








/*


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
*/

void all_evt_children(vertex_t v, Graph& g) {
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  boost::tie(out_begin, out_end) = out_edges(v,g);
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
      //std::cout << g[target(*out_cur,g)].name << " x " << (vertex_t)(out_cur - out_begin) << std::endl;
      std::cout << "Edge Type " << g[*out_cur].type << std::endl;
      
  }
  std::cout << std::endl;
  }

void all_evt_children(vertex_t v, Graph& g) {
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  boost::tie(out_begin, out_end) = out_edges(v,g);
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
      std::cout << g[target(*out_cur,g)].name << " x " << (vertex_t)(out_cur - out_begin) << std::endl;
      
  }
  std::cout << std::endl;
  }


vertex_t& getParents(vertex_t v, Graph& g) {

Graph::in_edge_iterator in_begin, in_end;
for (boost::tie(in_begin, in_end) = in_edges(v,g); in_begin != in_end; ++in_begin)
{   
    std::cout << g[source(*in_begin,g)].name << std::endl;
}
std::cout << std::endl;
}


int main() {

  

  boost::graph_traits < boost::adjacency_list <> >::vertex_iterator i, end;

  Graph g;
/*
  vertex_t root = boost::add_vertex((myVertex) {"A", (node_ptr)new Block(0xcafebabedeadbee7ULL, true)}, g);
  vertex_t Aa = boost::add_vertex((myVertex) {"Evt_A_0", (node_ptr)new TimingMsg(0xcafebabedeadbee6ULL, 0, 42, 2, 4)} , g);
  vertex_t Ab = boost::add_vertex((myVertex) {"Evt_A_1", (node_ptr)new Noop(5000, 1, 10000, 10)} , g);

  boost::add_edge(root, Aa, (myEdge){SYNC, 8}, g);
  boost::add_edge(root, Ab, (myEdge){SYNC, 2000}, g);

  vertex_t B0 = boost::add_vertex((myVertex) {"B0", (node_ptr)new Block(0xcafebabedeadbee7ULL, true)}, g);
  vertex_t B0a = boost::add_vertex((myVertex) {"Evt_B0_0", (node_ptr)new TimingMsg(0x0000beefdeadbee6ULL, 2, 5, 1, 4)} , g);

  boost::add_edge(root, B0, (myEdge){DEFAULT, 50000}, g);
  boost::add_edge(B0, B0a, (myEdge){SYNC, 7000}, g);


  vertex_t B1 = boost::add_vertex((myVertex) {"B1", (node_ptr)new Block(0xcafebabedeadbee7ULL, true)}, g);
  vertex_t B1a = boost::add_vertex((myVertex) {"Evt_B1_0", (node_ptr)new TimingMsg(0x0000beefdeadbee6ULL, 2, 5, 1, 4)} , g);

  boost::add_edge(root, B1, (myEdge){STANDARD, 50000}, g);
  boost::add_edge(B1, B1a, (myEdge){SYNC, 300}, g);

  vertex_t C = boost::add_vertex((myVertex) {"C", (node_ptr)new Block(0xcafebabedeadbee7ULL, true)}, g);
  vertex_t Ca = boost::add_vertex((myVertex) {"Evt_C_0", (node_ptr)new TimingMsg(0xcafebabedeadbee6ULL, 0, 42, 2, 4)} , g);
  vertex_t Cb = boost::add_vertex((myVertex) {"Evt_C_1", (node_ptr)new TimingMsg(0xdeadbeefdeadbee6ULL, 1, 10, 1, 3)} , g);

  boost::add_edge(B0, C, (myEdge){DEFAULT, 8000}, g);
  boost::add_edge(B1, C, (myEdge){STANDARD, 8000}, g);
  boost::add_edge(C, Ca, (myEdge){SYNC, 100}, g);
  boost::add_edge(C, Cb, (myEdge){SYNC, 250}, g);
  boost::add_edge(C, root, (myEdge){STANDARD, 10000}, g);


  vertex_t D = boost::add_vertex((myVertex) {"D", (node_ptr)new Block(0xcafebabedeadbee7ULL, true)}, g);
  vertex_t Da = boost::add_vertex((myVertex) {"Evt_D_0", (node_ptr)new TimingMsg(0xcafebabedeadbee6ULL, 0, 42, 2, 4)} , g);
  vertex_t Db = boost::add_vertex((myVertex) {"Evt_D_1", (node_ptr)new TimingMsg(0xdeadbeefdeadbee6ULL, 1, 10, 1, 3)} , g);

  boost::add_edge(C, D, (myEdge){DEFAULT, 10000}, g);
  boost::add_edge(D, Da, (myEdge){SYNC, 100}, g);
  boost::add_edge(D, Db, (myEdge){SYNC, 250}, g);

  vertex_t E = boost::add_vertex((myVertex) {"E", (node_ptr)new Block(0xcafebabedeadbee7ULL, true)}, g);
  vertex_t Ea = boost::add_vertex((myVertex) {"Evt_E_0", (node_ptr)new TimingMsg(0xcafebabedeadbee6ULL, 0, 42, 2, 4)} , g);

  boost::add_edge(D, E, (myEdge){DEFAULT, 100}, g);
  boost::add_edge(E, Ea, (myEdge){SYNC, 100}, g);

*/
  MemUnit mmu = MemUnit(1, 0x1000, 8192, g);
  mmu.allocate("Matze");
  mmu.allocate("Hund");
  mmu.allocate("Katze");
  mmu.allocate("Maus");

  //vertex_t vCur, vPrev; = boost::add_vertex((myVertex) {myName, }, g);

  vertex_t tmpCur, tmpPrev;
  bool firstRun = true;

  std::string myName = "Matze";
  uint32_t myHash = 1324;

  uint8_t testme[_MEM_BLOCK_SIZE];

  for (auto& x : mmu.allocMap) {
    std::cout << x.first << ": @ 0x" << std::hex << x.second.adr << " # 0x" << x.second.hash << '\n';
    if(firstRun) {
      tmpPrev = boost::add_vertex((myVertex) {x.first, x.second.hash}, g);
      g[tmpPrev].np = (node_ptr) new TimingMsg(x.first, x.second.hash, x.second.b, 0x0, 1234ULL, 0x1deadbeefULL, 0x2cafebabeULL, 0xf, 0xe);
    }  
    else         {
      tmpCur  = boost::add_vertex((myVertex) {x.first, x.second.hash}, g);
      g[tmpCur].np = (node_ptr) new TimingMsg(x.first, x.second.hash, x.second.b, 0x0, 1234ULL, 0x1deadbeefULL, 0x2cafebabeULL, 0xf, 0xe);
      boost::add_edge(tmpPrev, tmpCur, (myEdge) {"testedge"}, g);
      tmpPrev = tmpCur;  
    }  
    firstRun = false;
  }





/*
  vertex_t root = boost::add_vertex((myVertex) {"Abe", }, g);
  g[root].hash = FnvHash::fnvHash(x.first.c_str());
  g[root].np = (node_ptr) new Block(g[root].name, g[root].hash, 0x0, 5000);

  vertex_t tmp = boost::add_vertex((myVertex) {"Bob"}, g);
  g[tmp].hash = FnvHash::fnvHash(g[tmp].name.c_str());
  g[tmp].np = (node_ptr) new TimingMsg(g[tmp].name, g[tmp].hash, 0x0, 1234, 0x1deadbeef, 0x2cafebabe, 0xf, 0xe);
  boost::add_edge(root, tmp, (myEdge) {"testedge"}, g);
  /*
  vertex_t tmp, tmp1, tmp2, tmp3;
  tmp =  add_child_at("Evt_A_0", (node_ptr)new TimingMsg(300, 0, 40, 2, 4), root, g);

  tmp =  add_child_at("Evt_A_1", (node_ptr)new TimingMsg(400, 0, 41, 2, 4), tmp, g);
  tmp =  add_child_at("Evt_A_2", (node_ptr)new TimingMsg(600, 0, 42, 2, 4), tmp, g);
  tmp =  add_child_at("Evt_A_3", (node_ptr)new TimingMsg(700, 0, 43, 2, 4), tmp, g);
  tmp = add_child_at("B0", (node_ptr)new Block(6000, true), root, g);
  add_child_at("Evt_B_0", (node_ptr)new TimingMsg(300, 0, 42, 2, 4), tmp, g);
  add_child_at("Evt_B_1", (node_ptr)new TimingMsg(400, 0, 42, 2, 4), tmp, g);
  add_child_at("Evt_B_2", (node_ptr)new TimingMsg(0, 0, 42, 2, 4), tmp, g);
  tmp1 = add_child_at("C0", (node_ptr)new Block(2000, false), tmp, g);
  tmp2 = add_child_at("C1", (node_ptr)new Block(2300, false), tmp, g);
  tmp = add_child_at("D0", (node_ptr)new Block(1000, true), tmp2, g);
  tmp3 = add_child_at("Evt_D_0", (node_ptr)new Noop(8, 1, 10000, 10), tmp, g);
  tmp2 = add_child_at("Evt_D_1", (node_ptr)new TimingMsg(100, 0, 42, 2, 4), tmp, g);
  connect(root, tmp2, g);
  connect(tmp, tmp1, g);
  connect(root, tmp, g);
  connect(root, tmp3, g);
  tmp = add_child_at("E0", (node_ptr)new Block(9000, false), tmp, g);
  */
  std::ofstream out("./test.dot"); 

  
  

  //g[Aa].np->accept(v);
  //g[Ab].np->accept(v);


  boost::default_writer dw; 
  boost::write_graphviz(out, g, make_vertex_writer(boost::get(&myVertex::np, g)), dw, sample_graph_writer{"Matze"}, boost::get(&myVertex::name, g));
  //boost::write_graphviz(out, g, make_vertex_writer(boost::get(&myVertex::np, g)), );
  

  //vBuf myVBuf(8192);


  //boost::container::vector<vBuf> 
  
  


  //g[root].np->accept(VisitorCreateMemBlock(root, g, myVBuf)); 
  vAdr myAdr = vAdr(1);
  myAdr[0] = 0x12345678;

  BOOST_FOREACH( vertex_t v, vertices(g) ) {
    g[v].np->serialise(myAdr, myAdr);
    hexDump(g[v].name.c_str(), mmu.allocMap.at(g[v].name).b, _MEM_BLOCK_SIZE);
  } 
  
  return 0;	
}





