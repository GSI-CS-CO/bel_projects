
#include "scheduleIsomorphism.h"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>

#include "parseSchedule.h"
#include "printSchedule.h"
#include "scheduleCompare.h"

template <typename Graph1>
class iso_callback {
 public:
  iso_callback(const Graph1& graph1, const Graph1& graph2) : graph1_(graph1), graph2_(graph2) {}
  template <typename CorrespondenceMap1To2, typename CorrespondenceMap2To1>
  bool operator()(CorrespondenceMap1To2 f, CorrespondenceMap2To1) {
    BGL_FORALL_VERTICES_T(v, graph1_, Graph1) { vertex_iso_map.emplace_back(get(boost::vertex_index_t(), graph1_, v), get(boost::vertex_index_t(), graph2_, get(f, v))); }
    set_of_vertex_iso_map.push_back(vertex_iso_map);
    vertex_iso_map.clear();
    return false;
  }
  std::vector<std::vector<std::pair<int, int>>> get_setvmap() { return set_of_vertex_iso_map; }

 private:
  const Graph1& graph1_;
  const Graph1& graph2_;
  std::vector<std::vector<std::pair<int, int>>> set_of_vertex_iso_map;
  std::vector<std::pair<int, int>> vertex_iso_map;
};

template <typename Graph1>
// typedef typename boost::graph_traits<typename Graph1>::vertex_descriptor vertex_descriptor_t;
class GraphCompare {
 public:
  GraphCompare(Graph1& graph1, const Graph1& graph2) : graph1_(graph1), graph2_(graph2) {}
  bool operator()(long unsigned int v1, long unsigned int v2) { return graph1_[v1] == graph2_[v2]; }

 private:
  Graph1& graph1_;
  const Graph1& graph2_;
};

/* This is the main method to check for an isomorphism. It contains the call to vf2_subgraph_iso.
 */
int scheduleIsomorphic(std::string dotFile1, std::string dotFile2, configuration& config) {
  ScheduleGraph graph1, graph2;
  boost::dynamic_properties dp1 = setDynamicProperties(graph1);
  bool parse1 = parseSchedule(dotFile1, graph1, dp1, config);
  printSchedule("Graph 1:", graph1, dp1, config);
  boost::dynamic_properties dp2 = setDynamicProperties(graph2);
  bool parse2 = parseSchedule(dotFile2, graph2, dp2, config);
  printSchedule("Graph 2:", graph2, dp2, config);
  if (parse1 && parse2) {
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
    // typedef boost::property_map_equivalent<VertexNameMap, VertexNameMap> vertex_compare_t;
    // vertex_compare_t vertex_compare = make_property_map_equivalent(boost::get(&ScheduleVertex::name, *ref1), boost::get(&ScheduleVertex::name, *ref2));

    // create predicates for edges
    typedef boost::property_map_equivalent<EdgeNameMap, EdgeNameMap> edge_compare_t;
    edge_compare_t edge_compare = make_property_map_equivalent(boost::get(&ScheduleEdge::type, *ref1), get(&ScheduleEdge::type, *ref2));

    // Create callback
    //  boost::vf2_print_callback<ScheduleGraph, ScheduleGraph> callback(*ref1, *ref2);
    iso_callback<ScheduleGraph> callback(*ref1, *ref2);
    GraphCompare<ScheduleGraph> graphComparator(*ref1, *ref2);
    // Print out all subgraph isomorphism mappings between graph1 and graph2.
    // Function vertex_order_by_mult is used to compute the order of
    // vertices of graph1. This is the order in which the vertices are examined
    // during the matching process.
    int result = -1;
    bool isomorphic =
        vf2_subgraph_iso(*ref1, *ref2, std::ref(callback), vertex_order_by_mult(*ref1), boost::vertices_equivalent(std::ref(graphComparator)).edges_equivalent(edge_compare));
    if (num_vertices(*ref1) == num_vertices(*ref2) && num_edges(*ref1) == num_edges(*ref2)) {
      if (!config.silent) {
        std::cout << "Graphs " << getGraphName(*ref1) << " and " << getGraphName(*ref2) << " are " << (isomorphic ? "" : "NOT ") << "isomorphic." << std::endl;
      }
      result = (isomorphic ? EXIT_SUCCESS : NOT_ISOMORPHIC);
    } else {
      if (!config.silent) {
        std::cout << "Graph " << getGraphName(*ref1) << " is " << (isomorphic ? "" : "NOT ") << "isomorphic to a subgraph of graph " << getGraphName(*ref2) << "." << std::endl;
      }
      result = (isomorphic ? SUBGRAPH_ISOMORPHIC : NOT_ISOMORPHIC);
    }

    if (!config.silent) {
      // get vector from callback
      auto set_of_vertex_iso_map = callback.get_setvmap();

      if (set_of_vertex_iso_map.size() > 0) {
        // output vector size here
        std::cout << "Number of isomorphisms: " << set_of_vertex_iso_map.size() << std::endl;
        for (auto set_of_v : set_of_vertex_iso_map) {
          for (auto v : set_of_v) {
            std::cout << "(" << v.first << ", " << v.second << ") {" << (*ref1)[v.first].name << ", " << (*ref2)[v.second].name << "} ";
          }
          std::cout << std::endl;
        }
      }
    }
    return result;
  } else {
    return FILE_NOT_FOUND;
  }
}

boost::dynamic_properties setDynamicProperties(ScheduleGraph& g) {
  boost::dynamic_properties dp(boost::ignore_other_properties);
  boost::ref_property_map<ScheduleGraph*, std::string> gname(boost::get_property(g, boost::graph_name));
  dp.property("name", gname);
  // attributes of vertices
  dp.property("type", boost::get(&ScheduleVertex::type, g));
  dp.property("name", boost::get(&ScheduleVertex::name, g));
  dp.property("tperiod", boost::get(&ScheduleVertex::tperiod, g));
  dp.property("qlo", boost::get(&ScheduleVertex::qlo, g));
  dp.property("qhi", boost::get(&ScheduleVertex::qhi, g));
  dp.property("qil", boost::get(&ScheduleVertex::qil, g));
  dp.property("tef", boost::get(&ScheduleVertex::tef, g));
  dp.property("toffs", boost::get(&ScheduleVertex::toffs, g));
  dp.property("par", boost::get(&ScheduleVertex::par, g));
  dp.property("id", boost::get(&ScheduleVertex::id, g));
  dp.property("fid", boost::get(&ScheduleVertex::fid, g));
  dp.property("gid", boost::get(&ScheduleVertex::gid, g));
  dp.property("evtno", boost::get(&ScheduleVertex::evtno, g));
  dp.property("sid", boost::get(&ScheduleVertex::sid, g));
  dp.property("bpid", boost::get(&ScheduleVertex::bpid, g));
  dp.property("beamin", boost::get(&ScheduleVertex::beamin, g));
  dp.property("bpcstart", boost::get(&ScheduleVertex::bpcstart, g));
  dp.property("reqnobeam", boost::get(&ScheduleVertex::reqnobeam, g));
  dp.property("vacc", boost::get(&ScheduleVertex::vacc, g));
  dp.property("res", boost::get(&ScheduleVertex::res, g));
  // attribute of edges
  dp.property("type", boost::get(&ScheduleEdge::type, g));
  //  dp.property("name", boost::get(&ScheduleEdge::name, g));
  return dp;
}

std::string getGraphName(ScheduleGraph& g) { return boost::get_property(g, boost::graph_name); }
