#ifndef SCHEDULE_COMPARE_H
#define SCHEDULE_COMPARE_H

struct configuration {
  bool verbose = false;
  bool superverbose = false;
  bool silent = false;
};

void usage(char* program);
int main(int argc, char* argv[]);

#endif
