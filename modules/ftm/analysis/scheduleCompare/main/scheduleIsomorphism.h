#ifndef SCHEDULE_ISOMORPHISM_H
#define SCHEDULE_ISOMORPHISM_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <string>

#include "ScheduleEdge.h"
#include "ScheduleVertex.h"
#include "scheduleCompare.h"

typedef boost::property<boost::graph_name_t, std::string> GraphProperty;

// Using a vecS graphs => the index maps are implicit.
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, ScheduleVertex, ScheduleEdge, GraphProperty> ScheduleGraph;

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

int scheduleIsomorphic(std::string dotFile1, std::string dotfile2, configuration& config);
int testSingleGraph(std::string dotFile1, configuration& config);
boost::dynamic_properties setDynamicProperties(ScheduleGraph& g, configuration& config);
std::string getGraphName(ScheduleGraph& g);
void listVertexProtocols(ScheduleGraph& graph);
#endif
