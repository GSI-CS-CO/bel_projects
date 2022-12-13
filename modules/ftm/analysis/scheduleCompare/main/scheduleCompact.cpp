#include <set>

#include "scheduleCompact.h"
#include "printSchedule.h"
#include "scheduleCompare.h"

typedef long unsigned int VertexNum;
typedef std::set<VertexNum> VertexSet;

void printSet(VertexSet set1, std::string title);

void deleteChain(ScheduleGraph& graph1, VertexSet& candidates, VertexSet& deletes, VertexNum begin, VertexNum end);

int compactGraph(ScheduleGraph& graph1, configuration& config) {
    if (!config.silent) {
      std::cout << "Compacting " << getGraphName(graph1) << " with " << num_vertices(graph1) << " vertices." << std::endl;
    }
    boost::property_map<ScheduleGraph, boost::vertex_index_t>::type vertex_id = get(boost::vertex_index, graph1);
    VertexSet candidateList = {};
    VertexSet deleteList = {};
    VertexNum chainBegin = ULONG_MAX;
    VertexNum chainEnd = ULONG_MAX;
    //~ int count = 0;
    BOOST_FOREACH (boost::graph_traits<ScheduleGraph>::vertex_descriptor v, vertices(graph1)) {
      if (boost::in_degree(v, graph1) <= 1 && boost::out_degree(v, graph1) <= 1) {
        candidateList.insert(v);
        //~ ScheduleVertex vTemp = graph1[get(vertex_id, v)];
        //~ std::cout << count++ << " Candidate for compacting: " << v << ", " << vTemp.name << " (type=" << vTemp.type << ")" << std::endl;
      }
    }
    printSet(candidateList, "Candidates for deleting: ");
    while (!candidateList.empty()) {
      VertexNum chainIndex = *candidateList.begin();
      bool chainFirst = true;
      while (candidateList.count(chainIndex) > 0) {
        // if the vertex is already in the delete list, we are in a cycle and have to stop the while loop here.
        if (deleteList.count(chainIndex) > 0) {
          break;
        } else {
          deleteList.insert(chainIndex);
        }
        // determine the vertex before the chain
        if (chainFirst) {
          if (boost::in_degree(chainIndex, graph1) == 1) {
            ScheduleGraph::in_edge_iterator in_begin, in_end;
            boost::tie(in_begin, in_end) = in_edges(chainIndex, graph1);
            boost::graph_traits<ScheduleGraph>::vertex_descriptor source1 = source(*in_begin, graph1);
            std::cout << "In:  " << chainIndex << " " << *in_begin << " " << graph1[get(vertex_id, source1)].name << std::endl;
            chainBegin = get(vertex_id, source1);
          }
          chainFirst = false;
        }
        // determine the next vertex in the chain
        // if the next vertex is not in the chain, it is the chainEnd.
        if (boost::out_degree(chainIndex, graph1) == 1) {
          ScheduleGraph::out_edge_iterator out_begin, out_end;
          boost::tie(out_begin, out_end) = out_edges(chainIndex, graph1);
          boost::graph_traits<ScheduleGraph>::vertex_descriptor target1 = target(*out_begin, graph1);
          std::cout << "Out: " << chainIndex << " " << *out_begin << " " << graph1[get(vertex_id, target1)].name << std::endl;
          chainIndex = get(vertex_id, target1);
          if (candidateList.count(chainIndex) == 0) {
            chainEnd = chainIndex;
          }
        } else {
          break;
        }
      }
      deleteChain(graph1, candidateList, deleteList, chainBegin, chainEnd);
      chainBegin = ULONG_MAX;
      chainEnd = ULONG_MAX;
    }
          //~ boost::tie(out_begin, out_end) = out_edges(u, graph1);
          //~ typename boost::graph_traits<ScheduleGraph>::edge_descriptor e1 = *out_begin;
          //~ typename boost::graph_traits<ScheduleGraph>::vertex_descriptor u1 = target(e1, graph1);
          //~ ScheduleVertex uTemp = graph1[get(vertex_id, u)];
    saveSchedule("compact.dot", graph1, config);
    return 0;
}

void deleteChain(ScheduleGraph& graph1, VertexSet& candidates, VertexSet& deletes, VertexNum begin, VertexNum end) {
  std::cout << "Deleting chain in " << getGraphName(graph1) << " with " << deletes.size() << " vertices. Begin " << begin << ", end " << end << std::endl;
  configuration config1;
  if (deletes.size() > 1) {
    ScheduleVertex newVertex = ScheduleVertex();
    //~ newVertex.name = "new Vertex";
    // Delete vertices from chain.
    for (auto v1 : deletes) {
      newVertex.name = newVertex.name + " " + graph1[v1].name;
    }
    for (auto rit = deletes.rbegin(); rit != deletes.rend(); rit++) {
      VertexNum v1 = *rit;
      //~ std::string fileName = "compact-" + graph1[v1].name + ".dot";
      std::cout << "delete vertex " << v1 << ", " << newVertex.name << std::endl;
      clear_vertex(v1, graph1);
      remove_vertex(v1, graph1);
      //~ saveSchedule(fileName, graph1, config1);
    }
    // Add a new vertex.
    VertexNum newVertexNum = add_vertex(newVertex, graph1);
    std::cout << "newVertexNum: " << newVertexNum << ", '" << graph1[newVertexNum].name << "'" << std::endl;
    // Add two new edges if begin is defined or end is defined.
    if (begin != ULONG_MAX) {
      add_edge(begin, newVertexNum, graph1);
    }
    if (end != ULONG_MAX) {
      add_edge(newVertexNum, end, graph1);
    }
  }
  //~ printSet(deletes, " chain to delete: ");
  //~ printSet(candidates, " candidates before");
  for (auto e : deletes) {
    candidates.erase(e);
  }
  //~ printSet(candidates, " candidates after");
  deletes.clear();
  //~ printSet(deletes, " deletes empty");
}

void printSet(std::set<long unsigned int> set1, std::string title) {
  std::cout << title;
  for (auto e : set1) {
    std::cout << std::setw(4) << e;
  }
  std::cout << "." << std::endl;
}
