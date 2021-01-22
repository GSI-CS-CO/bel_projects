//#include <boost/graph/graphviz.hpp>
#include "scheduleIsomorphism.h"

#include <boost/graph/properties.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>

#include "parseSchedule.h"
#include "printSchedule.h"
#include "scheduleCompare.h"

template <typename Graph1, typename Graph2>
class iso_callback {
 public:
  // constructor
  iso_callback(const Graph1& graph1, const Graph2& graph2) : graph1_(graph1), graph2_(graph2) {}
  template < typename CorrespondenceMap1To2, typename CorrespondenceMap2To1 >
  bool operator()(CorrespondenceMap1To2 f, CorrespondenceMap2To1) {
    BGL_FORALL_VERTICES_T(v, graph1_, Graph1) { vertex_iso_map.emplace_back(get(boost::vertex_index_t(), graph1_, v), get(boost::vertex_index_t(), graph2_, get(f, v))); }
    set_of_vertex_iso_map.push_back(vertex_iso_map);
    vertex_iso_map.clear();

    return true;
  }
  /*
      template <typename CorrespondenceMap1To2, typename CorrespondenceMap2To1>
      bool operator()(CorrespondenceMap1To2 f, CorrespondenceMap2To1) {

          BGL_FORALL_VERTICES_T(v, graph1_, Graph1)
              vertex_iso_map.push_back(std::make_pair( get(boost::vertex_index_t(), graph1_, v) , get(boost::vertex_index_t(), graph2_, get(f, v))));
              set_of_vertex_iso_map.push_back(vertex_iso_map);
              vertex_iso_map.clear();

          return true;
      }*/
  std::vector<std::vector<std::pair<int, int>>> get_setvmap() { return set_of_vertex_iso_map; }

 private:
  const Graph1& graph1_;
  const Graph2& graph2_;
  std::vector<std::vector<std::pair<int, int>>> set_of_vertex_iso_map;
  std::vector<std::pair<int, int>> vertex_iso_map;
};

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
  //  boost::vf2_print_callback<ScheduleGraph, ScheduleGraph> callback(*ref1, *ref2);
  iso_callback<ScheduleGraph, ScheduleGraph> callback(*ref1, *ref2);

  // Print out all subgraph isomorphism mappings between graph1 and graph2.
  // Function vertex_order_by_mult is used to compute the order of
  // vertices of graph1. This is the order in which the vertices are examined
  // during the matching process.
  bool ret = vf2_subgraph_iso(*ref1, *ref2, std::ref(callback), vertex_order_by_mult(*ref1), vertices_equivalent(vertex_compare).edges_equivalent(edge_compare));
  std::cout << "ret: " << ret << std::endl;

  // get vector from callback
    auto set_of_vertex_iso_map = callback.get_setvmap();

    // output vector size here
    std::cout << set_of_vertex_iso_map.size() << std::endl;
    for (auto set_of_v : set_of_vertex_iso_map)
    {
        for (auto v : set_of_v)
            std::cout << "(" << v.first << ", " << v.second << ")" << " ";
        std::cout << std::endl;
    }
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
