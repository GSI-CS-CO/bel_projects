#include <set>

#include "scheduleCompact.h"
#include "printSchedule.h"
#include "scheduleCompare.h"

typedef long unsigned int VertexNum;
typedef boost::property_map<ScheduleGraph, boost::vertex_index_t>::type VertexId;
typedef std::set<VertexNum> VertexSet;
typedef boost::graph_traits<ScheduleGraph>::edge_descriptor EdgeDescriptor;
typedef boost::graph_traits<ScheduleGraph>::vertex_descriptor VertexDescriptor;

void printSet(VertexSet set1, std::string title);

void deleteChain(ScheduleGraph& graph1, VertexSet& candidates, VertexSet& deletes, VertexSet& deleteVertices, VertexNum begin, VertexNum end, EdgeDescriptor first, EdgeDescriptor last);

VertexNum topOfChain(VertexNum vertex, ScheduleGraph& graph1, VertexId id);

int compactGraph(ScheduleGraph& graph1, configuration& config) {
    if (!config.silent) {
      std::cout << "Compacting " << getGraphName(graph1) << " with " << num_vertices(graph1) << " vertices." << std::endl;
    }
    VertexId vertex_id = get(boost::vertex_index, graph1);
    VertexSet candidateList = {};
    VertexSet deleteList = {};
    VertexSet deleteVertices = {};
    VertexNum chainBegin = ULONG_MAX;
    VertexNum chainEnd = ULONG_MAX;
    BOOST_FOREACH (VertexDescriptor v, vertices(graph1)) {
      if (graph1[v].label.size() == 0) {
        graph1[v].label = graph1[v].name;
      }
      if (boost::in_degree(v, graph1) <= 1 && boost::out_degree(v, graph1) <= 1) {
        candidateList.insert(v);
      }
    }
    if (config.verbose) {
      printSet(candidateList, "Candidates for deleting: ");
    }
    while (!candidateList.empty()) {
      VertexNum chainIndex = topOfChain(*candidateList.begin(), graph1, vertex_id);
      bool chainFirst = true;
      EdgeDescriptor edgeFirst;
      EdgeDescriptor edgeLast;
      while (boost::in_degree(chainIndex, graph1) <= 1 && boost::out_degree(chainIndex, graph1) <= 1) {
        // if the vertex is already in the delete list, we are in a cycle and have to stop the while loop here.
        //~ std::cout << "chainIndex " << chainIndex;
        if (deleteList.count(chainIndex) > 0) {
          chainEnd = chainBegin;
          //~ std::cout << ", chainBegin: " << chainBegin << ", chainEnd: " << chainEnd;
          deleteList.erase(chainBegin);
          candidateList.erase(chainBegin);
          break;
        } else {
          deleteList.insert(chainIndex);
          candidateList.erase(chainIndex);
        }
        //~ std::cout << std::endl;
        if (config.verbose) {
          printSet(deleteList, "To delete:");
          printSet(candidateList, "Candidates: ");
        }
        // determine the vertex before the chain
        if (chainFirst) {
          if (boost::in_degree(chainIndex, graph1) == 1) {
            ScheduleGraph::in_edge_iterator in_begin, in_end;
            boost::tie(in_begin, in_end) = in_edges(chainIndex, graph1);
            VertexDescriptor source1 = source(*in_begin, graph1);
            edgeFirst = *in_begin;
            //~ std::cout << "In:  " << chainIndex << " " << *in_begin << " " << graph1[get(vertex_id, source1)].name << std::endl;
            chainBegin = get(vertex_id, source1);
          }
          chainFirst = false;
        }
        // determine the next vertex in the chain
        // if the next vertex is not in the chain, it is the chainEnd.
        if (boost::out_degree(chainIndex, graph1) == 1) {
          ScheduleGraph::out_edge_iterator out_begin, out_end;
          boost::tie(out_begin, out_end) = out_edges(chainIndex, graph1);
          VertexDescriptor target1 = target(*out_begin, graph1);
          edgeLast = *out_begin;
          //~ std::cout << "Out: " << chainIndex << " " << *out_begin << " " << graph1[get(vertex_id, target1)].name << ", " << get(vertex_id, target1) << std::endl;
          chainIndex = get(vertex_id, target1);
          if (boost::in_degree(chainIndex, graph1) > 1 || boost::out_degree(chainIndex, graph1) > 1) {
            chainEnd = chainIndex;
            //~ std::cout << "Set chainEnd, chainBegin: " << chainBegin << ", chainEnd: " << chainEnd << std::endl;
          }
        } else {
          break;
        }
      }
      deleteChain(graph1, candidateList, deleteList, deleteVertices, chainBegin, chainEnd, edgeFirst, edgeLast);
      chainBegin = ULONG_MAX;
      chainEnd = ULONG_MAX;
    }
    if (config.verbose) {
      printSet(deleteVertices, "Vertices to delete");
    }
    for (auto reverseIterator = deleteVertices.rbegin(); reverseIterator != deleteVertices.rend(); reverseIterator++) {
      remove_vertex(*reverseIterator, graph1);
    }
    saveSchedule("compact.dot", graph1, config);
    return 0;
}

void deleteChain(ScheduleGraph& graph1, VertexSet& candidates, VertexSet& deletes, VertexSet& deleteVertices, VertexNum begin, VertexNum end, EdgeDescriptor first, EdgeDescriptor last) {
  std::cout << "Deleting chain in " << getGraphName(graph1) << " with " << deletes.size() << " vertices. Begin " << begin << ", end " << end << std::endl;
  if (deletes.size() > 1) {
    ScheduleVertex newVertex = ScheduleVertex();
    // Concate names from vertices in chain and use this as name for new vertex.
    //~ std::cout << "0 newVertex: " << newVertex.name << "'" << std::endl;
    auto lastVertex = *deletes.rbegin();
    for (auto v1 : deletes) {
      if (newVertex.name.size() == 0) {
        newVertex.name = graph1[v1].name;
        newVertex.label = graph1[v1].label;
        //~ std::cout << "0: " << v1 << ", " << graph1[v1].name << ", pos=" << graph1[v1].pos << std::endl;
        newVertex.pos = graph1[v1].pos;
        newVertex.height = graph1[v1].height;
        newVertex.width = graph1[v1].width;
        newVertex._draw_ = graph1[v1]._draw_;
        newVertex._hdraw_ = graph1[v1]._hdraw_;
        newVertex._ldraw_ = graph1[v1]._ldraw_;
        newVertex.style = graph1[v1].style;
        newVertex.penwidth = graph1[v1].penwidth;
        newVertex.shape = graph1[v1].shape;
        newVertex.fillcolor = graph1[v1].fillcolor;
        newVertex.color = graph1[v1].color;
        newVertex.pattern = graph1[v1].pattern;
      } else {
        if (deletes.size() < 4) {
          newVertex.name = newVertex.name + "\n" + graph1[v1].name;
          if (graph1[v1].label.size() > 0) {
            newVertex.label = newVertex.label + "\n" + graph1[v1].label;
          }
        } else {
          if (v1 == lastVertex) {
            newVertex.name = newVertex.name + "\n...\n" + graph1[v1].name;
            if (graph1[v1].label.size() > 0) {
              newVertex.label = newVertex.label + "\n...\n" + graph1[v1].label;
            }
          }
        }
        //~ std::cout << "x: " << v1 << ", " << graph1[v1].pos << std::endl;
      }
    }
    // Add a new vertex.
    if (newVertex.label.size() == 0) {
      newVertex.label = newVertex.name;
    }
    VertexNum newVertexNum = add_vertex(newVertex, graph1);
    // Add two new edges if begin is defined or end is defined.
    if (begin != ULONG_MAX) {
      std::pair<boost::graph_traits<ScheduleGraph>::edge_descriptor, bool> beginEdge = add_edge(begin, newVertexNum, graph1);
      //~ std::cout << "Edge: " << beginEdge.first << ", " << beginEdge.second << ", pos '" << boost::get("pos", graph1, beginEdge.first) << "', begin: " << begin << ", new: " << newVertexNum << std::endl;
      graph1[beginEdge.first].pos = graph1[first].pos;
      graph1[beginEdge.first]._draw_ = graph1[first]._draw_;
      graph1[beginEdge.first]._hdraw_ = graph1[first]._hdraw_;
      graph1[beginEdge.first].type = graph1[first].type;
      graph1[beginEdge.first].color = graph1[first].color;
      //~ std::cout << "Edge: " << beginEdge.first << ", " << beginEdge.second << ", pos '" << graph1[beginEdge.first].pos << "', begin: " << begin << ", new: " << newVertexNum << std::endl;
    }
    if (end != ULONG_MAX) {
      std::pair<boost::graph_traits<ScheduleGraph>::edge_descriptor, bool> endEdge = add_edge(newVertexNum, end, graph1);
      graph1[endEdge.first].pos = graph1[last].pos;
      graph1[endEdge.first]._draw_ = graph1[last]._draw_;
      graph1[endEdge.first]._hdraw_ = graph1[last]._hdraw_;
      graph1[endEdge.first].type = graph1[last].type;
      graph1[endEdge.first].color = graph1[last].color;
    }
    //~ std::cout << "1 newVertexNum: " << newVertexNum << ", '" << graph1[newVertexNum].name << "', '" << newVertex.name << "'" << std::endl;
    // Delete edges for vertices in chain.
    for (auto reverseIterator = deletes.rbegin(); reverseIterator != deletes.rend(); reverseIterator++) {
      VertexNum v1 = *reverseIterator;
      //~ std::cout << "delete vertex " << v1 << std::endl;
      clear_vertex(v1, graph1);
      deleteVertices.insert(v1);
      //~ remove_vertex(v1, graph1);
      //~ begin--;
      //~ end--;
    }
  }
  deletes.clear();
  //~ printSet(deletes, " deletes empty");
}

VertexNum topOfChain(VertexNum vertex, ScheduleGraph& graph1, VertexId id) {
  // get the first ("top") vertex of a chain.
  VertexNum topVertex = vertex;
  if (boost::in_degree(topVertex, graph1) == 1) {
    do {
      ScheduleGraph::in_edge_iterator in_begin, in_end;
      boost::tie(in_begin, in_end) = in_edges(topVertex, graph1);
      VertexDescriptor source1 = source(*in_begin, graph1);
      VertexNum tempVertex = get(id, source1);
      //~ std::cout << "topOfChain: " << topVertex << " " << *in_begin << " " << graph1[tempVertex].name << ", " << tempVertex << std::endl;
      if (boost::in_degree(tempVertex, graph1) <= 1 && boost::out_degree(tempVertex, graph1) <= 1) {
        topVertex = tempVertex;
      } else {
        break;
      }
    } while (boost::in_degree(topVertex, graph1) == 1 && topVertex != vertex);
  }
  return topVertex;
}

void printSet(std::set<long unsigned int> set1, std::string title) {
  std::cout << title;
  for (auto e : set1) {
    std::cout << std::setw(4) << e;
  }
  std::cout << "." << std::endl;
}
