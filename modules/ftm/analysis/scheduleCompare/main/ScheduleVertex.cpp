#include "ScheduleVertex.h"

#include <iostream>

int ScheduleVertex::compare(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  // std::cout << "--V " << v1.name << ", " << v2.name << std::endl;
  //      return v1.type.compare(v2.type);
  if (v1.name == v2.name) {
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
      } else if ("tmsg" == v1.type) {
        return compareTmsg(v1, v2);
      } else if ("wait" == v1.type) {
        return compareWait(v1, v2);
      } else {
        return -1;
      }
    } else {
      return v1.type.compare(v2.type);
    }
  } else {
    return v1.name.compare(v2.name);
  }
}

int ScheduleVertex::compareBlock(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  if (v1.tperiod == v2.tperiod) {
    if (v1.qlo == v2.qlo) {
      if (v1.qhi == v2.qhi) {
        return v1.qil.compare(v2.qil);
      } else {
        return v1.qhi.compare(v2.qhi);
      }
    } else {
      return v1.qlo.compare(v2.qlo);
    }
  } else {
    return v1.tperiod.compare(v2.tperiod);
  }
}

int ScheduleVertex::compareFlow(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  if (v1.target == v2.target) {
    if (v1.tvalid == v2.tvalid) {
      if (v1.tabs == v2.tabs) {
        if (v1.prio == v2.prio) {
          if (v1.reps == v2.reps) {
            return v1.dst.compare(v2.dst);
          } else {
            return v1.reps.compare(v2.reps);
          }
        } else {
          return v1.prio.compare(v2.prio);
        }
      } else {
        return v1.tabs.compare(v2.tabs);
      }
    } else {
      return v1.tvalid.compare(v2.tvalid);
    }
  } else {
    return v1.target.compare(v2.target);
  }
}

int ScheduleVertex::compareFlush(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  if (v1.target == v2.target) {
    if (v1.tvalid == v2.tvalid) {
      if (v1.tabs == v2.tabs) {
        if (v1.clear == v2.clear) {
          return v1.ovr.compare(v2.ovr);
        } else {
          return v1.clear.compare(v2.clear);
        }
      } else {
        return v1.tabs.compare(v2.tabs);
      }
    } else {
      return v1.tvalid.compare(v2.tvalid);
    }
  } else {
    return v1.target.compare(v2.target);
  }
}
int ScheduleVertex::compareListdst(const ScheduleVertex& v1, const ScheduleVertex& v2) { return -1; }
int ScheduleVertex::compareNoop(const ScheduleVertex& v1, const ScheduleVertex& v2) { return -1; }
int ScheduleVertex::compareQbuf(const ScheduleVertex& v1, const ScheduleVertex& v2) { return 0; }
int ScheduleVertex::compareQinfo(const ScheduleVertex& v1, const ScheduleVertex& v2) { return 0; }
int ScheduleVertex::compareSwitch(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  if (v1.target == v2.target) {
    if (v1.tvalid == v2.tvalid) {
      if (v1.tabs == v2.tabs) {
        return v1.dst.compare(v2.dst);
      } else {
        return v1.tabs.compare(v2.tabs);
      }
    } else {
      return v1.tvalid.compare(v2.tvalid);
    }
  } else {
    return v1.target.compare(v2.target);
  }
}

int ScheduleVertex::compareTmsg(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  int result = -1;
  result = compareValues(v1.pattern, v2.pattern, "pattern", valueType::STRING);
  if (result != 0) {
    // check failed, no further checks needed.
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
  result = compareValues(v1.tef, v2.tef, "tef", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.par, v2.par, "par", valueType::STRING);
  if (result != 0) {
    return result;
  }
  result = compareValues(v1.id, v2.id, "id", valueType::STRING);
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
  result = compareValues(v1.evtno, v2.evtno, "evtno", valueType::STRING);
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

int ScheduleVertex::compareValues(const std::string& value1, const std::string& value2, const std::string& key, valueType type) {
  int result = -1;
  if (type == valueType::BOOLEAN) {
    result = compareBoolean(value1, value2);
  } else {
    result = value1.compare(value2);
  }
  if (result != 0) {
    protocol += "Result: " + std::to_string(result) + ", key: " + key + ", value1: '" + value1 + "', value2: '" + value2 + "'.\n";
  }
  return result;
}
