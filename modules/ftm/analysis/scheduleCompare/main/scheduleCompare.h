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

struct configuration {
  bool check = false;
  bool verbose = false;
  bool superverbose = false;
  bool silent = false;
};

void usage(char* program);
int main(int argc, char* argv[]);

#endif
