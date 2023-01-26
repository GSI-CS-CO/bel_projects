#ifndef REPLACE_CHAIN_IMPL_H
#define REPLACE_CHAIN_IMPL_H

#include "replaceChain.h"

typedef long unsigned int VertexNum;
typedef boost::property_map<ScheduleGraph, boost::vertex_index_t>::type VertexId;
typedef std::set<VertexNum> VertexSet;
typedef boost::graph_traits<ScheduleGraph>::edge_descriptor EdgeDescriptor;
typedef boost::graph_traits<ScheduleGraph>::vertex_descriptor VertexDescriptor;

int replaceChain(ScheduleGraph& graph1, configuration& config);

class ReplaceChain {
  public:
    ReplaceChain(ScheduleGraph& graph1, configuration& config) :
      g(&graph1), c(&config)
    {};
    void outputGraph();
    int replaceChainLoop();

  private:
    bool replaceSingleChain();
    bool findStartOfChain();
    bool checkToReplace(VertexNum v);
    void createVertexAndEdges(VertexNum v);
    bool insertVertexAndEdges();
    bool getStartOfChain(VertexNum v);
    EdgeDescriptor* createEdgeProperties(VertexNum v1, VertexNum v2, VertexNum v3, bool flag);
    VertexNum createVertexProperties(VertexNum v);
    ScheduleGraph* g;
    configuration* c;
    int counterReplacedChains = 0;
    VertexNum startOfChain = ULONG_MAX;
    VertexNum newVertex = ULONG_MAX;
    //~ VertexNum beforeChain = ULONG_MAX;
    //~ VertexNum afterChain = ULONG_MAX;
    EdgeDescriptor* beforeEdge;
    EdgeDescriptor* afterEdge;
    std::pair<EdgeDescriptor, bool> *newEdge;
    //~ VertexNum nextTestVertex = ULONG_MAX;
    VertexNum predecessor(VertexNum v);
    VertexNum successor(VertexNum v);
    VertexId id = boost::get(boost::vertex_index, *g);
    VertexSet chain = {};
};
#endif
