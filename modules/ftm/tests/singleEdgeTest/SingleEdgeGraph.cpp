#include "SingleEdgeGraph.h"

#include "block.h"
#include "carpeDM.h"
#include "carpeDMimpl.h"
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
    {dnt::sOrigin, NODE_TYPE_ORIGIN},
    {dnt::sStartThread, NODE_TYPE_STARTTHREAD},
};

namespace det = DotStr::Edge::TypeVal;
namespace dnm = DotStr::Node::MetaGen;



SingleEdgeGraph::SingleEdgeGraph(CarpeDM::CarpeDMimpl* carpeDM, configuration& config, std::string nodeT1, std::string nodeT2, std::string edgeT) {
  cdm = carpeDM;
  uint32_t flags = 0;
  // declare a graph object, adding the edges and edge properties
  Graph g = static_cast<Graph>(*this);
  // compose the first vertex
  v1 = boost::add_vertex(g);
  g[v1].name = "A1";
  g[v1].type = nodeT1;
  g[v1].patName = "patternA";
  g[v1].bpName = "beamA";
  g[v1].tOffs = "0";
  if (g[v1].type.compare(dnt::sTMsg) == 0) {
    g[v1].id_fid = "1";
    g[v1].id_gid = "33";
    if (edgeT.compare(det::sDynPar0) == 0) {
      g[v1].par = "0x000000000412099c";
      g[v1].tef = "2068673551";
      flags = NFLG_TMSG_DYN_PAR0_SMSK;
    } else if (edgeT.compare(det::sDynPar1) == 0) {
      g[v1].par = "0x0412099c00000000";
      g[v1].tef = "2068673551";
      flags = NFLG_TMSG_DYN_PAR1_SMSK;
    } else {
      g[v1].par = "1";
      g[v1].tef = "0";
    }
  } else if (g[v1].type.compare(dnt::sCmdFlush) == 0) {
    g[v1].qLo = "1";
    g[v1].qIl = "0";
    g[v1].qHi = "0";
  } else if (g[v1].type.compare(dnt::sCmdWait) == 0) {
    g[v1].tWait = "100";
  } else if (g[v1].type.compare(dnt::sStartThread) == 0) {
    g[v1].startOffs = "500";
  } else if (g[v1].type.compare(dnt::sBlock) == 0 || g[v1].type.compare(dnt::sBlockAlign) == 0) {
    flags=0x00100007;
    g[v1].tPeriod = "1000";
    g[v1].qLo = "1";
    if (edgeT.compare(det::sQPrio[1]) == 0) {
      g[v1].qHi = "1";
      flags |= 0x00200000;
    } else if (edgeT.compare(det::sQPrio[2]) == 0) {
      g[v1].qIl = "1";
      flags |= 0x00400000;
    }
    if (config.generateMetaNodes) {
      if (nodeT2.compare(dnt::sQInfo) != 0) {
        generateQmeta(g, v1, 0);
      }
    }
  }
  cdm->completeId(v1, g);
  flags |= NFLG_PAT_ENTRY_LM32_SMSK;
  setNodePointer(&g[v1], nodeT1, flags);
  flags = 0;
  // compose the second vertex
  v2 = boost::add_vertex(g);
  g[v2].name = "B2";
  g[v2].cpu = "0";
  g[v2].type = nodeT2;
  g[v2].patName = "patternA";
  g[v2].bpName = "beamA";
  g[v2].tOffs = "0";
  if (nodeT2.compare(dnt::sTMsg) == 0) {
    g[v2].id_fid = "1";
    g[v2].id_gid = "33";
    g[v2].par = "1";
    g[v2].tef = "0";
    cdm->completeId(v2, g);
  } else if (nodeT2.compare(dnt::sCmdFlow) == 0 || nodeT2.compare(dnt::sSwitch) == 0) {
    g[v2].tOffs = "0";
  } else if (nodeT2.compare(dnt::sCmdNoop) == 0) {
    g[v2].tOffs = "0";
  } else if (nodeT2.compare(dnt::sCmdWait) == 0) {
    g[v2].tWait = "200";
  } else if (nodeT2.compare(dnt::sStartThread) == 0) {
    g[v2].startOffs = "500";
  } else if (nodeT2.compare(dnt::sBlock) == 0 || nodeT2.compare(dnt::sBlockAlign) == 0) {
    flags=0x00100007;
    g[v2].tPeriod = "1000";
    g[v2].qLo = 1;
    cdm->completeId(v2, g);
    if (config.generateMetaNodes) {
      generateQmeta(g, v2, 0);
    }
  }
  flags |= NFLG_PAT_EXIT_LM32_SMSK;
  setNodePointer(&g[v2], nodeT2, flags);
  // connect v1 and v2 by an edge of type edgeT
  boost::add_edge(v1, v2, myEdge(edgeT), g);
  // connect v1 and v2 by an edge of type defdst in some cases
  if ((g[v1].type.compare(dnt::sCmdFlow) == 0 || g[v1].type.compare(dnt::sTMsg) == 0) && 
      (g[v2].type.compare(dnt::sBlock) == 0 || g[v2].type.compare(dnt::sBlockAlign) == 0) && 
      edgeT.compare(det::sDefDst) != 0 &&
      edgeT.compare(det::sCmdFlowDst) != 0 &&
      edgeT.compare(det::sCmdTarget) != 0) {
    boost::add_edge(v1, v2, myEdge(det::sDefDst), g);
  }
  if ((g[v1].type.compare(dnt::sCmdWait) == 0 || g[v1].type.compare(dnt::sSwitch) == 0 || g[v1].type.compare(dnt::sOrigin) == 0 || g[v1].type.compare(dnt::sCmdFlush) == 0) &&
      (g[v2].type.compare(dnt::sBlock) == 0 || g[v2].type.compare(dnt::sBlockAlign) == 0) &&
      edgeT.compare(det::sDefDst) != 0 &&
      edgeT.compare(det::sCmdFlowDst) != 0) {
    boost::add_edge(v1, v2, myEdge(det::sDefDst), g);
    if (g[v1].type.compare(dnt::sSwitch) == 0 && edgeT.compare(det::sCmdTarget) != 0) {
      boost::add_edge(v1, v2, myEdge(det::sCmdTarget), g);
    }
    if (g[v1].type.compare(dnt::sSwitch) == 0 && edgeT.compare(det::sSwitchDst) == 0) {
      boost::add_edge(v2, v2, myEdge(det::sDefDst), g);
    }
  }
  if (g[v1].type.compare(dnt::sCmdFlush) == 0 &&
      (g[v2].type.compare(dnt::sBlock) == 0 || g[v2].type.compare(dnt::sBlockAlign) == 0) &&
      edgeT.compare(det::sCmdFlushOvr) == 0) {
    boost::add_edge(v1, v2, myEdge(det::sCmdTarget), g);
  }
  // add child vertex, blocks for a meta vertex, or a buffer vertex if necessary.
  g1 = g;
  extendWithChild(edgeT, config);
  extendOrphanNode();
  extendSecondQbuf();
}

void SingleEdgeGraph::extendWithChild(std::string edgeT, configuration& config) {
  uint32_t flags = 0;
  if ((g1[v2].np->isEvent()) || (g1[v2].type.compare(dnt::sQInfo) == 0) || 
      (g1[v1].type.compare(dnt::sCmdFlow) == 0) ||
      (g1[v1].type.compare(dnt::sCmdNoop) == 0)) {
    std::string v3Type = (g1[v2].type.compare(dnt::sQInfo) == 0) ? dnt::sQBuf : dnt::sBlock;
    std::string v3Edge = (g1[v2].type.compare(dnt::sQInfo) == 0) ? det::sMeta : det::sDefDst;
    v3 = boost::add_vertex(g1);
    g1[v3].name = "C3";
    g1[v3].type = v3Type;
    g1[v3].patName = "patternA";
    g1[v3].bpName = "beamA";
    if (g1[v3].type.compare(dnt::sBlock) == 0 || g1[v3].type.compare(dnt::sBlockAlign) == 0) {
      flags=0x00100007;
      g1[v3].tPeriod = "2000";
      g1[v3].qLo = 1;
      cdm->completeId(v3, g1);
      if (config.generateMetaNodes) {
        generateQmeta(g1, v3, 0);
      }
    }
    setNodePointer(&g1[v3], v3Type, flags);
    boost::add_edge(v2, v3, myEdge(v3Edge), g1);
    if (g1[v1].type.compare(dnt::sSwitch) == 0 && v3Type.compare(dnt::sBlock) == 0) {
      boost::add_edge(v1, v3, myEdge(det::sCmdTarget), g1);
      if (edgeT.compare(det::sDefDst) != 0) {
        boost::add_edge(v3, v2, myEdge(det::sDefDst), g1);
      }
    }
    if (g1[v1].type.compare(dnt::sCmdFlush) == 0 && v3Type.compare(dnt::sBlock) == 0) {
      boost::add_edge(v1, v3, myEdge(det::sCmdTarget), g1);
    }
    if (g1[v2].type.compare(dnt::sCmdWait) == 0 || 
        g1[v2].type.compare(dnt::sBlock) == 0 || 
        g1[v2].type.compare(dnt::sBlockAlign) == 0 || 
        g1[v2].type.compare(dnt::sCmdFlow) == 0 || 
        g1[v2].type.compare(dnt::sCmdFlush) == 0 || 
        g1[v2].type.compare(dnt::sCmdNoop) == 0 || 
        g1[v2].type.compare(dnt::sOrigin) == 0 || 
        g1[v2].type.compare(dnt::sStartThread) == 0 || 
        g1[v2].type.compare(dnt::sSwitch) == 0 || 
        g1[v2].type.compare(dnt::sTMsg) == 0) {
      if (edgeT.compare(det::sDefDst) != 0) {
        boost::add_edge(v1, v3, myEdge(det::sDefDst), g1);
      }
    } else if (g1[v2].type.compare(dnt::sQInfo) == 0) {
      v4 = boost::add_vertex(g1);
      g1[v4].name = "D4";
      g1[v4].type = v3Type;
      g1[v4].patName = "patternA";
      g1[v4].bpName = "beamA";
      setNodePointer(&g1[v4], v3Type, 0);
      boost::add_edge(v2, v4, myEdge(v3Edge), g1);
    }
  }
}

void SingleEdgeGraph::extendOrphanNode() {
  if (g1[v1].np->isMeta() && g1[v1].type.compare(dnt::sDstList) != 0) {
    v5 = boost::add_vertex(g1);
    g1[v5].name = "E5";
    g1[v5].type = dnt::sBlock;
    g1[v5].patName = "patternA";
    g1[v5].bpName = "beamA";
    g1[v5].tPeriod = "1000";
    g1[v5].qLo = "1";
    uint32_t flags=0x00100007;
    flags |= NFLG_PAT_ENTRY_LM32_SMSK;
    flags |= NFLG_PAT_EXIT_LM32_SMSK;
    setNodePointer(&g1[v5], dnt::sBlock, flags);
    boost::add_edge(v5, v1, myEdge(det::sQPrio[0]), g1);
    v6 = boost::add_vertex(g1);
    g1[v6].name = "E5_ListDst_0";
    g1[v6].type = dnt::sDstList;
    g1[v6].patName = "patternA";
    g1[v6].bpName = "beamA";
    setNodePointer(&g1[v6], dnt::sDstList, 0);
    boost::add_edge(v6, v5, myEdge(det::sDefDst), g1);
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
        g1[v3].patName = "patternA";
        g1[v3].bpName = "beamA";
        setNodePointer(&g1[v3], dnt::sQBuf, 0);
        boost::add_edge(v1, v3, myEdge(det::sMeta), g1);
        break;
      }
    }
  }
}

void SingleEdgeGraph::setNodePointer(myVertex* vertex, std::string type, uint32_t flags) {
  uint32_t hash = 0;
  uint8_t cpu = 0;
  switch (nodeMap[type]) {
    case NODE_TYPE_TMSG:
      vertex->np = (node_ptr) new TimingMsg(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags, s2u<uint64_t>(vertex->tOffs), s2u<uint64_t>(vertex->id),
                                            s2u<uint64_t>(vertex->par), s2u<uint32_t>(vertex->tef), s2u<uint32_t>(vertex->res));
      break;
    case NODE_TYPE_CNOOP:
      vertex->np = (node_ptr) new Noop(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags, s2u<uint64_t>(vertex->tOffs), s2u<uint64_t>(vertex->tValid),
                                            s2u<uint8_t>(vertex->prio), s2u<uint32_t>(vertex->qty), s2u<bool>(vertex->vabs));
      break;
    case NODE_TYPE_CFLOW:
      vertex->np = (node_ptr) new Flow(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags, s2u<uint64_t>(vertex->tOffs), s2u<uint64_t>(vertex->tValid),
                                            s2u<uint8_t>(vertex->prio), s2u<uint32_t>(vertex->qty), s2u<bool>(vertex->vabs), s2u<bool>(vertex->perma));
      break;
    case NODE_TYPE_CSWITCH:
      vertex->np = (node_ptr) new Switch(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags, s2u<uint64_t>(vertex->tOffs));
      break;
    case NODE_TYPE_CFLUSH:
      vertex->np = (node_ptr) new Flush(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags, s2u<uint64_t>(vertex->tOffs), s2u<uint64_t>(vertex->tValid),
                                            s2u<uint8_t>(vertex->prio), s2u<bool>(vertex->qIl), s2u<bool>(vertex->qHi), s2u<bool>(vertex->qLo), s2u<bool>(vertex->vabs), s2u<bool>(vertex->perma));
      break;
    case NODE_TYPE_CWAIT:
      vertex->np = (node_ptr) new Wait(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags, s2u<uint64_t>(vertex->tOffs), s2u<uint64_t>(vertex->tValid),
                                            s2u<uint8_t>(vertex->prio), s2u<uint32_t>(vertex->tWait), s2u<bool>(vertex->vabs));
      break;
    case NODE_TYPE_BLOCK_FIXED:
      vertex->np = (node_ptr) new BlockFixed(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags, s2u<uint64_t>(vertex->tPeriod));
      break;
    case NODE_TYPE_BLOCK_ALIGN:
      vertex->np = (node_ptr) new BlockAlign(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags, s2u<uint64_t>(vertex->tPeriod));
      break;
    case NODE_TYPE_QUEUE:
      vertex->np = (node_ptr) new CmdQMeta(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags);
      break;
    case NODE_TYPE_ALTDST:
      //~ std::cout << "setNodePointer: NODE_TYPE_ALTDST, " << vertex->name << std::endl;
      vertex->np = (node_ptr) new DestList(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags);
      break;
    case NODE_TYPE_QBUF:
      vertex->np = (node_ptr) new CmdQBuffer(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags);
      break;
    case NODE_TYPE_ORIGIN:
      vertex->np = (node_ptr) new Origin(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags, s2u<uint64_t>(vertex->tOffs), 0);
      break;
    case NODE_TYPE_STARTTHREAD:
      vertex->np = (node_ptr) new StartThread(vertex->name, vertex->patName, vertex->bpName, hash, cpu, flags, s2u<uint64_t>(vertex->tOffs), s2u<uint64_t>(vertex->startOffs), 0);
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


void SingleEdgeGraph::generateQmeta(Graph& g, vertex_t v, int prio) {
  const std::string nameBl = g[v].name + dnm::sQBufListTag + dnm::sQPrioPrefix[prio];
  const std::string nameB0 = g[v].name + dnm::sQBufTag     + dnm::sQPrioPrefix[prio] + dnm::s1stQBufSuffix;
  const std::string nameB1 = g[v].name + dnm::sQBufTag     + dnm::sQPrioPrefix[prio] + dnm::s2ndQBufSuffix;

  vertex_t vBl = boost::add_vertex(myVertex(nameBl, g[v].cpu, 0, nullptr, dnt::sQInfo, DotStr::Misc::sHexZero), g);
  vertex_t vB0 = boost::add_vertex(myVertex(nameB0, g[v].cpu, 0, nullptr, dnt::sQBuf,  DotStr::Misc::sHexZero), g);
  vertex_t vB1 = boost::add_vertex(myVertex(nameB1, g[v].cpu, 0, nullptr, dnt::sQBuf,  DotStr::Misc::sHexZero), g);

  g[vBl].patName = g[v].patName;
  g[vB0].patName = g[v].patName;
  g[vB1].patName = g[v].patName;
  setNodePointer(&g[vBl], dnt::sQInfo, 0);
  setNodePointer(&g[vB0], dnt::sQBuf, 0);
  setNodePointer(&g[vB1], dnt::sQBuf, 0);

  boost::add_edge(v,   vBl, myEdge(det::sQPrio[prio]), g);
  boost::add_edge(vBl, vB0, myEdge(det::sMeta),    g);
  boost::add_edge(vBl, vB1, myEdge(det::sMeta),    g);
}

