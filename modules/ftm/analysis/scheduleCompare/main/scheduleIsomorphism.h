#ifndef SCHEDULE_ISOMORPHISM_H
#define SCHEDULE_ISOMORPHISM_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <string>

#include "scheduleCompare.h"

class ScheduleVertex {
 public:
  std::string name = std::string("");
  std::string type = std::string("");

  int compare(const ScheduleVertex& v1, const ScheduleVertex& v2) {
    // std::cout << "--V " << v1.name << ", " << v2.name << std::endl;
    //      return v1.type.compare(v2.type);
    if (v1.name == v2.name) {
      return v1.type.compare(v2.type);
    } else {
      return v1.name.compare(v2.name);
    }
  }

  int compare(const ScheduleVertex& v2) {
    std::cout << "--> " << this->name << ", " << v2.name << std::endl;
    //      return this->type.compare(v2.type);
    if (this->name == v2.name) {
      return this->type.compare(v2.type);
    } else {
      return this->name.compare(v2.name);
    }
  }

  inline bool operator==(const ScheduleVertex& rhs) { return compare(*this, rhs) == 0; }
  inline bool operator!=(const ScheduleVertex& rhs) { return compare(*this, rhs) != 0; }
  inline bool operator<(const ScheduleVertex& rhs) { return compare(*this, rhs) < 0; }
  inline bool operator>(const ScheduleVertex& rhs) { return compare(*this, rhs) > 0; }
  inline bool operator<=(const ScheduleVertex& rhs) { return compare(*this, rhs) <= 0; }
  inline bool operator>=(const ScheduleVertex& rhs) { return compare(*this, rhs) >= 0; }
};

class ScheduleEdge {
 public:
  std::string name = std::string("");
  std::string type = std::string("");

  int compare(const ScheduleEdge& e1, const ScheduleEdge& e2) {
    std::cout << "--E " << e1.name << ", " << e2.name << std::endl;
    if (e1.name == e2.name) {
      return e1.type.compare(e2.type);
    } else {
      return e1.name.compare(e2.name);
    }
  }

  inline bool operator==(const ScheduleEdge& rhs) { return compare(*this, rhs) == 0; }
  inline bool operator!=(const ScheduleEdge& rhs) { return compare(*this, rhs) != 0; }
  inline bool operator<(const ScheduleEdge& rhs) { return compare(*this, rhs) < 0; }
  inline bool operator>(const ScheduleEdge& rhs) { return compare(*this, rhs) > 0; }
  inline bool operator<=(const ScheduleEdge& rhs) { return compare(*this, rhs) <= 0; }
  inline bool operator>=(const ScheduleEdge& rhs) { return compare(*this, rhs) >= 0; }
};

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

bool scheduleIsomorphic(std::string dotFile1, std::string dotfile2, configuration& config);
boost::dynamic_properties setDynamicProperties(ScheduleGraph& g);
std::string getGraphName(ScheduleGraph& g);
#endif
