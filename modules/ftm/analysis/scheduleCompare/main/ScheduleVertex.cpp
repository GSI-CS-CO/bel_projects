#include "ScheduleVertex.h"

#include <algorithm>
#include <iostream>
#include <sstream>

std::ostream& operator<<(std::ostream& os, const ScheduleVertex& vertex) {
  os << vertex.name;
  return os;
}

ScheduleVertex::operator std::string() {
  return this->name;
}

void ScheduleVertex::switchCompareNames(const bool flag){
  this->compareNames = flag;
}

void ScheduleVertex::switchUndefinedAsEmpty(const bool flag){
  this->undefinedAsEmpty = flag;
}

int ScheduleVertex::compare(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  //~ std::cout << "--V " << v1.name << ", " << v2.name << " | " << v1.protocol << std::endl;
  if (!v1.compareNames || v1.name == v2.name) {
    if (v1.type == "") {
      return 0;
    } else if (v1.type == v2.type) {
      if ("block" == v1.type || "blockalign" == v1.type) {
        return compareBlock(v1, v2);
      } else if ("flow" == v1.type) {
        return compareFlow(v1, v2);
      } else if ("flush" == v1.type) {
        return compareFlush(v1, v2);
      } else if ("listdst" == v1.type) {
        return compareListdst(v1, v2);
      } else if ("noop" == v1.type) {
        return compareNoop(v1, v2);
      } else if ("qbuf" == v1.type) {
        return compareQbuf(v1, v2);
      } else if ("qinfo" == v1.type) {
        return compareQinfo(v1, v2);
      } else if ("switch" == v1.type) {
        return compareSwitch(v1, v2);
      } else if ("origin" == v1.type) {
        return compareOrigin(v1, v2);
      } else if ("startthread" == v1.type) {
        return compareStartthread(v1, v2);
      } else if ("tmsg" == v1.type) {
        //~ std::cout << "--V tmsg " << compareTmsg(v1, v2) << std::endl;
        return compareTmsg(v1, v2);
      } else if ("wait" == v1.type) {
        return compareWait(v1, v2);
      } else if ("global" == v1.type) {
        return compareGlobal(v1, v2);
      } else {
        return -1;
      }
    } else {
      return v1.type.compare(v2.type);
    }
  } else {
    bool result = !v1.compareNames || v1.name.compare(v2.name);
    if (result != 0) {
      protocol += " != '" + v2.name + "';";
    }
    return result;
  }
}

int ScheduleVertex::compareBlock(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  int result = compareValues(v1.tperiod, v2.tperiod, "tperiod", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.cpu, v2.cpu, "cpu", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.patentry, v2.patentry, "patentry", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.patexit, v2.patexit, "patexit", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.pattern, v2.pattern, "pattern", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.bpentry, v2.bpentry, "bpentry", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.bpexit, v2.bpexit, "bpexit", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.beamproc, v2.beamproc, "beamproc", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.qlo, v2.qlo, "qlo", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.qhi, v2.qhi, "qhi", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.qil, v2.qil, "qil", valueType::BOOLEAN);
  return result;
}

int ScheduleVertex::compareFlow(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  int result = compareValues(v1.pattern, v2.pattern, "pattern", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.cpu, v2.cpu, "cpu", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.beamproc, v2.beamproc, "beamproc", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.patentry, v2.patentry, "patentry", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.bpentry, v2.bpentry, "bpentry", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.qlo, v2.qlo, "qlo", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.qhi, v2.qhi, "qhi", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.qil, v2.qil, "qil", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.target, v2.target, "target", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.tvalid, v2.tvalid, "tvalid", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.toffs, v2.toffs, "toffs", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.tabs, v2.tabs, "tabs", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.prio, v2.prio, "prio", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.reps, v2.reps, "reps", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.qty, v2.qty, "qty", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.permanent, v2.permanent, "permanent", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.dst, v2.dst, "dst", valueType::STRING);
  return result;
}

int ScheduleVertex::compareFlush(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  int result = compareValues(v1.target, v2.target, "target", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.tvalid, v2.tvalid, "tvalid", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.tabs, v2.tabs, "tabs", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.clear, v2.clear, "clear", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.ovr, v2.ovr, "ovr", valueType::STRING);
  return result;
}

int ScheduleVertex::compareGlobal(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  int result = compareValues(v1.pattern, v2.pattern, "pattern", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.cpu, v2.cpu, "cpu", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.section, v2.section, "section", valueType::STRING);
  return result;
}

int ScheduleVertex::compareListdst(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  int result = compareValues(v1.pattern, v2.pattern, "pattern", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.cpu, v2.cpu, "cpu", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.beamproc, v2.beamproc, "beamproc", valueType::STRING);
  return result;
}

int ScheduleVertex::compareNoop(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  int result = compareValues(v1.pattern, v2.pattern, "pattern", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.cpu, v2.cpu, "cpu", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.tvalid, v2.tvalid, "tvalid", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.vabs, v2.vabs, "vabs", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.toffs, v2.toffs, "toffs", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.prio, v2.prio, "prio", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.qty, v2.qty, "qty", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.beamproc, v2.beamproc, "beamproc", valueType::STRING);
  return result;
}

int ScheduleVertex::compareQbuf(const ScheduleVertex& v1, const ScheduleVertex& v2) {
    int result = compareValues(v1.pattern, v2.pattern, "pattern", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.cpu, v2.cpu, "cpu", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.beamproc, v2.beamproc, "beamproc", valueType::STRING);
  return result;
 }

int ScheduleVertex::compareQinfo(const ScheduleVertex& v1, const ScheduleVertex& v2) {   int result = compareValues(v1.pattern, v2.pattern, "pattern", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.cpu, v2.cpu, "cpu", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.beamproc, v2.beamproc, "beamproc", valueType::STRING);
  return result;
 }

int ScheduleVertex::compareSwitch(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  int result = -1;
  result = compareValues(v1.target, v2.target, "target", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.tvalid, v2.tvalid, "tvalid", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.tabs, v2.tabs, "tabs", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.dst, v2.dst, "dst", valueType::STRING);
  return result;
}

int ScheduleVertex::compareOrigin(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  int result = -1;
  result = compareValues(v1.pattern, v2.pattern, "pattern", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.thread, v2.thread, "thread", valueType::STRING);
  return result;
}

int ScheduleVertex::compareStartthread(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  int result = -1;
  result = compareValues(v1.pattern, v2.pattern, "pattern", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.startoffs, v2.startoffs, "startoffs", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.thread, v2.thread, "thread", valueType::STRING);
  return result;
}

int ScheduleVertex::compareTmsg(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  int result = compareValues(v1.pattern, v2.pattern, "pattern", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.cpu, v2.cpu, "cpu", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.beamproc, v2.beamproc, "beamproc", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.patentry, v2.patentry, "patentry", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.bpentry, v2.bpentry, "bpentry", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.toffs, v2.toffs, "toffs", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.tef, v2.tef, "tef", valueType::HEX);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.par, v2.par, "par", valueType::HEX);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.id, v2.id, "id", valueType::HEX);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.fid, v2.fid, "fid", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.gid, v2.gid, "gid", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.evtno, v2.evtno, "evtno", valueType::HEX);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.sid, v2.sid, "sid", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.bpid, v2.bpid, "bpid", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.beamin, v2.beamin, "beamin", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.bpcstart, v2.bpcstart, "bpcstart", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.reqnobeam, v2.reqnobeam, "reqnobeam", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.vacc, v2.vacc, "vacc", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.res, v2.res, "res", valueType::STRING);
  return result;
}

int ScheduleVertex::compareWait(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  int result = compareValues(v1.target, v2.target, "target", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.tvalid, v2.tvalid, "tvalid", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.tabs, v2.tabs, "tabs", valueType::BOOLEAN);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.twait, v2.twait, "twait", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.wabs, v2.wabs, "wabs", valueType::BOOLEAN);
  return result;
}

int ScheduleVertex::compareBoolean(const std::string& bool1, const std::string& bool2) {
  if (("1" == bool1) || ("True" == bool1) || ("true" == bool1)) {
    return !(("1" == bool2) || ("True" == bool2) || ("true" == bool2));
  } else if (bool1.empty() || ("0" == bool1) || ("False" == bool1) || ("false" == bool1)) {
    return !(bool2.empty() || ("0" == bool2) || ("False" == bool2) || ("false" == bool2));
  } else {
    return -1;
  }
}

int ScheduleVertex::compareHex(const std::string& inHex1, const std::string& inHex2) {
  std::string hex1 = (inHex1.empty()) ? "0" : inHex1;
  std::string hex2 = (inHex2.empty()) ? "0" : inHex2;
  if (startsWith(hex1, "0x", false) && startsWith(hex2, "0X", false)) {
    // hex1 and hex2 are hexadecimal numbers
    unsigned long x1;
    unsigned long x2;
    std::stringstream hexStream1;
    std::stringstream hexStream2;
    hexStream1 << std::hex << hex1;
    hexStream2 << std::hex << hex2;
    hexStream1 >> x1;
    hexStream2 >> x2;
    if (x1 < x2) {
      return -1;
    } else if (x1 > x2) {
      return 1;
    } else {
      return 0;
    }
  } else if (startsWith(hex1, "0x", false) && !startsWith(hex2, "0X", false)) {
    // hex1 is a hexadecimal number, hex2 is a decimal number
    unsigned long x1;
    unsigned long x2;
    std::stringstream hexStream1;
    std::stringstream stream2;
    hexStream1 << std::hex << hex1;
    stream2 << hex2;
    hexStream1 >> x1;
    stream2 >> x2;
    if (x1 < x2) {
      return -1;
    } else if (x1 > x2) {
      return 1;
    } else {
      return 0;
    }
  } else if (!startsWith(hex1, "0x", false) && startsWith(hex2, "0X", false)) {
    // hex1 is a decimal number, hex2 is a hexadecimal number
    unsigned long x1;
    unsigned long x2;
    std::stringstream stream1;
    std::stringstream hexStream2;
    stream1 << hex1;
    hexStream2 << std::hex << hex2;
    stream1 >> x1;
    hexStream2 >> x2;
    if (x1 < x2) {
      return -1;
    } else if (x1 > x2) {
      return 1;
    } else {
      return 0;
    }
  } else {
    return hex1.compare(hex2);
  }
}

int ScheduleVertex::compareValues(const std::string& value1, const std::string& value2, const std::string& key, valueType type) {
  int result = -1;
  if (type == valueType::BOOLEAN) {
    result = compareBoolean(value1, value2);
  } else if (type == valueType::HEX) {
    result = compareHex(value1, value2);
  } else if (type == valueType::STRING) {
    result = value1.compare(value2);
    if (result != 0 && this->undefinedAsEmpty) {
      // when value1 and value2 are both "undefined", result == 0 and no further handling needed.
      if (value1.compare("") == 0 && value2.compare("undefined") == 0) {
        result = 0;
      } else if (value1.compare("undefined") == 0 && value2.compare("") == 0) {
        result = 0;
      } else if (value1.compare("") == 0 && value2.compare("0") == 0) {
        result = 0;
      } else if (value1.compare("0") == 0 && value2.compare("") == 0) {
        result = 0;
      }
    }
  } else {
    // unknown valueType
  }
  if (result != 0) {
    protocol += " compare: " + std::to_string(result) + ", type: " + std::to_string(int(type)) + ", key: " + key + ", value1: '" + value1 + "', value2: '" + value2 + "'.";
  }
  return result;
}

/*
 * Returns true, iff the string 'value' starts with given string 'start'.
 * Flag caseSensitive controls if the test is done case sensitive.
 */
bool ScheduleVertex::startsWith(std::string value, std::string start, bool caseSensitive) {
  if (!caseSensitive) {
    // Convert value to lower case
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    // Convert start to lower case
    std::transform(start.begin(), start.end(), start.begin(), ::tolower);
  }
  return (value.find(start) == 0);
}

std::string ScheduleVertex::printProtocol() {
  std::string result = std::string("");
  if (!this->protocol.empty()) {
    result = std::string("Vertex: ") + std::string(*this) + this->protocol;
  }
  return result;
}
