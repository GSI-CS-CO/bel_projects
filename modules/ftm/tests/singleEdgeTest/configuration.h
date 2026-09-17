#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <climits>

// constants for irregular results
const int BAD_ARGUMENTS = 11;
const int USAGE_MESSAGE = 14;
const int VERSION_MESSAGE = 19;

struct configuration {
  // option -q
  bool silent = false;
  // option -vv
  bool superverbose = false;
  // option -s
  bool generateMetaNodes = true;
  // option -v
  bool verbose = false;
};

#endif
