#include "parseSchedule.h"

#include <sys/stat.h>

#include <boost/graph/graphviz.hpp>
#include <istream>

#include "scheduleCompare.h"

inline bool file_exists(const std::string &file_name);

bool parseSchedule(std::string &dot_file, ScheduleGraph &g, boost::dynamic_properties &dp, configuration &config) {
  bool result = false;
  if (file_exists(dot_file)) {
    if (config.superverbose) {
      std::cout << "Reading graph from " << dot_file << ", ";
    }
    std::filebuf fileBuffer;
    fileBuffer.open(dot_file, std::ios::in);
    std::istream dot_stream(&fileBuffer);
    result = read_graphviz(dot_stream, g, dp, "name");
    if (config.superverbose) {
      std::cout << "read " << num_vertices(g) << " vertices, " << num_edges(g) << " edges." << std::endl;
    }
  } else {
    std::cerr << "Reading graph from " << dot_file << ", file not found." << std::endl;
    result = false;
  }
  return result;
}

inline bool file_exists(const std::string &file_name) {
  struct stat buffer;
  return (stat(file_name.c_str(), &buffer) == 0);
}