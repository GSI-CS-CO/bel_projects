#include "scheduleCompact.h"
#include "printSchedule.h"
#include "scheduleCompare.h"

int compactGraph(ScheduleGraph& graph1, configuration& config) {
    boost::property_map<ScheduleGraph, boost::vertex_index_t>::type vertex_id = get(boost::vertex_index, graph1);
    if (!config.silent) {
      std::cout << "Compacting " << getGraphName(graph1) << " with " << num_vertices(graph1) << " vertices." << std::endl;
    }
    ScheduleGraph::out_edge_iterator out_begin, out_end;
    ScheduleGraph::in_edge_iterator in_begin, in_end;
    int count = 0;
    BOOST_FOREACH (boost::graph_traits<ScheduleGraph>::vertex_descriptor v, vertices(graph1)) {
      if (boost::out_degree(v, graph1) == 1) {
        boost::tie(out_begin, out_end) = out_edges(v, graph1);
        typename boost::graph_traits<ScheduleGraph>::edge_descriptor e = *out_begin;
        typename boost::graph_traits<ScheduleGraph>::vertex_descriptor u = target(e, graph1);
        //~ boost::tie(in_begin, in_end) = in_edges(u, graph1);
        if (boost::in_degree(u, graph1) == 1 && boost::out_degree(u, graph1) == 1) {
          remove_edge(v,u,graph1);
          boost::tie(out_begin, out_end) = out_edges(u, graph1);
          typename boost::graph_traits<ScheduleGraph>::edge_descriptor e1 = *out_begin;
          typename boost::graph_traits<ScheduleGraph>::vertex_descriptor u1 = target(e1, graph1);
          remove_edge(u,u1,graph1);
          remove_vertex(u, graph1);
          add_edge(v,u1,graph1);
          if (!config.silent) {
            ScheduleVertex vTemp = graph1[get(vertex_id, v)];
            ScheduleVertex uTemp = graph1[get(vertex_id, u)];
            std::cout << count++ << " Candidate for compacting: " << vTemp.name << " (type=" << vTemp.type << ") label= " << uTemp.type << "." << uTemp.name << std::endl;
          }
        }
      }
    }
    saveSchedule("compact.dot", graph1, config);
    return 0;
}
