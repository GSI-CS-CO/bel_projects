#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <climits>

// constants for regular results of scheduleCompare
const int NOT_ISOMORPHIC = 1;
const int SUBGRAPH_ISOMORPHIC = 2;

// constants for irregular results of scheduleCompare and replaceChain
const int BAD_ARGUMENTS = 11;
const int MISSING_ARGUMENT = 12;
const int FILE_NOT_FOUND = 13;
const int USAGE_MESSAGE = 14;
const int PARSE_ERROR = 15;
const int PARSE_ERROR_GRAPHVIZ = 16;
const int TEST_SUCCESS = 17;
const int TEST_FAIL = 18;
const int VERSION_MESSAGE = 19;

struct configuration {
  // option -1: use first version of replaceChain
  bool firstVersion = false;
  // blocksSeparated, option -b
  bool blocksSeparated = false;
  // option -c
  bool check = false;
  // replaceChain: -c <n>
  int chainCount = INT_MAX;
  // option -n
  bool compareNames = true;
  // option -o: output file name
  std::string outputFile = std::string("");
  // option -w: overwrite output file
  bool overwrite = false;
  // internal option, used for replaceChain
  bool extraProperties = false;
  // internal option, used for replaceChain
  bool replaceChain = false;
  // option -s
  bool silent = false;
  // option -vv
  bool superverbose = false;
  // option -t
  bool test = false;
  // option -u - interpret "undefined" as empty string.
  bool undefinedAsEmpty = false;
  // option -v
  bool verbose = false;
};

#endif
