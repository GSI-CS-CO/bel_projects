#include "ScheduleEdge.h"

#include <iostream>

int ScheduleEdge::compare(const ScheduleEdge& e1, const ScheduleEdge& e2) {
  //~ std::cout << "--E " << e1 << ", " << e1.name << " maps to " << e2 << ", " << e2.name << std::endl;
  if (e1.name == e2.name) {
    return e1.type.compare(e2.type);
  } else {
    return e1.name.compare(e2.name);
  }
}

std::ostream& operator<<(std::ostream& os, const ScheduleEdge& edge)
{
  os << edge.vertex_source << " -> " << edge.vertex_target << " [" << edge.type << "]";
  return os;
}
