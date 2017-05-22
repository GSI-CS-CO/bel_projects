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
  while(n--) asm("nop"); // if "nop" is missing, compiler will optimize the loop away
}
#define DELAY5us    delay_96plus32n_ns(153)
#define DELAY10us   delay_96plus32n_ns(310)
#define DELAY20us   delay_96plus32n_ns(622)
#define DELAY40us   delay_96plus32n_ns(1247)
#define DELAY50us   delay_96plus32n_ns(1560)
#define DELAY100us  delay_96plus32n_ns(3122)
#define DELAY1000us delay_96plus32n_ns(31220)

void make_mil_timestamp(uint64_t tai, uint8_t *UTC1, // 0xE0 ms[ 9: 2]
                                      uint8_t *UTC2, // 0xE1 ms[ 1: 0] s[30:25]
                                      uint8_t *UTC3, // 0xE2  s[24:16] 
                                      uint8_t *UTC4, // 0xE3  s[15: 8]
                                      uint8_t *UTC5) // 0xE4  s[ 7: 0]
{
  uint64_t ms = tai/1000000; // conversion from ns to ms
//  uint64_t 
  //TODO finalize this
}

uint32_t wait_until_tai_poll(volatile ECACtrlRegs *eca, uint64_t tai_stop)
{
  // poll current time until it is later than the stop time
  TAI_t tai_now; 
  do {
    ECACtrl_getTAI(eca, &tai_now);
  } while (tai_now.value < tai_stop);
  uint32_t lateness_ns = tai_now.value - tai_stop; // we are too late by so many us
//  if (lateness_ns < 5096) {
    uint32_t additional_delay = 1500 - lateness_ns;
    additional_delay -= 96;
    additional_delay >>= 5;
    delay_96plus32n_ns(additional_delay);
//  }
  return lateness_ns;
}

uint32_t wait_until_tai(volatile ECACtrlRegs *eca, uint64_t tai_stop)
{
  // get current time, calculate waiting time, and wait.
  TAI_t tai_now; 
  ECACtrl_getTAI(eca, &tai_now);
  if (tai_stop < tai_now.value) return 1; 
  uint32_t ns_to_go = tai_stop - tai_now.value; 
  uint32_t delay = ns_to_go;
  delay -= 96;
  delay >>= 5;
  delay_96plus32n_ns(delay);
  return 0;
}

void eventHandler(volatile ECACtrlRegs  *eca,
                  volatile ECAQueueRegs *eca_queue, 
                  volatile MilPiggyRegs *mil_piggy)
{
  if (ECAQueue_actionPresent(eca_queue))
  {
    uint32_t evtNo, evtCode, virtAcc;
    if (eca_queue->tag_get == MY_ECA_TAG &&
        ECAQueue_getMilEventData(eca_queue, &evtNo, &evtCode, &virtAcc))
    {
      uint32_t milTelegram = 0;  
      TAI_t tai_now, tai_deadl; 
      uint32_t dt;
      //ECACtrl_getTAI(eca, &tai_now);
      ECAQueue_getDeadl(eca_queue, &tai_deadl);
      //mprintf("evtCode=%d\n",evtCode);
      switch (evtCode)
      {
        case 32:
         // generate MIL event EVT_START_CYCLE, followed by 5 UTC EVENTS
          milTelegram  = virtAcc << 8;
          milTelegram |= 32;//evtCode; // EVT_START_CYCLE
          dt = wait_until_tai(eca, tai_deadl.value + 2*1000000);
          MilPiggy_writeCmd(mil_piggy, milTelegram); 
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
          mprintf("dt1=%d\n",dt);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);
          milTelegram &= ~(0x000000ff);
          milTelegram |= 0x22;//0xE0; // EVT_UTC_1
          dt = wait_until_tai(eca, tai_deadl.value + 2*2000000);
          MilPiggy_writeCmd(mil_piggy, milTelegram); 
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
          mprintf("dt2=%d\n",dt);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);
          milTelegram &= ~(0x000000ff);
          milTelegram |= 0x22;//0xE1; // EVT_UTC_2
          dt = wait_until_tai(eca, tai_deadl.value + 2*3000000);
          MilPiggy_writeCmd(mil_piggy, milTelegram); 
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
          mprintf("dt3=%d\n",dt);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);
          milTelegram &= ~(0x000000ff);
          milTelegram |= 0x22;//0xE2; // EVT_UTC_3
          dt = wait_until_tai(eca, tai_deadl.value + 2*4000000);
          MilPiggy_writeCmd(mil_piggy, milTelegram); 
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
          mprintf("dt4=%d\n",dt);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);
          milTelegram &= ~(0x000000ff);
          milTelegram |= 0x22;//0xE3; // EVT_UTC_4
          dt = wait_until_tai(eca, tai_deadl.value + 2*5000000);
          MilPiggy_writeCmd(mil_piggy, milTelegram); 
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
          mprintf("dt5=%d\n",dt);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);
          milTelegram &= ~(0x000000ff);
          milTelegram |= 0x22;//0xE4; // EVT_UTC_5
          dt = wait_until_tai(eca, tai_deadl.value + 2*6000000);
          MilPiggy_writeCmd(mil_piggy, milTelegram); 
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
          mprintf("dt6=%d\n",dt);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);
        break;
        default:
          // generate MIL event
          milTelegram  = virtAcc << 8;
          milTelegram |= evtCode;
          dt = wait_until_tai(eca, tai_deadl.value + 2*7000000);
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
                                   volatile ECACtrlRegs *eca_ctrl)
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
  init();   // initialize 'boot' lm32

  // MilPiggy setup
  volatile MilPiggyRegs *mil_piggy = MilPiggy_init(0);
  MilPiggy_lemoOut1Enable(mil_piggy);
  MilPiggy_lemoOut2Enable(mil_piggy);

  // ECAQueue setup
  volatile ECAQueueRegs *eca_queue = ECAQueue_init(0);
  uint32_t n_events = ECAQueue_clear(eca_queue);
  mprintf("found %d events in eca queue, popped all of them\n", n_events);

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
    //ECAQueue_clear(eca_queue);

    // do whatever has to be done
    //MilPiggy_lemoOut1High(mil_piggy);
    //MilPiggy_lemoOut2High(mil_piggy);
    eventHandler(eca_ctrl, eca_queue, mil_piggy);
    //MilPiggy_lemoOut1Low(mil_piggy);
    //MilPiggy_lemoOut2Low(mil_piggy);
     DELAY10us;

//    testOfFunction_wait_until_tai(mil_piggy, eca_ctrl);

    // poll user commands
    MilCmd_poll(mil_cmd);
  } 
} 
