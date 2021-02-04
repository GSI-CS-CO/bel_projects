#include "ScheduleVertex.h"

#include <iostream>

int ScheduleVertex::compare(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  // std::cout << "--V " << v1.name << ", " << v2.name << std::endl;
  //      return v1.type.compare(v2.type);
  if (v1.name == v2.name) {
    if (v1.type == v2.type) {
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
  if (v1.toffs == v2.toffs) {
    if (v1.tef == v2.tef) {
      if (v1.par == v2.par) {
        if (v1.id == v2.id) {
          if (v1.fid == v2.fid) {
            if (v1.gid == v2.gid) {
              if (v1.evtno == v2.evtno) {
                if (v1.sid == v2.sid) {
                  if (v1.bpid == v2.bpid) {
                    if (v1.beamin == v2.beamin) {
                      if (v1.bpcstart == v2.bpcstart) {
                        if (v1.reqnobeam == v2.reqnobeam) {
                          if (v1.vacc == v2.vacc) {
                            return v1.res.compare(v2.res);
                          } else {
                            return v1.vacc.compare(v2.vacc);
                          }
                        } else {
                          return v1.reqnobeam.compare(v2.reqnobeam);
                        }
                      } else {
                        return v1.bpcstart.compare(v2.bpcstart);
                      }
                    } else {
                      return v1.beamin.compare(v2.beamin);
                    }
                  } else {
                    return v1.bpid.compare(v2.bpid);
                  }
                } else {
                  return v1.sid.compare(v2.sid);
                }
              } else {
                return v1.evtno.compare(v2.evtno);
              }
            } else {
              return v1.gid.compare(v2.gid);
            }
          } else {
            return v1.fid.compare(v2.fid);
          }
        } else {
          return v1.id.compare(v2.id);
        }
      } else {
        return v1.par.compare(v2.par);
      }
    } else {
      return v1.tef.compare(v2.tef);
    }
  } else {
    return v1.toffs.compare(v2.toffs);
  }
}

int ScheduleVertex::compareWait(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  if (v1.target == v2.target) {
    if (v1.tvalid == v2.tvalid) {
      if (v1.tabs == v2.tabs) {
        if (v1.twait == v2.twait) {
          return v1.wabs.compare(v2.wabs);
        } else {
          return v1.twait.compare(v2.twait);
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
