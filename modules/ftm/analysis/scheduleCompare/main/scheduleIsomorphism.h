#ifndef SCHEDULE_ISOMORPHISM_H
#define SCHEDULE_ISOMORPHISM_H

#include "ScheduleEdge.h"
#include "ScheduleVertex.h"
#include "ScheduleGraph.h"
#include "scheduleCompare.h"

int scheduleIsomorphic(std::string dotFile1, std::string dotfile2, configuration& config);
int testSingleGraph(std::string dotFile1, configuration& config);
void listVertexProtocols(ScheduleGraph& graph, const std::string prefix);
void listEdgeProtocols(ScheduleGraph& graph, const std::string prefix);
void switchCompareNames(ScheduleGraph& graph, const bool flag);
void switchUndefinedAsEmpty(ScheduleGraph& graph, const bool flag);
#endif
