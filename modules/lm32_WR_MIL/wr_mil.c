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


void eventHandler(volatile ECAQueueRegs *eca_queue, 
                  volatile MilPiggyRegs *mil_piggy)
{
  if (ECAQueue_actionPresent(eca_queue))
  {
    EvtId_t evtId = { 
      .part.hi = eca_queue->event_id_hi_get,
      .part.lo = eca_queue->event_id_lo_get
    };
    TAI_t evtDeadl = { 
      .part.hi = eca_queue->deadline_hi_get,
      .part.lo = eca_queue->deadline_lo_get
    };

    uint32_t evtNo, evtCode, virtAcc;
    if (eca_queue->tag_get == MY_ECA_TAG &&
        ECAQueue_getMilEventData(eca_queue, &evtNo, &evtCode, &virtAcc))
    {
      uint32_t milTelegram = 0;  
      //mprintf("evtCode=%d\n",evtCode);
      switch (evtCode)
      {
        case 8://32:
          // generate MIL event EVT_START_CYCLE, followed by 5 UTC EVENTS
          milTelegram  = virtAcc << 8;
          milTelegram |= 32;//evtCode; // EVT_START_CYCLE
          MilPiggy_writeCmd(mil_piggy, milTelegram); DELAY50us;
          milTelegram &= ~(0x000000ff);
          milTelegram |= 0x22;//0xE0; // EVT_UTC_1
          MilPiggy_writeCmd(mil_piggy, milTelegram); DELAY50us;
          milTelegram &= ~(0x000000ff);
          milTelegram |= 0x22;//0xE1; // EVT_UTC_2
          MilPiggy_writeCmd(mil_piggy, milTelegram); DELAY50us;
          milTelegram &= ~(0x000000ff);
          milTelegram |= 0x22;//0xE2; // EVT_UTC_3
          MilPiggy_writeCmd(mil_piggy, milTelegram); DELAY50us;
          milTelegram &= ~(0x000000ff);
          milTelegram |= 0x22;//0xE3; // EVT_UTC_4
          MilPiggy_writeCmd(mil_piggy, milTelegram); DELAY50us;
          milTelegram &= ~(0x000000ff);
          milTelegram |= 0x22;//0xE4; // EVT_UTC_5
          MilPiggy_writeCmd(mil_piggy, milTelegram); DELAY50us;
          milTelegram &= ~(0x000000ff);
          milTelegram |= 55;//0xE4; // EVT_UTC_5
          MilPiggy_writeCmd(mil_piggy, milTelegram); DELAY50us;
        break;
        default:
          // generate MIL event
          milTelegram  = virtAcc << 8;
          milTelegram |= evtCode;
          MilPiggy_writeCmd(mil_piggy, milTelegram);
          break;
      }
      uint32_t actTag = ECAQueue_getActTag(eca_queue);
    }
    // remove action ECA queue 
    ECAQueue_actionPop(eca_queue);
  }
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
  mprintf("found %d events in eca queue\n", n_events);

  // ECACtrl setup
  volatile ECACtrlRegs *eca_ctrl = ECACtrl_init(0);

  // Cmd setup
  volatile MilCmdRegs *mil_cmd = MilCmd_init(0);

  while (1) {
    // do whatever has to be done
    //MilPiggy_lemoOut1High(mil_piggy);
    //MilPiggy_lemoOut2High(mil_piggy);
    DELAY50us;
    eventHandler(eca_queue, mil_piggy);
    //MilPiggy_lemoOut1Low(mil_piggy);
    //MilPiggy_lemoOut2Low(mil_piggy);
    DELAY50us;

    // poll user commands
    MilCmd_poll(mil_cmd);
  } 
} 
