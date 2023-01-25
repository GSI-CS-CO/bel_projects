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

template < typename Graph >
class ScheduleGraphPropertiesWriter {
public:
  ScheduleGraphPropertiesWriter(
    const boost::dynamic_properties& dp, const Graph& g)
  : g(&g), dp(&dp)
  {
  }

  void operator()(std::ostream& out) const
  {
    out << "graph [\n";
    for (boost::dynamic_properties::const_iterator i = dp->begin(); i != dp->end(); ++i) {
      if (typeid(Graph*) == i->second->key() && i->second->get_string(const_cast< Graph* >(g)).size() > 0) {
        // const_cast here is to match interface used in read_graphviz
        out << i->first << "="
            << boost::escape_dot_string(
                   i->second->get_string(const_cast< Graph* >(g)))
            << "\n";
      }
    }
    out << "]\n";
  }

private:
    const Graph* g;
    const boost::dynamic_properties* dp;
};

class DynamicPropertiesWriter
{
public:
  DynamicPropertiesWriter(const boost::dynamic_properties& dp) : dp(&dp) {}

  template < typename Descriptor >
  void operator()(std::ostream& out, Descriptor key) const {
    bool first = true;
    for (boost::dynamic_properties::const_iterator i = dp->begin(); i != dp->end(); ++i) {
      if (typeid(key) == i->second->key() && i->second->get_string(key).size() > 0) {
        out << (first ? " [" : ", ");
        first = false;
        out << i->first << "=" << boost::escape_dot_string(i->second->get_string(key));
      }
    }
    if (!first) {
      out << "]";
    }
  }

private:
  const boost::dynamic_properties* dp;
};

class DynamicVertexPropertiesWriter
{
public:
  DynamicVertexPropertiesWriter(
      const boost::dynamic_properties& dp, const std::string& node_id)
  : dp(&dp), node_id(&node_id) {}

  template < typename Descriptor >
  void operator()(std::ostream& out, Descriptor key) const {
    bool first = true;
    for (boost::dynamic_properties::const_iterator i = dp->begin(); i != dp->end(); ++i) {
      if (typeid(key) == i->second->key() && i->first != *node_id && i->second->get_string(key).size() > 0) {
        out << (first ? " [" : ", ");
        first = false;
        out << i->first << "=" << boost::escape_dot_string(i->second->get_string(key));
      }
    }
    if (!first) {
        out << "]";
    }
  }

private:
  const boost::dynamic_properties* dp;
  const std::string* node_id;
};

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
  //~ boost::write_graphviz_dp(fText, g, dp, "name");
  typedef typename boost::graph_traits< ScheduleGraph >::vertex_descriptor Vertex;
  boost::write_graphviz(fText, g, DynamicVertexPropertiesWriter(dp, "name"), DynamicPropertiesWriter(dp),
      ScheduleGraphPropertiesWriter<ScheduleGraph>(dp, g), boost::graph::detail::node_id_property_map< Vertex >(dp, "name"));
  fText.close();
}
