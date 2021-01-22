#include "parseSchedule.h"

#include <boost/graph/graphviz.hpp>
#include <istream>

#include "scheduleCompare.h"

bool parseSchedule(std::string &dot_file, ScheduleGraph &g, boost::dynamic_properties &dp, configuration &config) {
  if (config.superverbose) {
    std::cout << "Reading graph from " << dot_file << std::endl;
  }
  std::filebuf fileBuffer;
  fileBuffer.open(dot_file, std::ios::in);
  std::istream dot_stream(&fileBuffer);
  return read_graphviz(dot_stream, g, dp, "name");
}
