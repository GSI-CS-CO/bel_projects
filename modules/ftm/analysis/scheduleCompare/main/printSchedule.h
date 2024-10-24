#ifndef PRINT_SCHEDULE_H
#define PRINT_SCHEDULE_H

#include "scheduleCompare.h"
#include "scheduleIsomorphism.h"

void printSchedule(std::string header, ScheduleGraph& g, boost::dynamic_properties& dp, configuration& config);
void saveSchedule(ScheduleGraph& g, configuration& config);
void saveScheduleIndex(std::string fileName, ScheduleGraph& g, configuration& config);
void printScheduleIndex(std::string title, ScheduleGraph& g, configuration& config);

#endif
