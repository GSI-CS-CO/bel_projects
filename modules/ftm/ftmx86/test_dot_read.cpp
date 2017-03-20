

#include <boost/graph/graphviz.hpp>
//#include <boost/graph/src/read_graphviz_new.cpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <sstream>
#include <stdint.h>

using namespace boost;
using namespace std;

//
// Create a custom graph properties
//  (see the documentation for adjacency list)
struct graph_identifier_t { typedef graph_property_tag kind; };
struct vertex_label_t { typedef vertex_property_tag kind; };
struct vertex_prop0_t { typedef vertex_property_tag kind; };
struct vertex_64b_t { typedef vertex_property_tag kind; };
struct vertex_32b_t { typedef vertex_property_tag kind; };
struct vertex_16b_t { typedef vertex_property_tag kind; };
struct vertex_8b_t { typedef vertex_property_tag kind; };

int main() {

  // Vertex properties
  typedef property < vertex_name_t, string,
            property < vertex_label_t, string,
              property < vertex_prop0_t, string,
                property < vertex_64b_t, uint64_t,
                  property < vertex_32b_t, uint32_t,
                    property < vertex_16b_t, uint16_t,
                      property < vertex_8b_t, uint8_t,
                        property < vertex_root_t, int > > > > > > > >vertex_p;  
  // Edge properties
  typedef property < edge_name_t, string > edge_p;
  // Graph properties
  typedef property < graph_name_t, string,
            property < graph_identifier_t, string > > graph_p;
  // adjacency_list-based type
  typedef adjacency_list < vecS, vecS, directedS,
    vertex_p, edge_p, graph_p > graph_t;

  // Construct an empty graph and prepare the dynamic_property_maps.
  graph_t graph(0);
  dynamic_properties dp;

  property_map<graph_t, vertex_name_t>::type vname =
    get(vertex_name, graph);
  dp.property("node_id",get(vertex_name, graph));

  property_map<graph_t, vertex_label_t>::type vlabel =
    get(vertex_label_t(), graph);
  dp.property("label",vlabel);

   property_map<graph_t, edge_name_t>::type elabel =
    get(edge_name, graph);
  dp.property("label",elabel);

  // Use ref_property_map to turn a graph property into a property map
  ref_property_map<graph_t*,string> 
    gname(get_property(graph,graph_name));
  dp.property("name",gname);

    property_map<graph_t, vertex_root_t>::type root =
    get(vertex_root, graph);
  dp.property("root",root);


  // Use ref_property_map to turn a graph property into a property map
  ref_property_map<graph_t*,string> 
    gid(get_property(graph,graph_identifier_t()));
  dp.property("identifier",gid);
  // Sample graph as an istream;

  //Command properties 

  //Event properties 
  dp.property("evt_time_offs",  get(vertex_64b_t(), graph));
  dp.property("evt_tm_par",     get(vertex_64b_t(), graph));
  dp.property("evt_tm_tef",     get(vertex_32b_t(), graph));
  dp.property("evt_tm_res",     get(vertex_32b_t(), graph));
  dp.property("evt_tm_id",      get(vertex_64b_t(), graph));
  dp.property("evt_tm_id_fid",  get(vertex_16b_t(), graph));
  dp.property("evt_tm_id_gid",  get(vertex_16b_t(), graph));
  dp.property("evt_tm_id_eno",  get(vertex_16b_t(), graph));
  dp.property("evt_tm_id_sid",  get(vertex_16b_t(), graph));
  dp.property("evt_tm_id_bpid", get(vertex_16b_t(), graph));
  dp.property("evt_tm_id_res",  get(vertex_16b_t(), graph));
  //Block properties 
  dp.property("block_time_period",get(vertex_64b_t(), graph));
  dp.property("block_has_cmdq", (property_map<graph_t, vertex_8b_t>::type)get(vertex_8b_t(), graph));



const char* dot = 
"digraph \
{ \
  graph [name=\"GRAPH\", identifier=\"CX2A1Z\"] \
    \
    a [label=\"NODE_A\", root=\"1\", myprop0=\"sad\", block_has_cmdq=1] \
    b [label=\"NODE_B\", root=\"0\", myprop0=\"stupid\", evt_time_offs=2, evt_tm_par=10] \
    c [label=\"NODE_C\", root=\"0\"] \
    d [label=\"NODE_D\", root=\"0\", evt_time_offs=42] \
 \
 a -> b [label=\"EDGE_1\"] \
 b -> c [label=\"EDGE_2\"] \
}";


  istringstream gvgraph(dot);

  bool status = read_graphviz(gvgraph,graph,dp,"node_id");

  cout << "graph " << get("name",dp,&graph) <<
      " (" << get("identifier",dp,&graph) << ")\n\n";

  BOOST_FOREACH( graph_t::vertex_descriptor v, vertices(graph) ) {
    cout << "vertex " << get("node_id",dp,v) << " myprop0 " << get("myprop0",dp,v) << " evt_time_offs " << get("evt_time_offs",dp,v) << " evt_time_offs " << get("evt_time_offs",dp,v) << " block_has_cmdq " << get("block_has_cmdq",dp,v) <<
      " (" << get("label",dp,v) << ")\n";
  }

  return 0;
}
