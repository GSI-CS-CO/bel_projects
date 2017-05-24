/********************************************************************************************
 *  wr_mil.c
 *
 *  created : 2017
 *  author  : Dietrich Beck, Mathias Kreider GSI-Darmstadt
 *  version : 23-Mar-2017
 *
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2017  Dietrich Beck
 * GSI Helmholtzzentrum für Schwerionenforschung GmbH
 * Planckstraße 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: d.beck@gsi.de
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
 * For all questions and ideas contact: d.beck@gsi.de
 * Last update: 25-April-2015
 ********************************************************************************************/

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

/* includes specific for bel_projects */
#include "mprintf.h"
#include "mini_sdb.h"
#include "irq.h"
#include "aux.h"
#include "dbg.h"

/* local includes for wr_mil firmware*/
#include "wr_mil_value64bit.h"
#include "wr_mil_piggy.h"
#include "wr_mil_eca_queue.h"
#include "wr_mil_eca_ctrl.h"
#include "wr_mil_cmd.h"
#include "wr_mil_utils.h"

// for the event handler
//#include "../../ip_cores/saftlib/drivers/eca_flags.h"

int init()
{
  int cpu_id;
  discoverPeriphery();    // mini-sdb: get info on important Wishbone infrastructure, such as (this) CPU, flash, ...
  uart_init_hw();         // init UART, required for printf... . To view print message, you may use 'eb-console' from the host
  cpu_id = getCpuIdx();            // get ID of THIS CPU
  isr_table_clr();        // set MSI IRQ handler
  irq_set_mask(0x01);     // ...
  irq_disable();          // ...
  return cpu_id;
}

// produce an output pulse on both lemo outputs of the SCU
void lemoPulse12(volatile MilPiggyRegs *mil_piggy)
{
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);
}

#define N_UTC_EVENTS           5          // the number of EVT_UTC events
#define ECA_QUEUE_LM32_TAG     0x00000004 // the tag for ECA actions we (the LM32) want to receive
#define MIL_EVT_START_CYCLE    0x20       // the special event that causes five additional MIL events:
                                          // EVT_UTC_1, EVT_UTC_2, EVT_UTC_3, EVT_UTC_4, EVT_UTC_5
#define WR_MIL_GATEWAY_LATENCY 73575      // latency in units of nanoseconds
                                          // this value was determined by measuring the time difference
                                          // of the MIL event rising edge and the ECA output rising edge (no offset)
                                          // and make this time difference 100.0(5)us
void eventHandler(volatile ECACtrlRegs  *eca,
                  volatile ECAQueueRegs *eca_queue, 
                  volatile MilPiggyRegs *mil_piggy)
{
  if (ECAQueue_actionPresent(eca_queue))
  {
    uint32_t evtCode, milTelegram;
    // select all events from the eca queue that are for the LM32 
    // AND that have an evtNo that is supposed to be translated into a MIL event (indicated
    //     by the return value of ECAQueue_getMilEventData being != 0)
    if ((eca_queue->tag_get == ECA_QUEUE_LM32_TAG) &&
         ECAQueue_getMilEventData(eca_queue, &evtCode, &milTelegram))
    {
      TAI_t    tai_deadl; 
      uint32_t EVT_UTC[N_UTC_EVENTS];
      uint32_t dt;
      ECAQueue_getDeadl(eca_queue, &tai_deadl);
      uint64_t mil_event_time = tai_deadl.value + WR_MIL_GATEWAY_LATENCY; // add 20us to the deadline
          make_mil_timestamp(mil_event_time, EVT_UTC);     

      switch (evtCode)
      {
        case MIL_EVT_START_CYCLE: 
        // generate MIL event EVT_START_CYCLE, followed by EVT_UTC_1/2/3/4/5 EVENTS
          //          make_mil_timestamp(mil_event_time, EVT_UTC);     
          dt = wait_until_tai(eca, mil_event_time);
          MilPiggy_writeCmd(mil_piggy, milTelegram); 
          if (dt){ // use lemo output of SCU to indicate that a deadline could not be respected
            lemoPulse12(mil_piggy);
          }
          // create the five events EVT_UTC_1/2/3/4/5 with seconds and miliseconds since 01/01/2008
          for (int i = 0; i < N_UTC_EVENTS; ++i)
          {
            // Churn out the EVT_UTC MIL events as fast as possible. 
            //  This results in approx. 21 us between two successive events.
            MilPiggy_writeCmd(mil_piggy, EVT_UTC[i]); 
          }
        break;
        default:
          // generate MIL event
          dt = wait_until_tai(eca, mil_event_time);
          MilPiggy_writeCmd(mil_piggy, milTelegram);
          break;
      }
      uint32_t actTag = ECAQueue_getActTag(eca_queue);
    }
    // remove action from ECA queue 
    ECAQueue_actionPop(eca_queue);
  }
}


// this fucnction creates a series of pulses that are triggered by the return from 
// the function wait_until_tai. The lemo outputs can be observed on the Oscilloscope
// in order to measure the timing precision of the wait_until_tai function.
void testOfFunction_wait_until_tai(volatile MilPiggyRegs *mil_piggy,
                                   volatile ECACtrlRegs  *eca_ctrl)
{
    TAI_t tai_now; 
    ECACtrl_getTAI(eca_ctrl, &tai_now);

    uint32_t lateness1 = wait_until_tai(eca_ctrl, tai_now.value + 20000); // start with 20 us margin
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);

    uint32_t lateness2 = wait_until_tai(eca_ctrl, tai_now.value + 1020000ll); // 1 ms after the first pulse
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);

    uint32_t lateness3 = wait_until_tai(eca_ctrl, tai_now.value + 4020000ll); // 4 ms after the first pulse
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);

    uint32_t lateness4 = wait_until_tai(eca_ctrl, tai_now.value + 10020000ll); // 10 ms after the first pulse
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);

    mprintf("%d %d %d %d\n",lateness1, lateness2, lateness3, lateness4); // see if any of the pulses was too late

    for (int i = 0; i < 50; ++i) DELAY1000us; // wait 50 ms before the next series of pulses is generated
}

// after some initialization, the program enters a tight loop where the event handler is called.
// in the event handler, the ECA queue is polled for events and if the event number is in the range [0...255]
// a MIL event is generated. In the special case of event number = MIL_EVT_START_CYCLE = 0x20, in addition to the
// MIL_EVT_START_CYCLE five MIL events are generated that contain a converted WR timestamp with milisecond precision.
void main(void) 
{
  init();   

  // MilPiggy 
  volatile MilPiggyRegs *mil_piggy = MilPiggy_init();
  MilPiggy_lemoOut1Enable(mil_piggy);
  MilPiggy_lemoOut2Enable(mil_piggy);

  // ECAQueue 
  volatile ECAQueueRegs *eca_queue = ECAQueue_init();
  uint32_t n_events = ECAQueue_clear(eca_queue);
  mprintf("popped %d events from the eca queue\n", n_events);

  // ECACtrl 
  volatile ECACtrlRegs *eca_ctrl = ECACtrl_init();
  mprintf("eca ctrl regs at %08x\n", eca_ctrl);

  // Command
  volatile MilCmdRegs *mil_cmd = MilCmd_init();
  mprintf("mil cmd regs at %08x\n", mil_cmd);

  // say hello on the console
  TAI_t nowTAI; 
  ECACtrl_getTAI(eca_ctrl, &nowTAI);
  mprintf("TAI now: 0x%08x%08x\n", nowTAI.part.hi, nowTAI.part.lo);

  while (1) {
    // do whatever has to be done
    eventHandler(eca_ctrl, eca_queue, mil_piggy);
    DELAY10us;

    //testOfFunction_wait_until_tai(mil_piggy, eca_ctrl);

    //poll user commands
    MilCmd_poll(mil_cmd);
  } 
} 
