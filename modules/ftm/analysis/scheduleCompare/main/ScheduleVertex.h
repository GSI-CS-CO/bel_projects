#ifndef SCHEDULE_VERTEX_H
#define SCHEDULE_VERTEX_H

#include <string>

class ScheduleVertex {
 public:
  std::string name = std::string("");
  std::string type = std::string("");
  std::string tperiod = std::string("");
  std::string qlo = std::string("");
  std::string qhi = std::string("");
  std::string qil = std::string("");
  std::string x = std::string("");

  int compare(const ScheduleVertex& v1, const ScheduleVertex& v2);
  int compare(const ScheduleVertex& v2);

  inline bool operator==(const ScheduleVertex& rhs) { return compare(*this, rhs) == 0; }
  inline bool operator!=(const ScheduleVertex& rhs) { return compare(*this, rhs) != 0; }
  inline bool operator<(const ScheduleVertex& rhs) { return compare(*this, rhs) < 0; }
  inline bool operator>(const ScheduleVertex& rhs) { return compare(*this, rhs) > 0; }
  inline bool operator<=(const ScheduleVertex& rhs) { return compare(*this, rhs) <= 0; }
  inline bool operator>=(const ScheduleVertex& rhs) { return compare(*this, rhs) >= 0; }
};
#endif