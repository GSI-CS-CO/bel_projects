
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
    BGL_FORALL_VERTICES_T(v, graph1_, Graph1) {
      vertex_iso_map.emplace_back(get(boost::vertex_index_t(), graph1_, v), get(boost::vertex_index_t(), graph2_, get(f, v)));
    }
    set_of_vertex_iso_map.push_back(vertex_iso_map);
    vertex_iso_map.clear();
    return false;
  }
  std::vector<std::vector<std::pair<int, int>>> get_setvmap() {
    return set_of_vertex_iso_map;
  }

 private:
  const Graph1& graph1_;
  const Graph1& graph2_;
  std::vector<std::pair<int, int>> vertex_iso_map;
  std::vector<std::vector<std::pair<int, int>>> set_of_vertex_iso_map;
};

template <typename Graph1>
void printIsomorphisms(std::ostream& out, std::vector<std::vector<std::pair<int, int>>>& set_of_vertex_iso_map, const Graph1& graph1, const Graph1& graph2, bool verbose) {
  for (auto isomorphism : set_of_vertex_iso_map) {
    for (auto vertex_pair : isomorphism) {
      out << "(" << vertex_pair.first << ", " << vertex_pair.second << ") ";
      if (verbose) {
        out << "{" << (graph1)[vertex_pair.first].name << ", " << (graph2)[vertex_pair.second].name << "} ";
      }
    }
    out << std::endl;
  }
}

template <typename Graph1>
class GraphCompare {
 public:
  GraphCompare(Graph1& graph1, const Graph1& graph2) : graph1_(graph1), graph2_(graph2) {}
  bool operator()(long unsigned int v1, long unsigned int v2) {
    //~ std::cout << "GraphCompare: " << v1 << ", " << v2 << std::endl;
    return graph1_[v1] == graph2_[v2];
  }

 private:
  Graph1& graph1_;
  const Graph1& graph2_;
};

template <typename Graph1>
class EdgeCompare {
 public:
  EdgeCompare(Graph1& graph1, const Graph1& graph2) : graph1_(graph1), graph2_(graph2) {}
  using EG1 = typename Graph1::edge_descriptor;
  bool operator()(EG1 e1, EG1 e2) {
    //~ std::cout << "EdgeCompare: " << e1 << ", " << graph1_[e1] << ", " << e2 << ", " << graph2_[e2] << std::endl;
    return graph1_[e1] == graph2_[e2];
  }

 private:
  Graph1& graph1_;
  const Graph1& graph2_;
};

/* This is the main method to check for an isomorphism. It contains the call to vf2_subgraph_iso.
 */
int scheduleIsomorphic(std::string dotFile1, std::string dotFile2, configuration& config) {
  ScheduleGraph graph1, graph2;
  bool parse1 = false;
  bool parse2 = false;
  int result = -1;
  try {
    boost::dynamic_properties dp1 = setDynamicProperties(graph1, config);
    parse1 = parseSchedule(dotFile1, graph1, dp1, config);
    printSchedule("Graph 1:", graph1, dp1, config);
  } catch (boost::property_not_found &excep) {
    std::cerr << "Parsing graph1: Property not found" << excep.what() << std::endl;
    result = PARSE_ERROR;
  } catch (boost::bad_graphviz_syntax &excep) {
    std::cerr << "Parsing graph1: Bad Graphviz syntax: " << excep.what() << std::endl;
    result = PARSE_ERROR_GRAPHVIZ;
  }
  try {
    boost::dynamic_properties dp2 = setDynamicProperties(graph2, config);
    parse2 = parseSchedule(dotFile2, graph2, dp2, config);
    printSchedule("Graph 2:", graph2, dp2, config);
  } catch (boost::property_not_found &excep) {
    std::cerr << "Parsing graph2: Property not found" << excep.what() << std::endl;
    result = PARSE_ERROR;
  } catch (boost::bad_graphviz_syntax &excep) {
    std::cerr << "Parsing graph2: Bad Graphviz syntax: " << excep.what() << std::endl;
    result = PARSE_ERROR_GRAPHVIZ;
  }
  // std::cerr << "parse1: " << parse1 << ", parse2: " << parse2 << ", result: " << result << std::endl;
  if (parse1 && parse2) {
    // Use the smaller graph as graph1.
    ScheduleGraph *ref1, *ref2;
    std::string *refName1, *refName2;
    if (num_vertices(graph1) > num_vertices(graph2)) {
      ref1 = &graph2;
      refName1 = &dotFile2;
      ref2 = &graph1;
      refName2 = &dotFile1;
    } else {
      ref1 = &graph1;
      refName1 = &dotFile1;
      ref2 = &graph2;
      refName2 = &dotFile2;
    }

    // create predicates for edges
    //~ typedef boost::property_map_equivalent<EdgeNameMap, EdgeNameMap> edge_compare_t;
    //~ edge_compare_t edge_compare = make_property_map_equivalent(boost::get(&ScheduleEdge::type, *ref1), get(&ScheduleEdge::type, *ref2));
    // Alternative:
    EdgeCompare<ScheduleGraph> edgeComparator(*ref1, *ref2);

    // Create callback
    iso_callback<ScheduleGraph> callback(*ref1, *ref2);
    // create predicates for vertices
    GraphCompare<ScheduleGraph> graphComparator(*ref1, *ref2);
    // Print out all subgraph isomorphism mappings between graph1 and graph2.
    // Function vertex_order_by_mult is used to compute the order of
    // vertices of graph1. This is the order in which the vertices are examined
    // during the matching process.
    bool isomorphic = vf2_subgraph_iso(*ref1,                           // const GraphSmall& graph_small
                          *ref2,                                        // const GraphLarge& graph_large,
                          std::ref(callback),                           // SubGraphIsoMapCallback user_callback,
                          get(boost::vertex_index, *ref1),              // IndexMapSmall index_map_small,
                          get(boost::vertex_index, *ref2),              // IndexMapLarge index_map_large,
                          vertex_order_by_mult(*ref1),                  // const VertexOrderSmall& vertex_order_small,
                          std::ref(edgeComparator),                     // EdgeEquivalencePredicate edge_comp,
                          std::ref(graphComparator));                   // VertexEquivalencePredicate vertex_comp)
    if (num_vertices(*ref1) == num_vertices(*ref2) && num_edges(*ref1) == num_edges(*ref2)) {
      if (!config.silent) {
        std::cout << "Graphs " << getGraphName(*ref1) << " (" << *refName1 << ") and " << getGraphName(*ref2) << " (" << *refName2 << ") are " << (isomorphic ? "" : "NOT ")
                  << "isomorphic." << std::endl;
      }
      if (config.verbose) {
        listVertexProtocols(*ref1);
      }
      result = (isomorphic ? EXIT_SUCCESS : NOT_ISOMORPHIC);
    } else {
      if (!config.silent) {
        std::cout << "Graph " << getGraphName(*ref1) << " (" << *refName1 << ") is " << (isomorphic ? "" : "NOT ") << "isomorphic to a subgraph of graph " << getGraphName(*ref2)
                  << " (" << *refName2 << ")." << std::endl;
      }
      result = (isomorphic ? SUBGRAPH_ISOMORPHIC : NOT_ISOMORPHIC);
    }

    if (!config.silent) {
      // get vector from callback
      auto set_of_isomorphisms = callback.get_setvmap();

      if (set_of_isomorphisms.size() > 0) {
        // output vector size here
        std::cout << "Number of isomorphisms: " << set_of_isomorphisms.size() << std::endl;
        printIsomorphisms(std::cout, set_of_isomorphisms, *ref1, *ref2, config.superverbose);
      }
    }
    return result;
  } else {
    return (result == -1) ? FILE_NOT_FOUND : result;
  }
}

boost::dynamic_properties setDynamicProperties(ScheduleGraph& g, configuration& config) {
  boost::dynamic_properties dp = boost::dynamic_properties(boost::ignore_other_properties);
  if (config.check) {
    dp = boost::dynamic_properties();
    dp.property("cpu", boost::get(&ScheduleVertex::cpu, g));
    dp.property("qty", boost::get(&ScheduleVertex::qty, g));
    dp.property("vabs", boost::get(&ScheduleVertex::vabs, g));
    dp.property("flags", boost::get(&ScheduleVertex::flags, g));
    dp.property("shape", boost::get(&ScheduleVertex::shape, g));
    dp.property("penwidth", boost::get(&ScheduleVertex::penwidth, g));
    dp.property("fillcolor", boost::get(&ScheduleVertex::fillcolor, g));
    dp.property("color", boost::get(&ScheduleVertex::color, g));
    dp.property("style", boost::get(&ScheduleVertex::style, g));
    dp.property("color", boost::get(&ScheduleEdge::color, g));
  }
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
  dp.property("tvalid", boost::get(&ScheduleVertex::tvalid, g));
  dp.property("tabs", boost::get(&ScheduleVertex::tabs, g));
  dp.property("target", boost::get(&ScheduleVertex::target, g));
  dp.property("dst", boost::get(&ScheduleVertex::dst, g));
  dp.property("reps", boost::get(&ScheduleVertex::reps, g));
  dp.property("prio", boost::get(&ScheduleVertex::prio, g));
  dp.property("twait", boost::get(&ScheduleVertex::twait, g));
  dp.property("wabs", boost::get(&ScheduleVertex::wabs, g));
  dp.property("clear", boost::get(&ScheduleVertex::clear, g));
  dp.property("ovr", boost::get(&ScheduleVertex::ovr, g));
  dp.property("beamproc", boost::get(&ScheduleVertex::beamproc, g));
  dp.property("pattern", boost::get(&ScheduleVertex::pattern, g));
  dp.property("patentry", boost::get(&ScheduleVertex::patentry, g));
  dp.property("patexit", boost::get(&ScheduleVertex::patexit, g));
  dp.property("bpentry", boost::get(&ScheduleVertex::bpentry, g));
  dp.property("bpexit", boost::get(&ScheduleVertex::bpexit, g));
  // attribute of edges
  dp.property("type", boost::get(&ScheduleEdge::type, g));
  //  dp.property("name", boost::get(&ScheduleEdge::name, g));
  return dp;
}

std::string getGraphName(ScheduleGraph& g) { return boost::get_property(g, boost::graph_name); }

int testSingleGraph(std::string dotFile1, configuration& config) {
  ScheduleGraph graph1;
  bool parse1 = false;
  int result = -1;
  try {
    boost::dynamic_properties dp1 = setDynamicProperties(graph1, config);
    parse1 = parseSchedule(dotFile1, graph1, dp1, config);
    printSchedule("Graph:", graph1, dp1, config);
  } catch (boost::property_not_found &excep) {
    std::cerr << "Parsing graph: Property not found" << excep.what() << std::endl;
    result = PARSE_ERROR;
  } catch (boost::bad_graphviz_syntax &excep) {
    std::cerr << "Parsing graph: Bad Graphviz syntax: " << excep.what() << std::endl;
    result = PARSE_ERROR_GRAPHVIZ;
  }
  if (parse1) {
    result = TEST_SUCCESS;
    boost::property_map<ScheduleGraph, boost::vertex_index_t>::type vertex_id = get(boost::vertex_index, graph1);
    if (!config.silent) {
      std::cout << "Testing " << getGraphName(graph1) << " (file: " << dotFile1 << ") with " << num_vertices(graph1) << " vertices." << std::endl;
    }
    BOOST_FOREACH (boost::graph_traits<ScheduleGraph>::vertex_descriptor v, vertices(graph1)) {
      ScheduleVertex vTemp = graph1[get(vertex_id, v)];
      if (vTemp != vTemp) {
        if (!config.silent) {
          std::cout << "Test failed for " << vTemp.name << " (type=" << vTemp.type << "), protocol: " << vTemp.protocol << "." << std::endl;
        }
        result = TEST_FAIL;
      }
    }
    return result;
  } else {
    return (result == -1) ? FILE_NOT_FOUND : result;
  }
}

void listVertexProtocols(ScheduleGraph& graph) {
  boost::property_map<ScheduleGraph, boost::vertex_index_t>::type vertex_id = get(boost::vertex_index, graph);
  BOOST_FOREACH (boost::graph_traits<ScheduleGraph>::vertex_descriptor v, vertices(graph)) {
    ScheduleVertex vTemp = graph[get(vertex_id, v)];
    if (!vTemp.protocol.empty()) {
      std::cout << vTemp.name << ": " << vTemp.protocol << std::endl;
    }
  }
}
