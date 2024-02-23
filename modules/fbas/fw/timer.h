#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>

#include "dbg.h"              // DBPRINT*
#include "common-defs.h"      // COMMON_STATUS_*
#include "fbas_common.h"
#include "mini_sdb.h"         // pCpuWbTimer
#include "wb_timer_regs.h"

#define N_TIMERS      8       // number of software timers
#define TIMER_UNREG   0       // unregistered/unused timer
#define TIMER_REG     1       // registered timer
#define TIMER_ON      1       // timer is started
#define TIMER_OFF     0       // timer is stopped
#define TIMER_EXPIRED 1       // timer is expired

struct timer_s {
    uint32_t  period_ms;      // timer period, [ms]
    uint32_t  elapsed_ms;     // elapsed time, [ms]
    uint8_t   in_use;         // control flag
    uint8_t   running;        // control/status flag
};

struct timer_dbg_s {
    struct {
        int64_t  avg;         // (moving) average
        int64_t  min;         // minimum
        int64_t  max;         // maximum
    } period;
    uint64_t last;            // last access time
    uint32_t cnt;             // count
};

// functions for the HW timer
status_t timerSetupHw(uint32_t interval_ns);
status_t timerEnableHw(void);
status_t timerDisableHw(void);

// functions for the SW timers
void timerTickTimers(void);
void timerInitTimers(void);
void timerStart(struct timer_s *t);
void timerStop(struct timer_s *t);
void timerUnregister(struct timer_s *t);
struct timer_s * timerRegister(uint32_t period_ms);
uint8_t timerIsExpired(struct timer_s *t);

// functions for the timer debug
void timerInitDbg(struct timer_dbg_s *d);
void timerUpdateDbg(struct timer_dbg_s *d, uint64_t now);

#endif
