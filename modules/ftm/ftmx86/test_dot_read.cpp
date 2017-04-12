#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>  
#include <stdio.h>
#include <iostream>
#include <inttypes.h>
#include "common.h"
#include <boost/graph/graphviz.hpp>


#include "propwrite.h"

#include "node.h"
#include "timeblock.h"
#include "event.h"
#include "visitor.h"





int main() {



  // Construct an empty graph and prepare the dynamic_property_maps.
  Graph g;
  boost::dynamic_properties dp(boost::ignore_other_properties);
  dp.property("node_id", boost::get(&myVertex::name, g));
  dp.property("type",  boost::get(&myVertex::type, g));

  dp.property("type",  boost::get(&myEdge::type, g));

  //TODO replace the following hack with overloading read_graphviz with a version containing a factory for node objects
  dp.property("tPeriod", boost::get(&myVertex::tPeriod, g));
  dp.property("tOffs",  boost::get(&myVertex::tOffs, g));
  dp.property("tValid",  boost::get(&myVertex::tValid, g));
  dp.property("flags", boost::get(&myVertex::flags, g));
  dp.property("qty",  boost::get(&myVertex::qty, g));
  dp.property("id",  boost::get(&myVertex::id, g));
  dp.property("par",  boost::get(&myVertex::par, g));
  dp.property("tef",  boost::get(&myVertex::tef, g));


  std::ifstream in("./try.dot"); 
  std::cout << "T1" << std::endl;
  bool status = boost::read_graphviz(in,g,dp,"node_id");
  std::cout << "T2" << std::endl;
  boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
  boost::tie(vi, vi_end) = vertices(g);
  std::cout << "Size " << vi_end - vi << std::endl;

    std::string cmp; 

  BOOST_FOREACH( vertex_t v, vertices(g) ) {
    cmp = g[v].type;
    boost::algorithm::to_lower(cmp);

    if      (cmp == "block"    ){g[v].np = (node_ptr)new TimeBlock(g[v].name, g[v].flags, g[v].tPeriod); std::cout << "Type " << g[v].type << std::endl;}
    else if (cmp == "tmsg"     ){g[v].np = (node_ptr)new TimingMsg(g[v].name, g[v].flags, g[v].tOffs, g[v].id, g[v].par, g[v].tef); std::cout << "Type " << g[v].type << std::endl;}
    else if (cmp == "cflow"    ){g[v].np = (node_ptr)new Flow(g[v].name, g[v].flags, g[v].tOffs, g[v].tValid, g[v].qty); std::cout << "Type " << g[v].type << std::endl;}
    else if (cmp == "cflush"   ){g[v].np = (node_ptr)new Flush(g[v].name, g[v].flags, g[v].tOffs, g[v].tValid, g[v].flushIl, g[v].flushHi, g[v].flushLo, g[v].upToHi, g[v].upToLo); std::cout << "Type " << g[v].type << std::endl;}
    else if (cmp == "cnop"     ){g[v].np = (node_ptr)new Noop(g[v].name, g[v].flags, g[v].tOffs, g[v].tValid, g[v].qty); std::cout << "Type " << g[v].type << std::endl;}
    else if (cmp == "queue"    ){std::cerr << "not yet implemented " << g[v].type << std::endl;}
    else if (cmp == "destinfo" ){std::cerr << "not yet implemented " << g[v].type << std::endl;}
    else if (cmp == "queuebuf" ){std::cerr << "not yet implemented " << g[v].type << std::endl;}
    else if (cmp == "meta"     ){std::cerr << "not yet implemented " << g[v].type << std::endl;}
    else {std::cerr << "Cannot determine node type" << g[v].type << std::endl;}


    
  }

  BOOST_FOREACH( vertex_t v, vertices(g) ) {
    if (g[v].np == NULL ){std::cerr << " Node " << g[v].name << " was not initialised! " << g[v].type << std::endl;}
    else {g[v].np->show();}


    
  }

  


  return 0;
}
