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
  BOOST_FOREACH (VertexDescriptor descriptor, vertices(*g)) {
    std::cout << "descriptor: " << descriptor << ", id: " << boost::get(id, descriptor) << std::endl;
    result = getStartOfChain(boost::get(id, descriptor));
    if (result) {
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
    std::cout << "1 v: " << v << std::endl;
  } else if (boost::in_degree(v, *g) == 1 && boost::out_degree(v, *g) == 0) {
    // check p(v). If p(v) is in the chain, proceed this test with p(v).
    // Otherwise next vertex in loop (return false).
    std::cout << "2 v: " << v << std::endl;
    VertexNum p = predecessor(v);
    if (boost::in_degree(p, *g) <= 1 && boost::out_degree(p, *g) <= 1) {
      result = getStartOfChain(p);
    } else {
      result = false;
    }
  } else if (boost::in_degree(v, *g) == 0 && boost::out_degree(v, *g) == 1) {
    // v is start of chain.
    std::cout << "3 v: " << v << std::endl;
    result = true;
    startOfChain = boost::get(id, v);
  } else if (boost::in_degree(v, *g) == 1 && boost::out_degree(v, *g) == 1) {
    // check p(v). If p(v) is in this chain, proceed this test with p(v).
    // Otherwise v is start of chain.
    std::cout << "4 v: " << v << std::endl;
    VertexNum p = predecessor(v);
    if (boost::in_degree(p, *g) <= 1 && boost::out_degree(p, *g) <= 1) {
      result = getStartOfChain(p);
    } else {
      result = true;
      startOfChain = boost::get(id, v);
    }
  }
  std::cout << "result: " << result << ", startOfChain:" << startOfChain << std::endl;
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

void ReplaceChain::getBeforeEdge(VertexNum v) {
  ScheduleGraph::in_edge_iterator inBegin, inEnd;
  boost::tie(inBegin, inEnd) = boost::in_edges(v, *g);
  std::cout << "getBeforeEdge edge: " << *inBegin << ", " << std::endl;
  beforeEdgeOld = *inBegin;
}

void ReplaceChain::getAfterEdge(VertexNum v) {
  ScheduleGraph::out_edge_iterator outBegin, outEnd;
  boost::tie(outBegin, outEnd) = boost::out_edges(v, *g);
  std::cout << "getAfterEdge edge: " << *outBegin << ", " << std::endl;
  afterEdgeOld = *outBegin;
}

bool ReplaceChain::checkToReplace(VertexNum v) {
  bool result = true;
  if (chain.size() == 0) {
    VertexNum s = successor(v);
    std::cout << "checkToReplace v:" << v << ", s:" << s << std::endl;
    if (boost::in_degree(s, *g) == 1 && boost::out_degree(s, *g) <= 1) {
      chain.insert(v);
      printChain("checkToReplace 0 chain " + std::to_string(chain.size()) + ":");
      result = checkToReplace(s);
    } else {
      result = false;
    }
  } else {
    if (boost::in_degree(v, *g) == 1 && boost::out_degree(v, *g) <= 1) {
      chain.insert(v);
      printChain("checkToReplace 1 chain " + std::to_string(chain.size()) + ":");
      result = checkToReplace(successor(v));
    } else {
      result = false;
    }
  }
  //~ return !chain.empty();
  return result;
}

void ReplaceChain::createVertexAndEdges(VertexNum v) {
  std::cout << "createVertexAndEdges v:" << v << std::endl;
  chainStatus(std::cout);
  if (v == startOfChain) {
    // create a new vertex n and copy properties from v.
    newVertex = createVertexProperties(v);
    // if there is a predecessor, create an edge to it.
    VertexNum p = predecessor(v);
    std::cout << "0 createVertexAndEdges v:" << v << ", p " << p << std::endl;
    if (p != ULONG_MAX) {
      beforeEdge = createEdgeProperties(p, v, newVertex, true);
    }
  }
  // if there is a successor, create an edge to it.
  VertexNum s = successor(v);
  std::cout << "1 createVertexAndEdges v:" << v << ", s " << s << std::endl;
  if (s != ULONG_MAX && chain.count(s) == 0) {
    afterEdge = createEdgeProperties(newVertex, v, s, false);
  }
}

VertexNum ReplaceChain::createVertexProperties(VertexNum v) {
  std::cout << "createVertexProperties v:" << v << std::endl;
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
  saveScheduleIndex("cout", *g, *c);
  return newVertexNum;
}

EdgeDescriptor* ReplaceChain::createEdgeProperties(VertexNum v1, VertexNum v2, VertexNum v3, bool flag) {
  std::cout << "createEdgeProperties v1:" << v1 << ", v2:" << v2 << ", v3:" << v3 << ", flag: " << flag << std::endl;
  saveScheduleIndex("cout", *g, *c);
  *newEdge = add_edge(v1, v3, *g);
  std::cout << "createEdgeProperties 0 newEdge: " << newEdge->first << std::endl;
  EdgeDescriptor* edge;
  if (flag) {
    // flag = true: v3 is new. Create (v1, v3), copy properties from (v1, v2).
    getBeforeEdge(v2);
    edge = &beforeEdgeOld;
  } else {
    // flag = false: v1 is new. Create (v1, v3), copy properties from (v2, v3).
    getAfterEdge(v2);
    edge = &afterEdgeOld;
  }
  std::cout << "createEdgeProperties 1 newEdge: " << newEdge->first << std::endl;
  (*g)[newEdge->first].pos = (*g)[*edge].pos;
  (*g)[newEdge->first]._draw_ = (*g)[*edge]._draw_;
  (*g)[newEdge->first]._hdraw_ = (*g)[*edge]._hdraw_;
  (*g)[newEdge->first].type = (*g)[*edge].type;
  (*g)[newEdge->first].color = (*g)[*edge].color;
  return &(newEdge->first);
}

bool ReplaceChain::insertEdges() {
  bool result = false;
  std::cout << "insertEdges" << std::endl;
  for (auto v: chain) {
    createVertexAndEdges(v);
  }
  for (auto reverseIterator = chain.rbegin(); reverseIterator != chain.rend(); reverseIterator++) {
    //~ VertexNum v1 = *reverseIterator;
    //~ std::cout << "delete vertex " << v1 << std::endl;
    std::cout << "delete vertex " << *reverseIterator << std::endl;
    clear_vertex(*reverseIterator, *g);
    boost::remove_vertex(*reverseIterator, *g);
  }
  // prepare for the next chain.
  startOfChain = ULONG_MAX;
  chain.clear();
  counterReplacedChains++;
  return result;
}

bool ReplaceChain::replaceSingleChain() {
  bool result = false;
  if (findStartOfChain()) {
    std::cout << "replaceSingleChain startOfChain: " << startOfChain << std::endl;
    checkToReplace(startOfChain);
    if (chain.size() > 0) {
      result = insertEdges();
    }
  }
  std::cout << "replaceSingleChain returns: " << result << std::endl;
  return result;
}

int ReplaceChain::replaceChainLoop() {
  bool result = true;
  for (int i = 0; i < c->chainCount && result; i++) {
    std::cout << "replaceChainLoop counterReplacedChains: " << counterReplacedChains << ", startOfChain: " << startOfChain << std::endl;
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

void ReplaceChain::printChain(std::string title) {
  std::cout << title;
  for (auto e : chain) {
    std::cout << std::setw(4) << e;
  }
  std::cout << "." << std::endl;
}

void ReplaceChain::chainStatus(std::ostream& out) {
  out << "startOfChain " << startOfChain
      << ", newVertex " << newVertex
      << ", beforeEdge " << *beforeEdge
      << ", beforeEdgeOld " << beforeEdgeOld
      << ", afterEdge " << *afterEdge
      << ", afterEdgeOld " << afterEdgeOld
      << ", newEdge " << newEdge->first << " " << newEdge->second << std::endl;
}
