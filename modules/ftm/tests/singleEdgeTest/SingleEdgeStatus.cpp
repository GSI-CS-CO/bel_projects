#include "SingleEdgeStatus.h"

#include <iomanip>   // for std::setfill, setw
#include <iostream>  // for std::cout

int SingleEdgeStatus::get(std::string key) { return status_counter[key]; }
void SingleEdgeStatus::increment(std::string key) { status_counter[key]++; }

void SingleEdgeStatus::printStatus() {
  // get the maximal length of a key
  uint length = 0;
  uint lengthInt = 0;
  for (auto iterator : status_counter) {
    if (iterator.first.length() > length) {
      length = iterator.first.length();
    }
    if (std::to_string(iterator.second).length() > lengthInt) {
      lengthInt = std::to_string(iterator.second).length();
    }
  }
  for (auto iterator : status_counter) {
    std::cout << std::setfill(' ') << std::setw(length + 1) << iterator.first << ": " << std::right << std::setw(lengthInt) << iterator.second << std::endl;
  }
}
