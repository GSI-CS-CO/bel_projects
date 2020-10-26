#ifndef _SINGLEEDGESTATUS_H_
#define _SINGLEEDGESTATUS_H_

#include <map>

class SingleEdgeStatus {
 private:
  std::map<std::string, int> status_counter;

 public:
  int get(std::string key);
  void increment(std::string key);
  void printStatus();
};

#endif
