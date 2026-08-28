#ifndef _SINGLEEDGEGRAPH_H_
#define _SINGLEEDGEGRAPH_H_

#include "carpeDM.h"
#include "carpeDMimpl.h"
#include "dotstr.h"

#include "configuration.h"

namespace dnt = DotStr::Node::TypeVal;

extern std::map<std::string, int> nodeMap;

/**
\class SingleEdgeGraph
SingleEdgeGraph creates the test graphs. Each graph consists of two nodes an a
connecting edge. In some cases more nodes and edges are needed to form a valid test schedule.

The constructor creates a graph with node1 of a given type, node2 of a given type,
and an edge of given type to connect node1 and node2.

Special cases:
If node2 cannot be childless, add node3 and an edge from node2 to node3. See extendWithChild() for details.
Iif node2 is of type qinfo, add two nodes node3 and node4 of type qbuf and connect these with edges of type meta to node2.
If node1 cannot be orphan, add node5 and connect this with an edge to node1. See extendOrphanNode() for details.
If node1 is of type qinfo, add node3 of type qbuf and connect this to node1. See extendSecondQbuf() for details.
*/
class SingleEdgeGraph : public Graph {
 private:
  CarpeDM::CarpeDMimpl* cdm;
  /** The created graph. */
  Graph g1;
  /** Memorize the nodes for later use. node1 is stored in v1 an so on.*/
  vertex_t v1, v2, v3, v4, v5, v6;
  /** Complete the myVertex nodes with a valid node pointer depending on the type. */
  void setNodePointer(configuration& config, myVertex* vertex, std::string type, uint32_t flags);
  /** Extend graphs with forbiddden childless nodes. If node2 is of type event or qinfo, it may not be childless. */
  void extendWithChild(configuration& config, std::string edgeT);
  /** Extend graphs with forbidden orphan nodes. If node1 is of type meta, it may not be orphan. */
  void extendOrphanNode(configuration& config);
  /** Extend graphs with a second qbuf node if node1 has type qinfo. */
  void extendSecondQbuf(configuration& config);
  /** Generate the meta nodes for blocks*/
  void generateQmeta(configuration& config, Graph& g, vertex_t v, int prio);

 public:
  /** Create a graph from given node and edge types.
\param cdm CarpeDM instance.
\param config configuration from command line options.
\param nodeT1 Type of node1. Allowed values from namespace DotStr::Node::TypeVal.
\param nodeT2 Type of node2. Allowed values from namespace DotStr::Node::TypeVal.
\param edgeT Type of edge from node1 to node2. Allowed values from namespace DotStr::Edge::TypeVal.
  */
  SingleEdgeGraph(CarpeDM::CarpeDMimpl* cdm, configuration& config, std::string nodeT1, std::string nodeT2, std::string edgeT);
  /** Return the graph stored in this object. */
  Graph getGraph() { return g1; };
  /** Print the graph as nodes and edges to stdout. */
  void print(bool verbose);
  /** Write the dot file of the grapgh to folder dot/, adding fileNamePart to the file name. */
  void writeDotFile(std::string fileNamePart);
};

#endif
