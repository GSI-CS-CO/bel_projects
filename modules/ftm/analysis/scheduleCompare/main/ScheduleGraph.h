#ifndef SCHEDULE_GRAPH_H
#define SCHEDULE_GRAPH_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <string>

#include "configuration.h"
#include "ScheduleVertex.h"
#include "ScheduleEdge.h"

class GraphProperties {
  // graph [root="Demo",rankdir = TB, nodesep = 0.6, mindist = 1.0, ranksep = 1.0, overlap = false]
 public:
  std::string name = std::string("");
  std::string root = std::string("");
  std::string rankdir = std::string("");
  std::string nodesep = std::string("");
  std::string mindist = std::string("");
  std::string ranksep = std::string("");
  std::string overlap = std::string("");
  std::string xdotversion = std::string("");
  std::string _draw_ = std::string("");
  std::string bb = std::string("");
};

// Using a vecS graphs => the index maps are implicit.
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, ScheduleVertex, ScheduleEdge, GraphProperties> ScheduleGraph;

typedef boost::property_map<ScheduleGraph, std::string ScheduleVertex::*>::type VertexNameMap;
typedef boost::property_map<ScheduleGraph, std::string ScheduleEdge::*>::type EdgeNameMap;

template <class VertexNameMap>
struct nameEqualityFilter {
  nameEqualityFilter(VertexNameMap map1, VertexNameMap map2) : vertexNames1(map1), vertexNames2(map2) {}
  template <typename Vertex>
  bool operator()(const Vertex& v) const {
    bool ret = false;
    for (auto& it : vertexNames2) {
      ret |= (vertexNames1[v] == it.second);
    }
    return ret;
  }
  VertexNameMap vertexNames1;
  VertexNameMap vertexNames2;
};

boost::dynamic_properties setDynamicProperties(ScheduleGraph& g, configuration& config);
std::string getGraphName(ScheduleGraph& g);
void setGraphName(ScheduleGraph& g, std::string newName);

#endif
