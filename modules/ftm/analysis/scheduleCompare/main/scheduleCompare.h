#ifndef SCHEDULE_COMPARE_H
#define SCHEDULE_COMPARE_H

// constants for regular results of scheduleCompare
const int NOT_ISOMORPHIC = 1;
const int SUBGRAPH_ISOMORPHIC = 2;

// constants for irregular results of scheduleCompare
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
  // option -n
  bool CompareNames = true;
  // option -p
  bool compact = false;
  // internal option, used for compact graphs
  bool extraProperties = false;
  // option -s
  bool silent = false;
  // option -vv
  bool superverbose = false;
  // option -t
  bool test = false;
  // option -v
  bool verbose = false;
};

void usage(char* program);
int main(int argc, char* argv[]);

#endif
