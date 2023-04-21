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
    bool insertEdges();
    bool getStartOfChain(VertexNum v, VertexNum first);
    EdgeDescriptor* createEdgeProperties(VertexNum v1, VertexNum v2, VertexNum v3, bool flag);
    VertexNum createVertexProperties(VertexNum v);
    ScheduleGraph* g;
    configuration* c;
    int counterReplacedChains = 0;
    VertexNum startOfChain = ULONG_MAX;
    VertexNum newVertexNum = ULONG_MAX;
    // new edge 'before' the new vertex
    EdgeDescriptor* beforeEdge;
    EdgeDescriptor beforeEdgeOld;
    // new edge 'after' the new vertex
    EdgeDescriptor* afterEdge;
    EdgeDescriptor afterEdgeOld;
    std::pair<EdgeDescriptor, bool> newEdge;
    VertexNum anyPredecessor(VertexNum v);
    VertexNum predecessorInChain(VertexNum v);
    VertexNum predecessor(VertexNum v, bool inChain);
    VertexNum anySuccessor(VertexNum v);
    VertexNum successorInChain(VertexNum v);
    VertexNum successor(VertexNum v, bool inChain);
    void getBeforeEdge(VertexNum v);
    void getAfterEdge(VertexNum v);
    VertexId id = boost::get(boost::vertex_index, *g);
    VertexSet chain = {};
    void printChain(std::string title);
    void chainStatus(std::string title, std::ostream& out);
    std::string newName;
    std::string newLabel;
};
#endif
