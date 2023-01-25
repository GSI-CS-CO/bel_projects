#ifndef CONFIGURATION_H
#define CONFIGURATION_H

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

struct configuration {
  // option -c
  bool check = false;
  // replaceChain: -c <n>
  int chainCount = 0;
  // option -n
  bool compareNames = true;
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
  // option -v
  bool verbose = false;
};

#endif
