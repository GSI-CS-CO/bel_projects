#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>

#include "dbg.h"              // DBPRINT*
#include "common-defs.h"      // COMMON_STATUS_*
#include "fbas_common.h"
#include "mini_sdb.h"         // pCpuWbTimer
#include "wb_timer_regs.h"

status_t setupTimer(uint32_t preset);
status_t startTimer();
status_t stopTimer();

#endif
