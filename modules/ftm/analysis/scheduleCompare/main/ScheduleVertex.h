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
  std::string toffs = std::string("");
  std::string tef = std::string("");
  std::string par = std::string("");
  std::string id = std::string("");
  std::string fid = std::string("");
  std::string gid = std::string("");
  std::string evtno = std::string("");
  std::string sid = std::string("");
  std::string bpid = std::string("");
  std::string beamin = std::string("");
  std::string bpcstart = std::string("");
  std::string reqnobeam = std::string("");
  std::string vacc = std::string("");
  std::string res = std::string("");
  std::string x = std::string("");

  int compare(const ScheduleVertex& v1, const ScheduleVertex& v2);
  // int compare(const ScheduleVertex& v2);

  inline bool operator==(const ScheduleVertex& rhs) { return compare(*this, rhs) == 0; }
  inline bool operator!=(const ScheduleVertex& rhs) { return compare(*this, rhs) != 0; }
  inline bool operator<(const ScheduleVertex& rhs) { return compare(*this, rhs) < 0; }
  inline bool operator>(const ScheduleVertex& rhs) { return compare(*this, rhs) > 0; }
  inline bool operator<=(const ScheduleVertex& rhs) { return compare(*this, rhs) <= 0; }
  inline bool operator>=(const ScheduleVertex& rhs) { return compare(*this, rhs) >= 0; }

 private:
  int compareTmsg(const ScheduleVertex& v1, const ScheduleVertex& v2);
};
#endif