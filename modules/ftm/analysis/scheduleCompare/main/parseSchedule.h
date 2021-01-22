#ifndef PARSE_SCHEDULE_H
#define PARSE_SCHEDULE_H

#include <string>

#include "scheduleCompare.h"
#include "scheduleIsomorphism.h"

bool parseSchedule(std::string& dot_file, ScheduleGraph& g, boost::dynamic_properties& dp, configuration& config);

#endif
