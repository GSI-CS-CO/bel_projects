#ifndef SCHEDULE_VERTEX_H
#define SCHEDULE_VERTEX_H

#include <string>

class ScheduleVertex {
 public:
  std::string name = std::string("");
  std::string label = std::string("");
  std::string pos = std::string("");
  std::string _draw_ = std::string("");
  std::string _ldraw_ = std::string("");
  std::string _hdraw_ = std::string("");
  std::string height = std::string("");
  std::string width = std::string("");
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
  std::string tvalid = std::string("");
  std::string tabs = std::string("");
  std::string target = std::string("");
  std::string dst = std::string("");
  std::string reps = std::string("");
  std::string prio = std::string("");
  std::string twait = std::string("");
  std::string wabs = std::string("");
  std::string clear = std::string("");
  std::string ovr = std::string("");
  std::string beamproc = std::string("");
  std::string pattern = std::string("");
  std::string patentry = std::string("");
  std::string patexit = std::string("");
  std::string bpentry = std::string("");
  std::string bpexit = std::string("");
  std::string permanent = std::string("");
  std::string thread = std::string("");
  std::string startoffs = std::string("");
  std::string section = std::string("");
  // for syntax check of dot files:
  std::string cpu = std::string("");
  std::string qty = std::string("");
  std::string vabs = std::string("");
  std::string flags = std::string("");
  std::string shape = std::string("");
  std::string penwidth = std::string("");
  std::string fillcolor = std::string("");
  std::string color = std::string("");
  std::string style = std::string("");

  // protocol of failed compare
  std::string protocol = std::string("");

  int compare(const ScheduleVertex& v1, const ScheduleVertex& v2);
  std::string printProtocol();
  void switchCompareNames(const bool flag);
  void switchUndefinedAsEmpty(const bool flag);
  operator std::string();
  inline bool operator==(const ScheduleVertex& rhs) { return compare(*this, rhs) == 0; }
  inline bool operator!=(const ScheduleVertex& rhs) { return compare(*this, rhs) != 0; }
  inline bool operator<(const ScheduleVertex& rhs) { return compare(*this, rhs) < 0; }
  inline bool operator>(const ScheduleVertex& rhs) { return compare(*this, rhs) > 0; }
  inline bool operator<=(const ScheduleVertex& rhs) { return compare(*this, rhs) <= 0; }
  inline bool operator>=(const ScheduleVertex& rhs) { return compare(*this, rhs) >= 0; }

 private:
  enum class valueType { STRING, BOOLEAN, HEX };
  bool compareNames = true;
  bool undefinedAsEmpty = false;
  int compareBlock(const ScheduleVertex& v1, const ScheduleVertex& v2);
  int compareFlow(const ScheduleVertex& v1, const ScheduleVertex& v2);
  int compareFlush(const ScheduleVertex& v1, const ScheduleVertex& v2);
  int compareGlobal(const ScheduleVertex& v1, const ScheduleVertex& v2);
  int compareListdst(const ScheduleVertex& v1, const ScheduleVertex& v2);
  int compareNoop(const ScheduleVertex& v1, const ScheduleVertex& v2);
  int compareQbuf(const ScheduleVertex& v1, const ScheduleVertex& v2);
  int compareQinfo(const ScheduleVertex& v1, const ScheduleVertex& v2);
  int compareSwitch(const ScheduleVertex& v1, const ScheduleVertex& v2);
  int compareOrigin(const ScheduleVertex& v1, const ScheduleVertex& v2);
  int compareStartthread(const ScheduleVertex& v1, const ScheduleVertex& v2);
  int compareTmsg(const ScheduleVertex& v1, const ScheduleVertex& v2);
  int compareWait(const ScheduleVertex& v1, const ScheduleVertex& v2);
  int compareBoolean(const std::string& bool1, const std::string& bool2);
  int compareHex(const std::string& hex1, const std::string& hex2);
  bool startsWith(std::string value, std::string start, bool caseSensitive);
  int compareValues(const std::string& value1, const std::string& value2, const std::string& key, valueType type);
};

std::ostream& operator<<(std::ostream& os, const ScheduleVertex& vertex);

#endif
