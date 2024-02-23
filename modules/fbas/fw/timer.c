/********************************************************************************************
 *  timer.c
 *
 *  created : 2021
 *  author  : Enkhbold Ochirsuren, GSI-Darmstadt
 *  version : 10-June-2021
 *
 *  Functions to control the WB timer
 *
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2021  Enkhbold Ochirsuren
 * GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 * Planckstrasse 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: e.ochirsuren@gsi.de
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For all questions and ideas contact: e.ochirsuren@gsi.de
 * Last update: 10-June-2021
 ********************************************************************************************/

#include "timer.h"

static volatile uint32_t *wb_timer_preset;     // preset register (reload value on counter overflow)
static volatile uint32_t *wb_timer_config;     // config register (bit0=1 count-down timer)
static volatile uint32_t *wb_timer_counter;    // counter register
static volatile uint32_t *wb_timer_ticklen;    // tick length register (ticklen = 1000/62.5 = 16 ns, 1 tick every 16 ns)
static volatile uint32_t *wb_timer_ts_lo;      // timestamp low 32-bit (since timer start)
static volatile uint32_t *wb_timer_ts_hi;      // timestamp high 32-bit

static struct timer_s timers[N_TIMERS];        // software timers

void timerInitTimers(void);
void timerTickTimers(void);
static void timerInit(struct timer_s *t, uint64_t period_ms);
void timerStart(struct timer_s *t);
void timerStop(struct timer_s *t);
void timerUnregister(struct timer_s *t);
struct timer_s * timerRegister(uint32_t period_ms);
uint8_t timerIsExpired(struct timer_s *t);

/**
 * \brief Initialize all timers
*/
void timerInitTimers(void)
{
  struct timer_s *t;

  for (t = timers; t < &timers[N_TIMERS]; t++)
  {
    t->in_use = TIMER_UNREG;
    t->running = TIMER_OFF;
  };
}

/**
 * \brief Tick all timers
*/
void timerTickTimers(void)
{
  struct timer_s *t;

  for (t = timers; t < &timers[N_TIMERS]; t++)
    if (t->running)
      t->elapsed_ms++;
}

/**
 * \brief Initialize a given timer
 *
 * \param t  Target timer
 * \param period_ms  Timer period, [ms]
*/
static void timerInit(struct timer_s *t, uint64_t period_ms)
{
  t->period_ms = period_ms;
  t->elapsed_ms = 0;
  t->running = TIMER_OFF;
  t->in_use  = TIMER_UNREG;
}

/**
 * \brief Start a given timer
 *
 * \param t Target timer
*/
void timerStart(struct timer_s *t)
{
  if (!t->in_use)
    return;

  t->elapsed_ms = 0;
  t->running = TIMER_ON;
}

/**
 * \brief Stop a given timer
 *
 * \param t Target timer
*/
void timerStop(struct timer_s *t)
{
  if (!t->in_use)
    return;

  t->running = TIMER_OFF;
}

/**
 * \brief Release an active timer
 *
 * \param t Target timer
*/
void timerUnregister(struct timer_s *t)
{
  if (!t->in_use)
    return;

  t->in_use  = TIMER_UNREG;
  t->running = TIMER_OFF;
}

/**
 * \brief Declare new timer
 *
 * \param period_ms Timer period, [ms]
*/
struct timer_s * timerRegister(uint32_t period_ms)
{
  struct timer_s *t;

  for (t=timers; t<&timers[N_TIMERS]; t++) {
    if (!t->in_use)
      break;
  }

  // no timer is available
  if (t == &timers[N_TIMERS]) {
    return NULL;
  }

  timerInit(t, period_ms);
  t->in_use = TIMER_REG;
  return t;
}

/**
 * \brief Check if a given timer expired
 *
 * \param t Target timer
 * \return Non-zero if expired, otherwise zero
*/
uint8_t timerIsExpired(struct timer_s *t)
{
  if (t->running && (t->elapsed_ms >= t->period_ms)) {
    t->running = TIMER_OFF;
    return TIMER_EXPIRED;
  }

  return 0;
}

/**
 * \brief Initialize the timer debug
*/
void timerInitDbg(struct timer_dbg_s *d)
{
  d->period.avg = 0;
  d->period.min = 0;
  d->period.max = 0;
  d->last = 0;
  d->cnt  = 0;
}

/**
 * \brief Update the timer debug
 *
 * \param d Target timer debug
 * \param now Actual syste time
*/
void timerUpdateDbg(struct timer_dbg_s *d, uint64_t now)
{
  int64_t elapsed;

  if (d->last) {
    elapsed = now - d->last;
    d->period.avg = (elapsed + (d->cnt * d->period.avg)) / (d->cnt + 1);
    d->cnt++;

    if (elapsed > d->period.max)
      d->period.max = elapsed;

    if (elapsed < d->period.min || !d->period.min)
      d->period.min = elapsed;
  }

  d->last = now;
}

/**
 * \brief Set up the hardware timer
 *
 * \param interval_ns Timer interval, [ns]
 * \return status OK on success, otherwise ERROR
 **/
status_t timerSetupHw(uint32_t interval_ns)
{
  status_t status = COMMON_STATUS_OK;

  if ((uint32_t)pCpuWbTimer != ERROR_NOT_FOUND) {

    // get addresses of timer registers
    wb_timer_config  = pCpuWbTimer + (WB_TIMER_CONFIG >> 2);
    wb_timer_preset  = pCpuWbTimer + (WB_TIMER_PRESET >> 2);
    wb_timer_counter = pCpuWbTimer + (WB_TIMER_COUNTER >> 2);
    wb_timer_ticklen = pCpuWbTimer + (WB_TIMER_TICKLEN >> 2);
    wb_timer_ts_lo   = pCpuWbTimer + (WB_TIMER_TIMESTAMP_LO >> 2);
    wb_timer_ts_hi   = pCpuWbTimer + (WB_TIMER_TIMESTAMP_HI >> 2);

    // set the preset value
    *wb_timer_preset = interval_ns / *wb_timer_ticklen;
    DBPRINT3("WB timer preset %d\n", preset);
  }
  else {
    DBPRINT2("lm32 timer not found!\n");
    status = COMMON_STATUS_ERROR;
  }

  return status;
}

/**
 * \brief Enable the HW timer
 *
 * \return OK on success, otherwise ERROR
 **/
status_t timerEnableHw(void)
{
  status_t status = COMMON_STATUS_OK;

  if ((uint32_t)pCpuWbTimer != ERROR_NOT_FOUND) {
    *wb_timer_config = 0x1;              // start timer
    DBPRINT3("WB timer started.\n");
  }
  else {
    DBPRINT2("lm32 timer not found!\n");
    status = COMMON_STATUS_ERROR;
  }

  return status;
}

/**
 * \brief Disable the HW timer
 *
 * \return OK on success, otherwise ERROR
 **/
status_t timerDisableHw(void)
{
  status_t status = COMMON_STATUS_OK;

  if ((uint32_t)pCpuWbTimer != ERROR_NOT_FOUND) {
    *wb_timer_config = 0x0;              // stop timer
    DBPRINT3("WB timer stopped.\n");
  }
  else {
    DBPRINT2("lm32 timer not found!\n");
    status = COMMON_STATUS_ERROR;
  }

  return status;
}

/**
 * \brief get delay
 *
 * Calculate time delay in handling the timer interrupt
 *
 * \return delay time delay in nanoseconds
 **/
static uint64_t getTimerIrqDelay()
{
  static uint32_t len    = 0x0;
  static uint32_t preset = 0x0;

  uint64_t irqDelay;

  if (!len)    len    = *wb_timer_ticklen;         // read tick length [ns] of counter upon first run
  if (!preset) preset = *wb_timer_preset;          // read timer preset [ticks]

  irqDelay = (preset - *wb_timer_counter) * len;   // read actual counter value, calculate delay for IRQ and convert to nanoseconds

  return irqDelay;
}
