#include <iostream>

#include "ScheduleVertex.h"

int ScheduleVertex::compare(const ScheduleVertex& v1, const ScheduleVertex& v2) {
  // std::cout << "--V " << v1.name << ", " << v2.name << std::endl;
  //      return v1.type.compare(v2.type);
  if (v1.name == v2.name) {
    if (v1.type == v2.type) {
      if ("block" == v1.type) {
      } else if ("blockalign" == v1.type) {
      } else if ("flow" == v1.type) {
      } else if ("flush" == v1.type) {
      } else if ("listdst" == v1.type) {
      } else if ("noop" == v1.type) {
      } else if ("qbuf" == v1.type) {
      } else if ("qinfo" == v1.type) {
      } else if ("switch" == v1.type) {
      } else if ("tmsg" == v1.type) {
      } else if ("wait" == v1.type) {
      }
    }
    return v1.type.compare(v2.type);
  } else {
    return v1.name.compare(v2.name);
  }
}

int ScheduleVertex::compare(const ScheduleVertex& v2) {
  std::cout << "--> " << this->name << ", " << v2.name << std::endl;
  //      return this->type.compare(v2.type);
  if (this->name == v2.name) {
    return this->type.compare(v2.type);
  } else {
    return this->name.compare(v2.name);
  }
}
