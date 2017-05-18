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
#include "mprintf.h"
#include "mini_sdb.h"


/* includes specific for bel_projects */
#include "irq.h"
#include "aux.h"
#include "dbg.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA controle
#include "../../ip_cores/saftlib/drivers/eca_flags.h"

/* local includes for wr_mil firmware*/
#include "wr_mil_value64bit.h"
#include "wr_mil_piggy.h"
#include "wr_mil_eca_queue.h"
#include "wr_mil_eca_ctrl.h"
#include "wr_mil_cmd.h"

#define  MY_ECA_TAG      0x4 //just define a tag for ECA actions we want to receive

/* register maps for some selected Wishbone devices  */
#include "../../tools/wb_slaves.h" /* this is a hack */
#include "../../top/gsi_scu/scu_mil.h"

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

void newECAHandler(volatile ECAQueueRegs *eca_queue, volatile MilPiggyRegs *mil_piggy)
{
  // if (ECAQueue_getFlags(eca_queue) & (1<<ECA_VALID))
  // {
  //   EvtId_t evtId = { 
  //     .part.hi = eca_queue->event_id_hi_get,
  //     .part.lo = eca_queue->event_id_lo_get
  //   };
  //   TAI_t evtDeadl = { 
  //     .part.hi = eca_queue->deadline_hi_get,
  //     .part.lo = eca_queue->deadline_lo_get
  //   };
    // switch(eca_queue->tag_get)
    // {
    //   case MY_ECA_TAG:
        mil_piggy->wr_cmd = 32;
        for (int i = 0; i < 50; ++i)
        {
          DELAY50us;
          mil_piggy->wr_cmd = 0x22;
        }
        DELAY50us;
        mil_piggy->wr_cmd = 55;
        DELAY1000us;      
  //      break;
  //      default:
  //        mprintf("ecaHandler: unknown tag\n");
  //      break;
  //    }
  //   uint32_t actTag = ECAQueue_getActTag(eca_queue);
  // }
}


void main(void) 
{
  uint32_t i,j;
  
  init();   // initialize 'boot' lm32

  // MilPiggy setup
  volatile MilPiggyRegs *mil_piggy = MilPiggy_init(find_device_adr(GSI, SCU_MIL));
  mprintf("mil_piggy.pMILPiggy = 0x%08x\n", mil_piggy);
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
    MilPiggy_lemoOut1High(mil_piggy);
    MilPiggy_lemoOut2High(mil_piggy);
    newECAHandler(eca_queue, mil_piggy);
    MilPiggy_lemoOut1Low(mil_piggy);
    MilPiggy_lemoOut2Low(mil_piggy);
    DELAY100us;

    // poll user commands
    MilCmd_poll(mil_cmd);
  } 
} 
