#ifndef REPLACE_CHAIN_H
#define REPLACE_CHAIN_H

#include "configuration.h"
#include "ScheduleEdge.h"
#include "ScheduleVertex.h"
#include "ScheduleGraph.h"
#include "printSchedule.h"
#include "parseSchedule.h"
#include "scheduleCompact.h"

void usage(char* program);
int main(int argc, char* argv[]);
int compactSingleGraph(std::string dotFile1, configuration& config);

#endif
