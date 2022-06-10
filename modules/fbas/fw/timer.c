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

volatile uint32_t *wb_timer_preset;     // preset register of timer
volatile uint32_t *wb_timer_config;     // config register of time
volatile uint32_t *wb_timer_counter;    // counter of timer
volatile uint32_t *wb_timer_ticklen;    // period of a counter tick

/**
 * \brief set up the timer
 *
 * \param preset timer interval in nanoseconds
 *
 * \ret status
 **/
status_t setupTimer(uint32_t preset)
{
  status_t status = COMMON_STATUS_OK;

  if ((uint32_t)pCpuWbTimer != ERROR_NOT_FOUND) {

    // get addresses of timer registers
    wb_timer_config  = pCpuWbTimer + (WB_TIMER_CONFIG >> 2);
    wb_timer_preset  = pCpuWbTimer + (WB_TIMER_PRESET >> 2);
    wb_timer_counter = pCpuWbTimer + (WB_TIMER_COUNTER >> 2);
    wb_timer_ticklen = pCpuWbTimer + (WB_TIMER_TICKLEN >> 2);

    *wb_timer_preset = preset / *wb_timer_ticklen;
    DBPRINT3("WB timer preset %d\n", preset);
  }
  else {
    DBPRINT2("lm32 timer not found!\n");
    status = COMMON_STATUS_ERROR;
  }

  return status;
}

/**
 * \brief start the timer
 *
 * \param none
 *
 * \ret status
 **/
status_t startTimer()
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
 * \brief stop the timer
 *
 * \param none
 *
 * \ret status
 **/
status_t stopTimer()
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
 * \param none
 *
 * \ret delay time delay in nanoseconds
 **/
uint64_t getTimerIrqDelay()
{
  static uint32_t len    = 0x0;
  static uint32_t preset = 0x0;

  uint64_t ts;
  uint64_t irqDelay;

  if (!len)    len    = *wb_timer_ticklen;         // read tick length [ns] of counter upon first run
  if (!preset) preset = *wb_timer_preset;          // read timer preset [ticks]

  irqDelay = (preset - *wb_timer_counter) * len;   // read actual counter value, calculate delay for IRQ and convert to nanoseconds

  return irqDelay;
}
