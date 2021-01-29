#include <iostream>

#include "ScheduleEdge.h"

int ScheduleEdge::compare(const ScheduleEdge& e1, const ScheduleEdge& e2) {
  std::cout << "--E " << e1.name << ", " << e2.name << std::endl;
  if (e1.name == e2.name) {
    return e1.type.compare(e2.type);
  } else {
    return e1.name.compare(e2.name);
  }
}
