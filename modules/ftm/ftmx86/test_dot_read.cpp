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


#include "memunit.h"



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
  dp.property("id",   boost::get(&myVertex::id, g));
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
  bool status = boost::read_graphviz(in,g,dp,"node_id");
  boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
  boost::tie(vi, vi_end) = vertices(g);
  std::cout << "Size " << vi_end - vi << std::endl;

 
  //format all graph labels lowercase
    BOOST_FOREACH( edge_t e, edges(g) ) {
    std::transform(g[e].type.begin(), g[e].type.end(), g[e].type.begin(), ::tolower);
  }

  BOOST_FOREACH( vertex_t v, vertices(g) ) {
    std::transform(g[v].type.begin(), g[v].type.end(), g[v].type.begin(), ::tolower);
  }

  //create memory manager
  MemUnit mmu = MemUnit(1, 0x4110000, 0x1000000, 0x1000, 65536, g);
  
  //analyse and serialise
  mmu.prepareUpload(); 

  //show structure
  for (auto& it : mmu.getAllChunks()) {
    std::cout << "E@: 0x" << std::hex << mmu.adr2extAdr(it->adr) << " #: 0x" << it->hash << " -> Name: " << mmu.hash2name(it->hash) << std::endl;
    hexDump("", it->b, _MEM_BLOCK_SIZE);
  }
  
  //show results
  //for (auto& it : mmu.getUploadAdrs()) { std::cout << "WR @: 0x" << std::hex << it << std::endl;}
  vBuf data = mmu.getUploadData();
  vHexDump("EB to Transfer", data, data.size()); 
  
  mmu.parseDownloadData(data);

  std::ofstream out("./download.dot"); 
  boost::default_writer dw; 
  boost::write_graphviz(out, mmu.getDownGraph(), make_vertex_writer(boost::get(&myVertex::np, g)), dw, sample_graph_writer{"Matze"}, boost::get(&myVertex::name, mmu.getDownGraph()));

  return 0;
}
