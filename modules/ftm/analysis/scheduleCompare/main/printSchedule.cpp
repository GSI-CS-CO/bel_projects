#include "printSchedule.h"

void printSchedule(std::string header, ScheduleGraph& g, boost::dynamic_properties& dp, configuration& config) {
  if (config.superverbose) {
    auto vertex_pair = vertices(g);
    for (auto iter = vertex_pair.first; iter != vertex_pair.second; iter++) {
      std::cout << "vertex " << *iter << std::endl;
    }

    auto edge_pair = edges(g);
    for (auto iter = edge_pair.first; iter != edge_pair.second; iter++) {
      std::cout << "edge " << source(*iter, g) << " - " << target(*iter, g) << std::endl;
    }
  }

  if (config.verbose) {
    std::cout << std::endl << header << std::endl;
    boost::write_graphviz_dp(std::cout, g, dp, "name");
  }
}
