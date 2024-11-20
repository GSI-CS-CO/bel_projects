
#include "scheduleIsomorphism.h"

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>

#include "parseSchedule.h"
#include "printSchedule.h"
#include "scheduleCompare.h"
#include "scheduleCompact.h"

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
    isomorphismCounter++;
    return true;
  }

  std::vector<std::vector<std::pair<int, int>>> get_setvmap() {
    return set_of_vertex_iso_map;
  }

  int getIsomorphismCounter() {
    return isomorphismCounter;
  }

 private:
  const Graph1& graph1_;
  const Graph1& graph2_;
  std::vector<std::pair<int, int>> vertex_iso_map;
  std::vector<std::vector<std::pair<int, int>>> set_of_vertex_iso_map;
  int isomorphismCounter = 1;
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
    std::cerr << "Parsing graph1: Property not found " << excep.what() << std::endl;
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
    std::cerr << "Parsing graph2: Property not found " << excep.what() << std::endl;
    result = PARSE_ERROR;
  } catch (boost::bad_graphviz_syntax &excep) {
    std::cerr << "Parsing graph2: Bad Graphviz syntax: " << excep.what() << std::endl;
    result = PARSE_ERROR_GRAPHVIZ;
  }
  // std::cerr << "parse1: " << parse1 << ", parse2: " << parse2 << ", result: " << result << std::endl;
  if (parse1 && parse2) {
    // set the flag for comparing the names on all vertices of both graphs.
    switchCompareNames(graph1, config.compareNames);
    switchCompareNames(graph2, config.compareNames);
    // set the flag for handling "undefined" as empty string on all vertices of both graphs.
    switchUndefinedAsEmpty(graph1, config.undefinedAsEmpty);
    switchUndefinedAsEmpty(graph2, config.undefinedAsEmpty);
    // Use the smaller graph as graph1.
    ScheduleGraph *ref1, *ref2;
    std::string *refName1, *refName2;
    if (num_vertices(graph1) > num_vertices(graph2) ||
        (num_vertices(graph1) == num_vertices(graph2) && num_edges(graph1) > num_edges(graph2))) {
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
    std::string isSubgraph = "";
    if (num_vertices(*ref1) == num_vertices(*ref2) && num_edges(*ref1) == num_edges(*ref2)) {
      result = (isomorphic ? EXIT_SUCCESS : NOT_ISOMORPHIC);
    } else {
      result = (isomorphic ? SUBGRAPH_ISOMORPHIC : NOT_ISOMORPHIC);
      isSubgraph = "a subgraph of ";
    }

    if (!config.silent) {
      std::cout << "Graph " << getGraphName(*ref1) << " (" << *refName1 << ") is " << (isomorphic ? "" : "NOT ") << "isomorphic to "
                << isSubgraph << "graph " << getGraphName(*ref2) << " (" << *refName2 << ")." << std::endl;
      if (config.verbose) {
        std::string prefix = "Isomorphism " + std::to_string(callback.getIsomorphismCounter()) + ", Graph 1, ";
        listVertexProtocols(*ref1, prefix);
        listEdgeProtocols(*ref1, prefix);
      }
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

int testSingleGraph(std::string dotFile1, configuration& config) {
  ScheduleGraph graph1;
  bool parse1 = false;
  int result = -1;
  try {
    boost::dynamic_properties dp1 = setDynamicProperties(graph1, config);
    parse1 = parseSchedule(dotFile1, graph1, dp1, config);
    printSchedule("Graph:", graph1, dp1, config);
  } catch (boost::property_not_found &excep) {
    std::cerr << "Parsing graph: Property not found " << excep.what() << std::endl;
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

void listVertexProtocols(ScheduleGraph& graph, const std::string prefix) {
  auto vertex_pair = vertices(graph);
  for (auto iter = vertex_pair.first; iter != vertex_pair.second; iter++) {
    std::string protocol = graph[*iter].printProtocol();
    if (!protocol.empty()) {
    //~ if (protocol.find("compare") != std::string::npos) {
      std::cout << prefix << protocol << std::endl;
    }
  }
}

void listEdgeProtocols(ScheduleGraph& graph, const std::string prefix) {
  auto edge_pair = edges(graph);
  for (auto iter = edge_pair.first; iter != edge_pair.second; iter++) {
    std::string protocol = graph[*iter].printProtocol();
    if (protocol.find("Result") != std::string::npos) {
      std::cout << prefix << protocol << std::endl;
    }
  }
}

void switchCompareNames(ScheduleGraph& graph, const bool flag) {
  auto vertex_pair = vertices(graph);
  for (auto iter = vertex_pair.first; iter != vertex_pair.second; iter++) {
    graph[*iter].switchCompareNames(flag);
  }
}

void switchUndefinedAsEmpty(ScheduleGraph& graph, const bool flag) {
  auto vertex_pair = vertices(graph);
  for (auto iter = vertex_pair.first; iter != vertex_pair.second; iter++) {
    graph[*iter].switchUndefinedAsEmpty(flag);
  }
}
