/********************************************************************************************
 *  example.c
 *
 *  created : 2017
 *  author  : Dietrich Beck, Mathias Kreider GSI-Darmstadt
 *  version : 23-Mar-2017
 *
 *  example program for lm32 softcore on GSI timing receivers
 * 
 *  a few things are demonstrated
 *  - exchange of data via shared RAM. Shared RAM is accessible from the lm32 (this program) 
 *    and via Wishbone from outside the LM32.
 *  - communication with other Wishbone devices
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

// for the event handler
#include "../../ip_cores/saftlib/drivers/eca_flags.h"
#define  MY_ECA_TAG      0x00000004 //just define a tag for ECA actions we want to receive

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

void delay_96plus32n_ns(uint32_t n)
{
  while(n--) asm("nop"); // if asm("nop") is missing, compiler will optimize the loop away
}
#define DELAY1us    delay_96plus32n_ns(28)
#define DELAY2us    delay_96plus32n_ns(60)
#define DELAY5us    delay_96plus32n_ns(153)
#define DELAY10us   delay_96plus32n_ns(310)
#define DELAY20us   delay_96plus32n_ns(622)
#define DELAY40us   delay_96plus32n_ns(1247)
#define DELAY50us   delay_96plus32n_ns(1560)
#define DELAY100us  delay_96plus32n_ns(3122)
#define DELAY1000us delay_96plus32n_ns(31220)

void make_mil_timestamp(uint64_t tai, uint32_t *UTC) // UTC1 = UTC[0] =  ms[ 9: 2]          , code = 0xE0
                                                     // UTC2 = UTC[1] =  ms[ 1: 0] s[30:25] , code = 0xE1
                                                     // UTC3 = UTC[2] =   s[24:16]          , code = 0xE2
                                                     // UTC4 = UTC[3] =   s[15: 8]          , code = 0xE3
                                                     // UTC5 = UTC[4] =   s[ 7: 0]          , code = 0xE4
{
  uint64_t msNow  = tai/1000000;   // conversion from ns to ms
  uint64_t ms2008 = UINT64_C(1199145600000); // miliseconds between 01/01/1970 and 01/01/2008
                                             // the number was caluclated using: expr `date --date='01/01/2008' +%s` - `date --date='01/01/1970' +%s`
  uint64_t mil_timestamp = msNow - ms2008;
  uint32_t mil_ms        = mil_timestamp % 1000;
  uint32_t mil_sec       = mil_timestamp / 1000;
  UTC[0]  =  mil_ms       & 0x000000ff;  //  ms[9:2]   to UTC1[7:0]
  UTC[1]  = (mil_ms>>2)   & 0x000000C0;  //  ms[1:0]   to UTC2[7:5]
  UTC[1] |= (mil_sec>>24) & 0x0000002f;  // sec[29:24] to UTC2[5:0]
  UTC[2]  = (mil_sec>>16) & 0x000000ff;  // sec[23:16] to UTC3[7:0]
  UTC[3]  = (mil_sec>>8)  & 0x000000ff;  // sec[15:8]  to UTC4[7:0]
  UTC[4]  =  mil_sec      & 0x000000ff; 

  // add code number
  UTC[0] = (UTC[0] << 8) | 0x22; //0x20 ;// 0xE0;
  UTC[1] = (UTC[1] << 8) | 0x22; //0x21 ;// 0xE1;
  UTC[2] = (UTC[2] << 8) | 0x22; //0x22 ;// 0xE2;
  UTC[3] = (UTC[3] << 8) | 0x22; //0x23 ;// 0xE3;
  UTC[4] = (UTC[4] << 8) | 0x22; //0x24 ;// 0xE4;
}

uint32_t wait_until_tai_poll(volatile ECACtrlRegs *eca, uint64_t tai_stop)
{
  // poll current time until it is later than the stop time
  TAI_t tai_now; 
  do {
    ECACtrl_getTAI(eca, &tai_now);
  } while (tai_now.value < tai_stop);
  uint32_t lateness_ns = tai_now.value - tai_stop; // we are too late by so many ns
  return lateness_ns;
}

uint32_t wait_until_tai(volatile ECACtrlRegs *eca, uint64_t tai_stop)
{
  // Get current time, ...
  TAI_t tai_now; 
  ECACtrl_getTAI(eca, &tai_now);
  if (tai_stop < tai_now.value) return 1; // stop time is in the past
  // ... calculate waiting time, ...
  uint32_t ns_to_go = tai_stop - tai_now.value; 
  uint32_t delay = ns_to_go;
  delay /= 32; 
  // ... and wait.
  delay_96plus32n_ns(delay);
  return 0;
}

void lemoPulse12(volatile MilPiggyRegs *mil_piggy)
{
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);
}


#define WR_MIL_BRIDGE_LATENCY 65000 // latency in units of nanoseconds

void eventHandler(volatile ECACtrlRegs  *eca,
                  volatile ECAQueueRegs *eca_queue, 
                  volatile MilPiggyRegs *mil_piggy)
{
  if (ECAQueue_actionPresent(eca_queue))
  {
    uint32_t evtNo, evtCode, virtAcc;
    // select all events from the eca queue that are for the LM32
    if (eca_queue->tag_get == MY_ECA_TAG &&
        ECAQueue_getMilEventData(eca_queue, &evtNo, &evtCode, &virtAcc))
    {
      uint32_t milTelegram = 0;  
      TAI_t tai_now, tai_deadl; 
      uint32_t UTC[5];
      uint32_t dt;
      //ECACtrl_getTAI(eca, &tai_now);
      ECAQueue_getDeadl(eca_queue, &tai_deadl);
      uint64_t mil_event_time = tai_deadl.value + WR_MIL_BRIDGE_LATENCY; // add 20us to the deadline
      //mprintf("evtCode=%d\n",evtCode);
      switch (evtCode)
      {
        case 32: // EVT_START_CYCLE
         // generate MIL event EVT_START_CYCLE, followed by 5 UTC EVENTS
          milTelegram  = virtAcc << 8;
          milTelegram |= evtCode; 
          make_mil_timestamp(mil_event_time, UTC);     
          dt = wait_until_tai(eca, mil_event_time);
          MilPiggy_writeCmd(mil_piggy, milTelegram); 
          if (dt){
            lemoPulse12(mil_piggy);
          }
          // make_mil_timestamp function takes about 40 us (because of / and % operations)
          //wait_until_tai(eca, mil_event_time + 50000); // 
          for (int i = 0; i < 5; ++i)
          {
            // Churn out the UTC MIL events as fast as possible. 
            //  This results in 21 us between two successive events.
            MilPiggy_writeCmd(mil_piggy, UTC[i]); 
          }
        break;
        default:
          // generate MIL event
          milTelegram  = virtAcc << 8;
          milTelegram |= evtCode;
          dt = wait_until_tai(eca, mil_event_time);
          MilPiggy_writeCmd(mil_piggy, milTelegram);
          break;
      }
      uint32_t actTag = ECAQueue_getActTag(eca_queue);
    }
    // remove action ECA queue 
    ECAQueue_actionPop(eca_queue);
  }
}

void testOfFunction_wait_until_tai(volatile MilPiggyRegs *mil_piggy,
                                   volatile ECACtrlRegs  *eca_ctrl)
{
    TAI_t tai_now; 
    ECACtrl_getTAI(eca_ctrl, &tai_now);

    uint32_t lateness1 = wait_until_tai(eca_ctrl, tai_now.value + 20000);
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);

    uint32_t lateness2 = wait_until_tai(eca_ctrl, tai_now.value + 1020000ll);
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);

    mprintf("%d %d\n",lateness1, lateness2);

    for (int i = 0; i < 50; ++i) DELAY1000us;
}

void main(void) 
{
  init();   // initialize lm32

  // MilPiggy setup
  volatile MilPiggyRegs *mil_piggy = MilPiggy_init(0);
  MilPiggy_lemoOut1Enable(mil_piggy);
  MilPiggy_lemoOut2Enable(mil_piggy);

  // ECAQueue setup
  volatile ECAQueueRegs *eca_queue = ECAQueue_init(0);
  uint32_t n_events = ECAQueue_clear(eca_queue);
  mprintf("popped %d events from the eca queue\n", n_events);

  // ECACtrl setup
  volatile ECACtrlRegs *eca_ctrl = ECACtrl_init(0);
  mprintf("eca ctrl regs at %08x\n", eca_ctrl);

  // Cmd setup
  volatile MilCmdRegs *mil_cmd = MilCmd_init(0);
  mprintf("mil cmd regs at %08x\n", mil_cmd);

  TAI_t tai_now, tai_stop; 
  ECACtrl_getTAI(eca_ctrl, &tai_now);
  mprintf("TAI now: 0x%08x%08x\n", tai_now.part.hi, tai_now.part.lo);

  while (1) {
    // do whatever has to be done
    eventHandler(eca_ctrl, eca_queue, mil_piggy);
    DELAY10us;

//    testOfFunction_wait_until_tai(mil_piggy, eca_ctrl);
    // poll user commands
    MilCmd_poll(mil_cmd);
  } 
} 
