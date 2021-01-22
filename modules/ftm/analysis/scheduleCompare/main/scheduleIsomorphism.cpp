//#include <boost/graph/graphviz.hpp>
#include "scheduleIsomorphism.h"

#include <boost/graph/properties.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>

#include "parseSchedule.h"
#include "printSchedule.h"
#include "scheduleCompare.h"

bool scheduleIsomorphic(std::string dotFile1, std::string dotFile2, configuration& config) {
  ScheduleGraph graph1, graph2;
  boost::dynamic_properties dp1 = setDynamicProperties(graph1);
  parseSchedule(dotFile1, graph1, dp1, config);
  printSchedule("Graph 1:", graph1, dp1, config);
  boost::dynamic_properties dp2 = setDynamicProperties(graph2);
  parseSchedule(dotFile2, graph2, dp2, config);
  printSchedule("Graph 2:", graph2, dp2, config);

  // Use the smaller graph as graph1.
  ScheduleGraph *ref1, *ref2;
  if (num_vertices(graph1) > num_vertices(graph2)) {
    ref1 = &graph2;
    ref2 = &graph1;
  } else {
    ref1 = &graph1;
    ref2 = &graph2;
  }

  // create predicates for vertices
  typedef boost::property_map_equivalent<VertexNameMap, VertexNameMap> vertex_compare_t;
  vertex_compare_t vertex_compare = make_property_map_equivalent(boost::get(&ScheduleVertex::name, *ref1), boost::get(&ScheduleVertex::name, *ref2));
  // create predicates for edges
  typedef boost::property_map_equivalent<EdgeNameMap, EdgeNameMap> edge_compare_t;
  edge_compare_t edge_compare = make_property_map_equivalent(boost::get(&ScheduleEdge::name, *ref1), get(&ScheduleEdge::name, *ref2));

  // Create callback
  boost::vf2_print_callback<ScheduleGraph, ScheduleGraph> callback(*ref1, *ref2);

  // Print out all subgraph isomorphism mappings between graph1 and graph2.
  // Function vertex_order_by_mult is used to compute the order of
  // vertices of graph1. This is the order in which the vertices are examined
  // during the matching process.
  bool ret = vf2_subgraph_iso(*ref1, *ref2, callback, vertex_order_by_mult(*ref1), vertices_equivalent(vertex_compare).edges_equivalent(edge_compare));
  std::cout << "ret: " << ret << std::endl;
  return ret;
}

boost::dynamic_properties setDynamicProperties(ScheduleGraph& g) {
  boost::dynamic_properties dp(boost::ignore_other_properties);
  boost::ref_property_map<ScheduleGraph*, std::string> gname(boost::get_property(g, boost::graph_name));
  dp.property("name", gname);
  dp.property("type", boost::get(&ScheduleVertex::type, g));
  dp.property("name", boost::get(&ScheduleVertex::name, g));
  dp.property("type", boost::get(&ScheduleEdge::type, g));
  //  dp.property("name", boost::get(&ScheduleEdge::name, g));
  return dp;
}
