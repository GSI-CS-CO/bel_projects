#include <boost/shared_ptr.hpp>
#include <algorithm>  
#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <boost/graph/graphviz.hpp>

#include "visitor.h"
#include "common.h"
#include "propwrite.h"
#include "node.h"
#include "block.h"
#include "meta.h"
#include "event.h"

#include "memunit.h"

static void hexDump (const char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
       printf ("%s:\n", desc);

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
  printf ("\n");  
}

static void vHexDump (const char *desc, vBuf pc, int len) {
    int i;
    unsigned char buff[17];

    // Output description if given.
    if (desc != NULL)
       printf ("%s:\n", desc);

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
  printf ("\n");  
}

void all_evt_children(vertex_t v, Graph& g) {
  Graph::out_edge_iterator out_begin, out_end, out_cur;
  boost::tie(out_begin, out_end) = out_edges(v,g);
  std::cout << "Out Edges: " << (vertex_t)(out_end - out_begin) << std::endl;
  for (out_cur = out_begin; out_cur != out_end; ++out_cur)
  {   
    std::cout << "#" << (vertex_t)(out_cur - out_begin) << " -- " << g[*out_cur].type << " -> " << g[target(*out_cur,g)].name;
    if (g[target(*out_cur,g)].np == NULL) std::cout << "(UNDEFINED)" << std::endl;
    else {
      if (g[target(*out_cur,g)].np->isMeta()) std::cout << "(meta)" << std::endl;
      else std::cout << "(real)" << std::endl;
    }
  }
  std::cout << std::endl;
}



int main() {



  // Construct an empty graph and prepare the dynamic_property_maps.
  Graph g;
  boost::dynamic_properties dp(boost::ignore_other_properties);


  dp.property("type",  boost::get(&myEdge::type, g));

  dp.property("node_id", boost::get(&myVertex::name, g));
  dp.property("type",  boost::get(&myVertex::type, g));

  // not sure about this ... this should be several meaningful properties which are then grouped into "flags" field
  dp.property("flags", boost::get(&myVertex::flags, g));

  //Block
  dp.property("tPeriod", boost::get(&myVertex::tPeriod, g));

  // leave out prefilled block cmdq indices and cmdq buffers and for now
  //Events
  dp.property("tOffs",  boost::get(&myVertex::tOffs, g));
  //Timing Message
  dp.property("id",  boost::get(&myVertex::id, g));
  dp.property("par",  boost::get(&myVertex::par, g));
  dp.property("tef",  boost::get(&myVertex::tef, g));
  dp.property("res",  boost::get(&myVertex::res, g));
  //Commands
  dp.property("tValid",  boost::get(&myVertex::tValid, g));

  // Leave out Flush for now
 
  //Flow, Noop
  dp.property("qty",  boost::get(&myVertex::qty, g));

  //Wait
  dp.property("tWait",  boost::get(&myVertex::tWait, g));

 
  std::ifstream in("./try.dot"); 
  std::cout << "T1" << std::endl;
  bool status = boost::read_graphviz(in,g,dp,"node_id");
  std::cout << "T2" << std::endl;
  boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
  boost::tie(vi, vi_end) = vertices(g);
  std::cout << "Size " << vi_end - vi << std::endl;

    std::string cmp; 

  MemUnit mmu = MemUnit(1, 0x4110000, 0x1000000, 0x1000, 65536, g);
  
  BOOST_FOREACH( edge_t e, edges(g) ) {
    std::transform(g[e].type.begin(), g[e].type.end(), g[e].type.begin(), ::tolower);
  }

  BOOST_FOREACH( vertex_t v, vertices(g) ) {
    
    std::transform(g[v].type.begin(), g[v].type.end(), g[v].type.begin(), ::tolower);
    cmp = g[v].type;

    mmu.allocate(g[v].name);

    std::cout << "Test1" << std::endl;

    auto* x = mmu.lookupName(g[v].name);
    if(x == NULL) {std::cerr << "ERROR: Tried to lookup unallocated node " << g[v].name <<  std::endl; return -1;}

    //std::cout << "Type " << g[v].type << std::endl;
    //std::cout << g[v].name << ": @ 0x" << std::hex << x->adr << " # 0x" << x->hash << '\n';


    if      (cmp == "tmsg")     {g[v].np = (node_ptr) new   TimingMsg(g[v].name, x->hash, x->b, g[v].flags, g[v].tOffs, g[v].id, g[v].par, g[v].tef, g[v].res); }
    else if (cmp == "noop")     {g[v].np = (node_ptr) new        Noop(g[v].name, x->hash, x->b, g[v].flags, g[v].tOffs, g[v].tValid, g[v].qty); }
    else if (cmp == "flow")     {std::cerr << "not yet implemented " << g[v].type << std::endl;}
    else if (cmp == "flush")    {std::cerr << "not yet implemented " << g[v].type << std::endl;}
    else if (cmp == "wait")     {std::cerr << "not yet implemented " << g[v].type << std::endl;}
    else if (cmp == "block")    {g[v].np = (node_ptr) new     Block(g[v].name, x->hash, x->b, g[v].flags, g[v].tPeriod ); }
    else if (cmp == "qinfo")    {g[v].np = (node_ptr) new    CmdQMeta(g[v].name, x->hash, x->b, g[v].flags);}
    else if (cmp == "listdst") {g[v].np = (node_ptr) new DestList(g[v].name, x->hash, x->b, g[v].flags);}
    else if (cmp == "qbuf")     {g[v].np = (node_ptr) new  CmdQBuffer(g[v].name, x->hash, x->b, g[v].flags);}
    else if (cmp == "meta")     {std::cerr << "not yet implemented " << g[v].type << std::endl;}

    else                        {std::cerr << "Node type" << cmp << " not supported! " << std::endl;}


    
  }

  vAdr myAdr = vAdr(1);
  myAdr[0] = 0x12345678;



  BOOST_FOREACH( vertex_t v, vertices(g) ) {

    

    if (g[v].np == NULL ){std::cerr << " Node " << g[v].name << " was not initialised! " << g[v].type << std::endl;}
    else {
      //g[v].np->serialise(myAdr, myAdr);
      //std::cout << " *** " << g[v].name << " @ Int 0x" << std::hex << mmu.lookupName(g[v].name)->intAdr << " @ Ext 0x" << mmu.lookupName(g[v].name)->extAdr << " *** " << std::endl;
      g[v].np->accept(VisitorNodeCrawler(v, mmu));
      //hexDump(g[v].name.c_str(), mmu.lookupName(g[v].name)->b, _MEM_BLOCK_SIZE);
      //std::cout << std::endl;
      
      
    }
    //all_evt_children(v, g);
    /*
    auto vDst = getDst(v, g);
    for(auto it = vDst.begin(); it < vDst.end(); it++) {
      if (it - vDst.begin() == 0) std::cout << "defDst: ";
      else std::cout << "altDst: ";
      std::cout << g[*it].name << std::endl;
    }
    */
    
  }
  
  for (auto& it : mmu.getAllChunks()) {
    std::cout << "E@: 0x" << std::hex << mmu.adr2intAdr(it->adr) << " #: 0x" << it->hash << " -> Name: " << mmu.hash2name(it->hash) << std::endl;
    hexDump("", it->b, _MEM_BLOCK_SIZE);
  }
 
  for (auto& it : mmu.getUploadAdrs()) { std::cout << "WR @: 0x" << std::hex << it << std::endl;}

  vBuf data = mmu.getUploadData();


  vHexDump("EB to Transfer", data, data.size()); 


  return 0;
}
