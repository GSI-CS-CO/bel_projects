#include "replaceChainImpl.h"
#include "configuration.h"

int replaceChain(ScheduleGraph& graph1, configuration& config) {
  ReplaceChain object = ReplaceChain(graph1, config);
  int result = object.replaceChainLoop();
  object.outputGraph();
  return result;
}

bool ReplaceChain::findStartOfChain() {
  // result = true means: found start of a chain and this->startOfChain
  // is the VertexNum of the start.
  bool result = false;
  BOOST_FOREACH (VertexDescriptor v, vertices(*g)) {
    if (getStartOfChain(v)) {
      break;
    }
  }
  return result;
}

bool ReplaceChain::getStartOfChain(VertexNum v) {
  // checks that v is start of a chain.
  // If yes, return true and set this->startOfChain.
  // If no but v is in a chain, call this method recursively with p(v).
  // If no but v is not in chain, return false.
  bool result = false;
  if ((boost::in_degree(v, *g) == 0 && boost::out_degree(v, *g) == 0) || (boost::in_degree(v, *g) > 1 && boost::out_degree(v, *g) > 1)){
    // v is not in a chain, proceed with next vertex in loop (return false).
  } else if (boost::in_degree(v, *g) == 1 && boost::out_degree(v, *g) == 0) {
    // check p(v). If p(v) is in the chain, proceed this test with p(v).
    // Otherwise next vertex in loop (return false).
    VertexNum p = predecessor(v);
    if (boost::in_degree(p, *g) <= 1 && boost::out_degree(p, *g) <= 1) {
      result = getStartOfChain(p);
    } else {
      result = false;
    }
  } else if (boost::in_degree(v, *g) == 0 && boost::out_degree(v, *g) == 1) {
    // v is start of chain.
    result = true;
    startOfChain = boost::get(id, v);
  } else if (boost::in_degree(v, *g) == 1 && boost::out_degree(v, *g) == 1) {
    // check p(v). If p(v) is in this chain, proceed this test with p(v).
    // Otherwise v is start of chain.
    VertexNum p = predecessor(v);
    if (boost::in_degree(p, *g) <= 1 && boost::out_degree(p, *g) <= 1) {
      result = getStartOfChain(p);
    } else {
      result = true;
      startOfChain = boost::get(id, v);
    }
  }
  return result;
}

VertexNum ReplaceChain::predecessor(VertexNum v) {
  VertexNum predecessor = ULONG_MAX;
  if (boost::in_degree(v, *g) == 1) {
    ScheduleGraph::in_edge_iterator in_begin, in_end;
    boost::tie(in_begin, in_end) = boost::in_edges(v, *g);
    VertexDescriptor source1 = source(*in_begin, *g);
    predecessor = boost::get(id, source1);
  }
  return predecessor;
}

VertexNum ReplaceChain::successor(VertexNum v) {
  VertexNum successor = ULONG_MAX;
  if (boost::out_degree(v, *g) == 1) {
    ScheduleGraph::out_edge_iterator out_begin, out_end;
    boost::tie(out_begin, out_end) = boost::out_edges(v, *g);
    VertexDescriptor target1 = target(*out_begin, *g);
    successor = boost::get(id, target1);
  }
  return successor;
}

bool ReplaceChain::checkToReplace(VertexNum v) {
  VertexNum s = successor(v);
  if (boost::in_degree(s, *g) == 1 && boost::out_degree(s, *g) <= 1) {
    chain.insert(v);
    createVertexAndEdges(v);
    checkToReplace(s);
  }
  return !chain.empty();
}

void ReplaceChain::createVertexAndEdges(VertexNum v) {
  if (v == startOfChain) {
    // create a new vertex n and copy properties from v.
    newVertex = createVertexProperties(v);
    // if there is a predecessor, create an edge to it.
    VertexNum p = predecessor(v);
    if (p != ULONG_MAX) {
      beforeEdge = createEdgeProperties(p, v, newVertex, true);
    }
  }
  // if there is a successor, create an edge to it.
  VertexNum s = successor(v);
  if (s != ULONG_MAX && chain.count(s) == 0) {
    afterEdge = createEdgeProperties(newVertex, v, s, false);
  }
}

VertexNum ReplaceChain::createVertexProperties(VertexNum v) {
  ScheduleVertex newVertex = ScheduleVertex();
  // Concate names from vertices in chain and use this as name for new vertex.
  newVertex.name = (*g)[v].name;
  newVertex.label = (*g)[v].label;
  newVertex.pos = (*g)[v].pos;
  newVertex.height = (*g)[v].height;
  newVertex.width = (*g)[v].width;
  newVertex._draw_ = (*g)[v]._draw_;
  newVertex._hdraw_ = (*g)[v]._hdraw_;
  newVertex._ldraw_ = (*g)[v]._ldraw_;
  newVertex.style = (*g)[v].style;
  newVertex.penwidth = (*g)[v].penwidth;
  newVertex.shape = (*g)[v].shape;
  newVertex.fillcolor = (*g)[v].fillcolor;
  newVertex.color = (*g)[v].color;
  newVertex.pattern = (*g)[v].pattern;
  // if there is no label, copy the name.
  if (newVertex.label.size() == 0) {
    newVertex.label = newVertex.name;
  }
  // Add the new vertex to the graph.
  VertexNum newVertexNum = add_vertex(newVertex, *g);
  return newVertexNum;
}

EdgeDescriptor* ReplaceChain::createEdgeProperties(VertexNum v1, VertexNum v2, VertexNum v3, bool flag) {
  *newEdge = add_edge(v1, v3, *g);
  EdgeDescriptor* edge;
  if (flag) {
    // flag = true: v3 is new. Create (v1, v3), copy properties from (v1, v2).
    edge = beforeEdge;
  } else {
    // flag = false: v1 is new. Create (v1, v3), copy properties from (v2, v3).
    edge = afterEdge;
  }
  (*g)[newEdge->first].pos = (*g)[*edge].pos;
  (*g)[newEdge->first]._draw_ = (*g)[*edge]._draw_;
  (*g)[newEdge->first]._hdraw_ = (*g)[*edge]._hdraw_;
  (*g)[newEdge->first].type = (*g)[*edge].type;
  (*g)[newEdge->first].color = (*g)[*edge].color;
  return &(newEdge->first);
}

bool ReplaceChain::insertVertexAndEdges() {
  bool result = false;
  startOfChain = ULONG_MAX;
  //~ beforeChain = ULONG_MAX;
  //~ afterChain = ULONG_MAX;
  chain.clear();
  counterReplacedChains++;
  return result;
}

bool ReplaceChain::replaceSingleChain() {
  bool result = false;
  if (findStartOfChain()) {
    if (checkToReplace(startOfChain)) {
      result = insertVertexAndEdges();
    }
  }
  return result;
}

int ReplaceChain::replaceChainLoop() {
  bool result = true;
  for (int i = 0; i < c->chainCount && result; i++) {
    result = replaceSingleChain();
  }
  return result;
}

void ReplaceChain::outputGraph() {
  if (!c->silent) {
    std::cout << "Output to file: '" << c->outputFile << "', counter: " << counterReplacedChains << std::endl;
  }
  saveSchedule(c->outputFile, *g, *c);
}
