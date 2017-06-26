/********************************************************************************************
 *  wr_mil.c 
 *   based on ../lm32-example/example.c
 *
 *  created : 2017
 *  author  : Michael Reese, Dietrich Beck, Mathias Kreider GSI-Darmstadt
 *  version : 21-Jun-2017
 *
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2017  Michael Reese
 * GSI Helmholtzzentrum für Schwerionenforschung GmbH
 * Planckstraße 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: m.reese@gsi.de
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
 * For all questions and ideas contact: m.reese@gsi.de
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
#include "wr_mil_eca_queue.h"
#include "wr_mil_eca_ctrl.h"
#include "wr_mil_cmd.h"
#include "wr_mil_delay.h"
#include "../../top/gsi_scu/scu_mil.h"

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

// simply write 16bit word on MIL device bus that will mimic a Mil timing event
void mil_piggy_write_event(volatile uint32_t *piggy, uint32_t cmd)
{
     while(!(*(piggy + (MIL_REG_WR_RD_STATUS/4)) & MIL_CTRL_STAT_TRM_READY)) // wait until ready
     {
          DELAY05us; // delay a bit to have less pressure on the wishbone bus
     }

  *(piggy + (MIL_REG_WR_CMD/4)) = cmd; 
}

// produce an output pulse on both lemo outputs of the SCU
void lemoPulse12(volatile uint32_t *mil_piggy)
{
    setLemoOutputEvtMil(mil_piggy, 1, 1);
    setLemoOutputEvtMil(mil_piggy, 2, 1);
    setLemoOutputEvtMil(mil_piggy, 1, 0);
    setLemoOutputEvtMil(mil_piggy, 2, 0);
}

// convert 64-bit TAI from WR into an array of five MIL events (EVT_UTC_1/2/3/4/5 events with evtNr 0xE0 - 0xE4)
// arguments:
//   TAI:     a 64-bit WR-TAI value
//   EVT_UTC: points to an array of 5 uint32_t and will be filled 
//            with valid special MIL events:
//                            EVT_UTC_1 = EVT_UTC[0] =  ms[ 9: 2]          , code = 0xE0
//                            EVT_UTC_2 = EVT_UTC[1] =  ms[ 1: 0] s[30:25] , code = 0xE1
//                            EVT_UTC_3 = EVT_UTC[2] =   s[24:16]          , code = 0xE2
//                            EVT_UTC_4 = EVT_UTC[3] =   s[15: 8]          , code = 0xE3
//                            EVT_UTC_5 = EVT_UTC[4] =   s[ 7: 0]          , code = 0xE4
//            where s is a 30 bit number (seconds since 2008) and ms is a 10 bit number
//            containing the  milisecond fraction.
void make_mil_timestamp(uint64_t TAI, uint32_t *EVT_UTC)
{
  uint64_t msNow  = TAI / UINT64_C(1000000); // conversion from ns to ms (since 1970)
  uint64_t ms2008 = UINT64_C(1199142000000); // miliseconds at 01/01/2008  (since 1970)
                                             // the number was caluclated using: date --date='01/01/2008' +%s
  uint64_t mil_timestamp_ms = msNow - ms2008;
  uint32_t mil_ms           = mil_timestamp_ms % 1000;
  uint32_t mil_sec          = mil_timestamp_ms / UINT64_C(1000);

  EVT_UTC[0]  = (mil_ms>>2)   & 0x000000ff;  // mil_ms[9:2]    to EVT_UTC_1[7:0]
  EVT_UTC[1]  = (mil_ms<<6)   & 0x000000C0;  // mil_ms[1:0]    to EVT_UTC_2[7:5]
  EVT_UTC[1] |= (mil_sec>>24) & 0x0000003f;  // mil_sec[29:24] to EVT_UTC_2[5:0]
  EVT_UTC[2]  = (mil_sec>>16) & 0x000000ff;  // mil_sec[23:16] to EVT_UTC_3[7:0]
  EVT_UTC[3]  = (mil_sec>>8)  & 0x000000ff;  // mil_sec[15:8]  to EVT_UTC_4[7:0]
  EVT_UTC[4]  =  mil_sec      & 0x000000ff;  // mil_sec[7:0]   to EVT_UTC_5[7:0]

  // shift time information to the upper bits [15:8] and add code number
  EVT_UTC[0] = (EVT_UTC[0] << 8) | 0xE0;
  EVT_UTC[1] = (EVT_UTC[1] << 8) | 0xE1;
  EVT_UTC[2] = (EVT_UTC[2] << 8) | 0xE2;
  EVT_UTC[3] = (EVT_UTC[3] << 8) | 0xE3;
  EVT_UTC[4] = (EVT_UTC[4] << 8) | 0xE4;
}


#define N_UTC_EVENTS           5          // the number of EVT_UTC events
#define ECA_QUEUE_LM32_TAG     0x00000004 // the tag for ECA actions we (the LM32) want to receive
#define MIL_EVT_START_CYCLE    0x20       // the special event that causes five additional MIL events:
                                          // EVT_UTC_1, EVT_UTC_2, EVT_UTC_3, EVT_UTC_4, EVT_UTC_5
#define MIL_EVT_END_CYCLE      0x37       // the special event that causes five additional MIL events:
                                          // EVT_UTC_1, EVT_UTC_2, EVT_UTC_3, EVT_UTC_4, EVT_UTC_5
#define MIL_EVT_BEGIN_CMD_EXEC 0xf6       // 246 
#define MIL_EVT_COMMAND        0xff       // 255
#define MIL_EVT_END_CMD_EXEC   0xf5       // 245
#define WR_MIL_GATEWAY_LATENCY 73575      // additional latency in units of nanoseconds
                                          // this value was determined by measuring the time difference
                                          // of the MIL event rising edge and the ECA output rising edge (no offset)
                                          // and make this time difference 100.0(5)us
void eventHandler(volatile uint32_t *eca,
                  volatile uint32_t *eca_queue, 
                  volatile uint32_t *mil_piggy)
{
  if (ECAQueue_actionPresent(eca_queue))
  {
    uint32_t evtCode, milTelegram;
    // select all events from the eca queue that are for the LM32 
    // AND that have an evtNo that is supposed to be translated into a MIL event (indicated
    //     by the return value of ECAQueue_getMilEventData being != 0)
    if ((ECAQueue_getActTag(eca_queue) == ECA_QUEUE_LM32_TAG) &&
         ECAQueue_getMilEventData(eca_queue, &evtCode, &milTelegram))
    {
      TAI_t    tai_deadl; 
      uint32_t EVT_UTC[N_UTC_EVENTS];
      uint32_t too_late;
      ECAQueue_getDeadl(eca_queue, &tai_deadl);
      ECAQueue_actionPop(eca_queue);
      uint64_t mil_event_time = tai_deadl.value + WR_MIL_GATEWAY_LATENCY; // add 20us to the deadline
          make_mil_timestamp(mil_event_time, EVT_UTC);     

      //mprintf("evtCode=%x\n",evtCode);
      switch (evtCode)
      {
        //case MIL_EVT_START_CYCLE: 
        case MIL_EVT_END_CYCLE: 
        // generate MIL event EVT_START_CYCLE, followed by EVT_UTC_1/2/3/4/5 EVENTS
          //make_mil_timestamp(mil_event_time, EVT_UTC);     
          too_late = wait_until_tai(eca, mil_event_time);
          mil_piggy_write_event(mil_piggy, milTelegram); 
          for (int i = 0; i < 100; ++i) DELAY1000us; // 100 ms
          mil_piggy_write_event(mil_piggy, (milTelegram & 0x0000ff00) | MIL_EVT_BEGIN_CMD_EXEC); 
          for (int i = 0; i < 10; ++i) DELAY1000us; // 10 ms
          // create the five events EVT_UTC_1/2/3/4/5 with seconds and miliseconds since 01/01/2008
          for (int i = 0; i < N_UTC_EVENTS; ++i)
          {
            // Churn out the EVT_UTC MIL events as fast as possible. 
            //  This results in approx. 21 us between two successive events.
            mil_piggy_write_event(mil_piggy, EVT_UTC[i]); 
            DELAY100us;
            //mil_piggy_write_event(mil_piggy, 0x0000abc0 | i); 
          }
          for (int i = 0; i < 90; ++i) DELAY1000us; // 90 ms
          mil_piggy_write_event(mil_piggy, (milTelegram & 0x0000ff00) | MIL_EVT_COMMAND); 
          for (int i = 0; i < 100; ++i) DELAY1000us; // 100 ms
          mil_piggy_write_event(mil_piggy, (milTelegram & 0x0000ff00) | MIL_EVT_COMMAND); 
          for (int i = 0; i < 100; ++i) DELAY1000us; // 100 ms
          mil_piggy_write_event(mil_piggy, (milTelegram & 0x0000ff00) | MIL_EVT_END_CMD_EXEC); 
        break;
        default:
          // generate MIL event
          too_late = wait_until_tai(eca, mil_event_time);
          mil_piggy_write_event(mil_piggy, milTelegram);
          //mprintf("mil: %x\n",milTelegram);
          break;
      }
      if (too_late){ // use lemo output of SCU to indicate that a deadline could not be respected
        lemoPulse12(mil_piggy);
        mprintf("late: %d\n",too_late);
      }
    }
    // remove action from ECA queue 
  }
}


// this fucnction creates a series of pulses that are triggered by the return from 
// the function wait_until_tai. The lemo outputs can be observed on the Oscilloscope
// in order to measure the timing precision of the wait_until_tai function.
void testOfFunction_wait_until_tai(volatile uint32_t *mil_piggy,
                                   volatile uint32_t  *eca_ctrl)
{
    TAI_t tai_now; 
    ECACtrl_getTAI(eca_ctrl, &tai_now);

    uint32_t lateness1 = wait_until_tai(eca_ctrl, tai_now.value + 20000); // start with 20 us margin
    setLemoOutputEvtMil(mil_piggy, 1, 1);
    setLemoOutputEvtMil(mil_piggy, 2, 1);
    setLemoOutputEvtMil(mil_piggy, 1, 0);
    setLemoOutputEvtMil(mil_piggy, 2, 0);

    uint32_t lateness2 = wait_until_tai(eca_ctrl, tai_now.value + 1020000ll); // 1 ms after the first pulse
    setLemoOutputEvtMil(mil_piggy, 1, 1);
    setLemoOutputEvtMil(mil_piggy, 2, 1);
    setLemoOutputEvtMil(mil_piggy, 1, 0);
    setLemoOutputEvtMil(mil_piggy, 2, 0);

    uint32_t lateness3 = wait_until_tai(eca_ctrl, tai_now.value + 4020000ll); // 4 ms after the first pulse
    setLemoOutputEvtMil(mil_piggy, 1, 1);
    setLemoOutputEvtMil(mil_piggy, 2, 1);
    setLemoOutputEvtMil(mil_piggy, 1, 0);
    setLemoOutputEvtMil(mil_piggy, 2, 0);

    uint32_t lateness4 = wait_until_tai(eca_ctrl, tai_now.value + 10020000ll); // 10 ms after the first pulse
    setLemoOutputEvtMil(mil_piggy, 1, 1);
    setLemoOutputEvtMil(mil_piggy, 2, 1);
    setLemoOutputEvtMil(mil_piggy, 1, 0);
    setLemoOutputEvtMil(mil_piggy, 2, 0);

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
  volatile uint32_t *mil_piggy = (volatile uint32_t*) find_device_adr(GSI, SCU_MIL);
  configLemoOutputEvtMil(mil_piggy, 1);
  configLemoOutputEvtMil(mil_piggy, 2);
  mprintf("mil_reg_wr_rf_lemo_conf = 0x%08x\n", *(mil_piggy + (MIL_REG_WR_RF_LEMO_CONF/4)));

  // ECAQueue 
  volatile uint32_t *eca_queue = ECAQueue_init();
  uint32_t n_events = ECAQueue_clear(eca_queue);
  mprintf("popped %d events from the eca queue\n", n_events);

  // ECACtrl 
  volatile uint32_t *eca_ctrl = ECACtrl_init();
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
