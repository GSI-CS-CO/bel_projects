#include "SingleEdgeGraph.h"

#include "block.h"
#include "carpeDM.h"
#include "event.h"
#include "graph.h"
#include "meta.h"

std::map<std::string, int> nodeMap = {
    {dnt::sTMsg, NODE_TYPE_TMSG},
    {dnt::sCmdNoop, NODE_TYPE_CNOOP},
    {dnt::sCmdFlow, NODE_TYPE_CFLOW},
    {dnt::sSwitch, NODE_TYPE_CSWITCH},
    {dnt::sCmdFlush, NODE_TYPE_CFLUSH},
    {dnt::sCmdWait, NODE_TYPE_CWAIT},
    {dnt::sBlockFixed, NODE_TYPE_BLOCK_FIXED},
    {dnt::sBlockAlign, NODE_TYPE_BLOCK_ALIGN},
    {dnt::sQInfo, NODE_TYPE_QUEUE},
    {dnt::sDstList, NODE_TYPE_ALTDST},
    {dnt::sQBuf, NODE_TYPE_QBUF},
};

namespace det = DotStr::Edge::TypeVal;

SingleEdgeGraph::SingleEdgeGraph(CarpeDM* carpeDM, std::string nodeT1, std::string nodeT2, std::string edgeT) {
  cdm = carpeDM;
  // declare a graph object, adding the edges and edge properties
  Graph g = static_cast<Graph>(*this);
  v1 = boost::add_vertex(g);
  g[v1].name = "A1";
  g[v1].type = nodeT1;
  g[v1].patName = "patternA";
  g[v1].bpName = "beamA";
  if (g[v1].type.compare(dnt::sTMsg) == 0) {
    g[v1].id_fid = "1";
    cdm->completeId(v1, g);
  }
  setNodePointer(&g[v1], nodeT1);
  v2 = boost::add_vertex(g);
  g[v2].name = "B2";
  g[v2].type = nodeT2;
  g[v2].patName = "patternB";
  g[v2].bpName = "beamB";
  if (g[v2].type.compare(dnt::sTMsg) == 0) {
    g[v2].id_fid = "1";
    g[v2].id_gid = "33";
    cdm->completeId(v2, g);
  }
  setNodePointer(&g[v2], nodeT2);
  boost::add_edge(v1, v2, myEdge(edgeT), g);
  g1 = g;
  extendWithChild();
  extendOrphanNode();
  extendSecondQbuf();
}

void SingleEdgeGraph::extendWithChild() {
  if ((g1[v2].np->isEvent()) || (g1[v2].type == dnt::sQInfo)) {
    std::string v3Type = (g1[v2].type.compare(dnt::sQInfo) == 0) ? dnt::sQBuf : dnt::sBlock;
    std::string v3Edge = (g1[v2].type.compare(dnt::sQInfo) == 0) ? det::sMeta : det::sDefDst;
    v3 = boost::add_vertex(g1);
    g1[v3].name = "C3";
    g1[v3].type = v3Type;
    g1[v3].patName = "patternC";
    g1[v3].bpName = "beamC";
    setNodePointer(&g1[v3], v3Type);
    boost::add_edge(v2, v3, myEdge(v3Edge), g1);
    if (g1[v2].type.compare(dnt::sQInfo) == 0) {
      v4 = boost::add_vertex(g1);
      g1[v4].name = "D4";
      g1[v4].type = v3Type;
      g1[v4].patName = "patternC";
      g1[v4].bpName = "beamC";
      setNodePointer(&g1[v4], v3Type);
      boost::add_edge(v2, v4, myEdge(v3Edge), g1);
    }
  }
}

void SingleEdgeGraph::extendOrphanNode() {
  if (g1[v1].np->isMeta()) {
    std::string v5Type = dnt::sBlock;
    std::string v5Edge = det::sQPrio[0];
    v5 = boost::add_vertex(g1);
    g1[v5].name = "E5";
    g1[v5].type = v5Type;
    g1[v5].patName = "patternE";
    g1[v5].bpName = "beamE";
    setNodePointer(&g1[v5], v5Type);
    boost::add_edge(v5, v1, myEdge(v5Edge), g1);
  }
}

void SingleEdgeGraph::extendSecondQbuf() {
  if (g1[v1].type.compare(dnt::sQInfo) == 0 && g1[v2].type.compare(dnt::sQBuf) == 0) {
    Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(v1, g1);
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
      if (g1[*out_cur].type.compare(det::sMeta) == 0) {
        v3 = boost::add_vertex(g1);
        g1[v3].name = "C3";
        g1[v3].type = dnt::sQBuf;
        g1[v3].patName = "patternC";
        g1[v3].bpName = "beamC";
        setNodePointer(&g1[v3], dnt::sQBuf);
        boost::add_edge(v1, v3, myEdge(det::sMeta), g1);
        break;
      }
    }
  }
}

void SingleEdgeGraph::setNodePointer(myVertex* vertex, std::string type) {
  uint32_t hash = 0;
  uint8_t cpu = 0;
  uint32_t flags = 0;
  switch (nodeMap[type]) {
    case NODE_TYPE_TMSG:
      vertex->np = (node_ptr) new TimingMsg(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags, s2u<uint64_t>(vertex->tOffs), s2u<uint64_t>(vertex->id),
                                            s2u<uint64_t>(vertex->par), s2u<uint32_t>(vertex->tef), s2u<uint32_t>(vertex->res));
      break;
    case NODE_TYPE_CNOOP:
      vertex->np = (node_ptr) new Noop(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags);
      break;
    case NODE_TYPE_CFLOW:
      vertex->np = (node_ptr) new Flow(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags);
      break;
    case NODE_TYPE_CSWITCH:
      vertex->np = (node_ptr) new Switch(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags);
      break;
    case NODE_TYPE_CFLUSH:
      vertex->np = (node_ptr) new Flush(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags);
      break;
    case NODE_TYPE_CWAIT:
      vertex->np = (node_ptr) new Wait(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags);
      break;
    case NODE_TYPE_BLOCK_FIXED:
      vertex->np = (node_ptr) new BlockFixed(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags);
      break;
    case NODE_TYPE_BLOCK_ALIGN:
      vertex->np = (node_ptr) new BlockAlign(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags);
      break;
    case NODE_TYPE_QUEUE:
      vertex->np = (node_ptr) new CmdQMeta(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags);
      break;
    case NODE_TYPE_ALTDST:
      vertex->np = (node_ptr) new DestList(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags);
      break;
    case NODE_TYPE_QBUF:
      vertex->np = (node_ptr) new CmdQBuffer(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags);
      break;
    case NODE_TYPE_UNKNOWN:
      std::cerr << "not yet implemented " << vertex->type << std::endl;
      break;
    default:
      std::cerr << "Node type 0x" << std::hex << type << " not supported! " << std::endl;
  }
}

void SingleEdgeGraph::print(bool verbose) {
  boost::property_map<Graph, boost::vertex_index_t>::type vertex_id = get(boost::vertex_index, g1);
  if (verbose) {
    std::cout << "Vertices(g) = ";
    BOOST_FOREACH (boost::graph_traits<Graph>::vertex_descriptor v, vertices(g1)) {
      myVertex vTemp = g1[get(vertex_id, v)];
      std::cout << vTemp.name << " (type=" << vTemp.type << ", np=" << vTemp.np << ") ";
    }
    std::cout << std::endl;

    std::cout << "Edges(g) = ";
    BOOST_FOREACH (boost::graph_traits<Graph>::edge_descriptor e, edges(g1)) {
      std::cout << "(" << g1[get(vertex_id, source(e, g1))].name << "," << g1[get(vertex_id, target(e, g1))].name << ") ";
    }
    std::cout << std::endl;
  }
}

void SingleEdgeGraph::writeDotFile(std::string fileNamePart) {
  // fileNamePart = nodeT1 + "-" + nodeT2 + "-" + edgeT
  // write the graph to a dot file.
  // Don't filter meta nodes. Otherwise the graph may be incomplete without notice.
  cdm->writeDotFile("dot/testSingleEdge-" + fileNamePart + ".dot", g1, false);
}
