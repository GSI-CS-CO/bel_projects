#include <set>

#include "scheduleCompact.h"
#include "printSchedule.h"
#include "scheduleCompare.h"

typedef long unsigned int VertexNum;
typedef std::set<VertexNum> VertexSet;

void printSet(VertexSet set1, std::string title);

void deleteEdges(ScheduleGraph& graph1, VertexSet& candidates, VertexSet& deletes, VertexSet& deleteVertices, VertexNum begin, VertexNum end);

int compactGraph(ScheduleGraph& graph1, configuration& config) {
    if (!config.silent) {
      std::cout << "Compacting " << getGraphName(graph1) << " with " << num_vertices(graph1) << " vertices." << std::endl;
    }
    boost::property_map<ScheduleGraph, boost::vertex_index_t>::type vertex_id = get(boost::vertex_index, graph1);
    VertexSet candidateList = {};
    VertexSet deleteList = {};
    VertexSet deleteVertices = {};
    VertexNum chainBegin = ULONG_MAX;
    VertexNum chainEnd = ULONG_MAX;
    BOOST_FOREACH (boost::graph_traits<ScheduleGraph>::vertex_descriptor v, vertices(graph1)) {
      if (boost::in_degree(v, graph1) <= 1 && boost::out_degree(v, graph1) <= 1) {
        candidateList.insert(v);
      }
    }
    printSet(candidateList, "Candidates for deleting: ");
    while (!candidateList.empty()) {
      VertexNum chainIndex = *candidateList.begin();
      bool chainFirst = true;
      while (boost::in_degree(chainIndex, graph1) <= 1 && boost::out_degree(chainIndex, graph1) <= 1) {
        // if the vertex is already in the delete list, we are in a cycle and have to stop the while loop here.
        std::cout << "chainIndex " << chainIndex;
        if (deleteList.count(chainIndex) > 0) {
          chainEnd = chainBegin;
          std::cout << ", chainBegin: " << chainBegin << ", chainEnd: " << chainEnd;
          deleteList.erase(chainBegin);
          candidateList.erase(chainBegin);
          break;
        } else {
          deleteList.insert(chainIndex);
          candidateList.erase(chainIndex);
        }
        std::cout << std::endl;
        printSet(deleteList, "To delete:");
        printSet(candidateList, "Candidates: ");
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
          std::cout << "Out: " << chainIndex << " " << *out_begin << " " << graph1[get(vertex_id, target1)].name << ", " << get(vertex_id, target1) << std::endl;
          chainIndex = get(vertex_id, target1);
          if (boost::in_degree(chainIndex, graph1) > 1) {
            chainEnd = chainIndex;
            std::cout << "Set chainEnd, chainBegin: " << chainBegin << ", chainEnd: " << chainEnd << std::endl;
          }
        } else {
          break;
        }
      }
      deleteEdges(graph1, candidateList, deleteList, deleteVertices, chainBegin, chainEnd);
      chainBegin = ULONG_MAX;
      chainEnd = ULONG_MAX;
    }
    printSet(deleteVertices, "Vertices to delete");
    for (auto reverseIterator = deleteVertices.rbegin(); reverseIterator != deleteVertices.rend(); reverseIterator++) {
      remove_vertex(*reverseIterator, graph1);
    }
    saveSchedule("compact.dot", graph1, config);
    return 0;
}

void deleteEdges(ScheduleGraph& graph1, VertexSet& candidates, VertexSet& deletes, VertexSet& deleteVertices, VertexNum begin, VertexNum end) {
  std::cout << "Deleting chain in " << getGraphName(graph1) << " with " << deletes.size() << " vertices. Begin " << begin << ", end " << end << std::endl;
  configuration config1;
  if (deletes.size() > 1) {
    ScheduleVertex newVertex = ScheduleVertex();
    // Delete vertices from chain.
    for (auto v1 : deletes) {
      if (newVertex.name.size() == 0) {
        newVertex.name = graph1[v1].name;
      } else {
        newVertex.name = newVertex.name + "\n" + graph1[v1].name;
      }
    }
    for (auto reverseIterator = deletes.rbegin(); reverseIterator != deletes.rend(); reverseIterator++) {
      VertexNum v1 = *reverseIterator;
      std::cout << "delete vertex " << v1 << std::endl;
      clear_vertex(v1, graph1);
      deleteVertices.insert(v1);
      //~ remove_vertex(v1, graph1);
      //~ begin--;
      //~ end--;
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
