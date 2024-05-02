#ifndef SCHEDULE_EDGE_H
#define SCHEDULE_EDGE_H

#include <string>

#include "ScheduleVertex.h"

class ScheduleEdge {
 public:
  std::string name = std::string("");
  std::string type = std::string("");
  std::string _draw_ = std::string("");
  std::string _hdraw_ = std::string("");
  std::string pos = std::string("");
  std::string color = std::string("");
  std::string fieldhead = std::string("");
  std::string fieldtail = std::string("");
  std::string fieldwidth = std::string("");
  ScheduleVertex& vertex_source = *(new ScheduleVertex());
  ScheduleVertex& vertex_target = *(new ScheduleVertex());

  // protocol of failed compare
  std::string protocol = std::string("");

  int compare(const ScheduleEdge& e1, const ScheduleEdge& e2);
  operator std::string();
  std::string printProtocol();

  inline bool operator==(const ScheduleEdge& rhs) { return compare(*this, rhs) == 0; }
  inline bool operator!=(const ScheduleEdge& rhs) { return compare(*this, rhs) != 0; }
  inline bool operator<(const ScheduleEdge& rhs) { return compare(*this, rhs) < 0; }
  inline bool operator>(const ScheduleEdge& rhs) { return compare(*this, rhs) > 0; }
  inline bool operator<=(const ScheduleEdge& rhs) { return compare(*this, rhs) <= 0; }
  inline bool operator>=(const ScheduleEdge& rhs) { return compare(*this, rhs) >= 0; }
};

std::ostream& operator<<(std::ostream& os, const ScheduleEdge& edge);

#endif
