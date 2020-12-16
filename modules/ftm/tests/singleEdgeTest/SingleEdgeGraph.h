#ifndef _SINGLEEDGEGRAPH_H_
#define _SINGLEEDGEGRAPH_H_

#include "carpeDM.h"
#include "dotstr.h"

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
  CarpeDM* cdm;
  /** The created graph. */
  Graph g1;
  /** Memorize the nodes for later use. node1 is stored in v1 an so on.*/
  vertex_t v1, v2, v3, v4, v5;
  /** Complete the myVertex nodes with a valid node pointer depending on the type. */
  void setNodePointer(myVertex* vertex, std::string type);
  /** Extend graphs with forbiddden childless nodes. If node2 is of type event or qinfo, it may not be childless. */
  void extendWithChild();
  /** Extend graphs with forbidden orphan nodes. If node1 is of type meta, it may not be orphan. */
  void extendOrphanNode();
  /** Extend graphs with a second qbuf node if node1 has type qinfo. */
  void extendSecondQbuf();

 public:
  /** Create a graph from given node and edge types.
\param nodeT1 Type of node1. Allowed values from namespace DotStr::Node::TypeVal.
\param nodeT2 Type of node2. Allowed values from namespace DotStr::Node::TypeVal.
\param edgeT Type of edge from node1 to node2. Allowed values from namespace DotStr::Edge::TypeVal.
  */
  SingleEdgeGraph(CarpeDM* cdm, std::string nodeT1, std::string nodeT2, std::string edgeT);
  /** Return the graph stored in this object. */
  Graph getGraph() { return g1; };
  /** Print the graph as nodes and edges to stdout. */
  void print(bool verbose);
  /** Write the dot file of the grapgh to folder dot/, adding fileNamePart to the file name. */
  void writeDotFile(std::string fileNamePart);
};

#endif
