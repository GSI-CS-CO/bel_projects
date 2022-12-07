#include "printSchedule.h"

void printSchedule(std::string header, ScheduleGraph& g, boost::dynamic_properties& dp, configuration& config) {
  if (config.verbose) {
    std::cout << std::endl << header << std::endl;
  }
  if (config.superverbose) {
    auto vertex_pair = vertices(g);
    for (auto iter = vertex_pair.first; iter != vertex_pair.second; iter++) {
      std::cout << "vertex " << *iter << ": " << g[*iter].name << std::endl;
    }

    auto edge_pair = edges(g);
    for (auto iter = edge_pair.first; iter != edge_pair.second; iter++) {
      std::cout << "edge " << source(*iter, g) << ": " << g[source(*iter, g)].name << " - " << target(*iter, g) << ": " << g[target(*iter, g)].name << std::endl;
    }
  }
  if (config.verbose) {
    boost::write_graphviz_dp(std::cout, g, dp, "name");
  }
}

void saveSchedule(std::string fileName, ScheduleGraph& g, configuration& config) {
  boost::dynamic_properties dp = setDynamicProperties(g, config);
  if (config.superverbose) {
    auto vertex_pair = vertices(g);
    for (auto iter = vertex_pair.first; iter != vertex_pair.second; iter++) {
      std::cout << "vertex " << *iter << ": " << g[*iter].name << std::endl;
    }

    auto edge_pair = edges(g);
    for (auto iter = edge_pair.first; iter != edge_pair.second; iter++) {
      std::cout << "edge " << source(*iter, g) << ": " << g[source(*iter, g)].name << " - " << target(*iter, g) << ": " << g[target(*iter, g)].name << std::endl;
    }
  }
  std::string graphName = getGraphName(g);
  setGraphName(g, graphName + std::string("-compact"));
  std::ofstream fText(fileName);
  boost::write_graphviz_dp(fText, g, dp, "name");
  fText.close();
}
