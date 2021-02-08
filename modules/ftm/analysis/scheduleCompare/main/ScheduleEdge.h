#ifndef SCHEDULE_EDGE_H
#define SCHEDULE_EDGE_H

#include <string>

class ScheduleEdge {
 public:
  std::string name = std::string("");
  std::string type = std::string("");
  std::string color = std::string("");

  int compare(const ScheduleEdge& e1, const ScheduleEdge& e2);

  inline bool operator==(const ScheduleEdge& rhs) { return compare(*this, rhs) == 0; }
  inline bool operator!=(const ScheduleEdge& rhs) { return compare(*this, rhs) != 0; }
  inline bool operator<(const ScheduleEdge& rhs) { return compare(*this, rhs) < 0; }
  inline bool operator>(const ScheduleEdge& rhs) { return compare(*this, rhs) > 0; }
  inline bool operator<=(const ScheduleEdge& rhs) { return compare(*this, rhs) <= 0; }
  inline bool operator>=(const ScheduleEdge& rhs) { return compare(*this, rhs) >= 0; }
};
#endif
