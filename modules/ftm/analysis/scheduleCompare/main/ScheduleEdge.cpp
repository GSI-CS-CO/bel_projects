#include "ScheduleEdge.h"

#include <iostream>

int ScheduleEdge::compare(const ScheduleEdge& e1, const ScheduleEdge& e2) {
  //~ std::cout << "--E " << e1 << ", " << e1.name << " maps to " << e2 << ", " << e2.name << std::endl;
  int result = -1;
  std::string key = std::string("");
  std::string value1 = std::string("");
  std::string value2 = std::string("");
  if (e1.name == e2.name) {
    result = e1.type.compare(e2.type);
    key = "type";
    value1 = e1.type;
    value2 = e2.type;
  } else {
    result = e1.name.compare(e2.name);
    key = "name";
    value1 = e1.name;
    value2 = e2.name;
  }
  if (result != 0) {
    protocol += "Result: " + std::to_string(result) + ", key: " + key + ", value1: '" + value1 + "', value2: '" + value2 + "'.";
  }
  return result;
}

std::string ScheduleEdge::printProtocol() {
  return std::string("Edge: ") + std::string(*this) + std::string(": ") + this->protocol;
}

ScheduleEdge::operator std::string() {
  return std::string(this->vertex_source) + std::string(" -> ") + std::string(this->vertex_target) + std::string(" [") + this->type + std::string("]");
}

std::ostream& operator<<(std::ostream& os, const ScheduleEdge& edge) {
  os << edge.vertex_source << " -> " << edge.vertex_target << " [" << edge.type << "]";
  return os;
}
