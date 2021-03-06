/*******************************************************************************
 *  ecaMsiExample.c
 *
 *  created : 2019, GSI Darmstadt
 *  author  : Dietrich Beck, Enkhbold Ochirsuren
 *  version : 15-Apr-2019
 *
 *  This example demonstrates handling of message-signaled interrupts (MSI)
 *  caused by ECA channel.
 *  ECA is capable to send MSIs on certain conditions such as producing actions
 *  on reception of timing messages.
 *
 *  build: make clean && make TARGET=ecaMsiExample
 *  deploy: scp ecaMsiExample.bin root@scuxl0304.acc:.
 *  load: eb-fwload dev/wbm0 u 0x0 ecaMsiExample.bin
 *  run: eb-reset dev/wbm0 cpureset 0 (assume only one LM32 is instantiated)
 *  debug: eb-console dev/wbm0
 *
 *  To run example:
 *  - set ECA rules for eCPU action channel
 *	saft-ecpu-ctl tr0 -d -c 0x1122334455667788 0xFFFFFFFFFFFFFFFF 0 0x42
 *  - debug firmware output
 *	eb-console dev/wbm0
 *  - inject timing message (invoke on the second terminal)
 *	saft-ctl -p tr0 inject 0x1122334455667788 0x8877887766556642 0
 *
 * -----------------------------------------------------------------------------
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
 ******************************************************************************/

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

/* includes specific for bel_projects */
#include "mprintf.h"
#include "mini_sdb.h"
#include "aux.h"
#include "dbg.h"
#include "syscon.h"

/* register maps for some selected Wishbone devices  */
#include "../../tools/wb_slaves.h" /* this is a hack */
#include "../../ip_cores/saftlib/drivers/eca_flags.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"

#define STATUS_OK         0UL
#define STATUS_ERR        1UL

/* ECA relevant definitions */
#define ECAQMAX           4UL   // max number of ECA channels in the system
#define ECACHANNELFORLM32 2UL   // the id of an ECA channel for embedded CPU
#define ECA_VALID_ACTION  0x00040000UL // ECA valid action
#define MY_ACT_TAG        0x42UL       // ECA actions tagged for this eCPU

/* shared memory map for communication via Wishbone  */
#include <ecaMsiExample_shared_mmap.h>        // autogenerated upon building firmware

// global variables
volatile uint32_t *pEcaCtl;         // WB address of ECA control
volatile uint32_t *pEca;            // WB address of ECA event input (discoverPeriphery())
volatile uint32_t *pECAQ;           // WB address of ECA queue
int gEcaChECPU = 0;                 // ECA channel for an embedded CPU (LM32), connected to ECA queue pointed by pECAQ

/*******************************************************************************
 *
 * Clear ECA queue
 *
 * @param[in] cnt The number pending actions
 * \return        The number of cleared actions
 *
 ******************************************************************************/
uint32_t clearEcaQueue(uint32_t cnt)
{
  uint32_t flag;                // flag for the next action
  uint32_t i, j = 0;

  for ( i = 0; i < cnt; ++i) {

    flag = *(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2)); // read flag and check if there was an action

    if (flag & (0x0001 << ECA_VALID)) {
      *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;  // pop action from channel
      ++j;
    }
  }

  return j;
}

/*******************************************************************************
 *
 * Clear pending valid actions
 *
 ******************************************************************************/
void clearActions()
{
  uint32_t valCnt;

  *(pEcaCtl + (ECA_CHANNEL_SELECT_RW >> 2)) = gEcaChECPU;    // select ECA channel for LM32
  *(pEcaCtl + (ECA_CHANNEL_NUM_SELECT_RW >> 2)) = 0x00;      // set the sub channel index
  valCnt = *(pEcaCtl + (ECA_CHANNEL_VALID_COUNT_GET >> 2));  // get/clear valid count

  if (valCnt) {
    mprintf("pending actions: %d\n", valCnt);
    valCnt = clearEcaQueue(valCnt);                          // pop pending actions
    mprintf("cleared actions: %d\n", valCnt);
  }
}

/*******************************************************************************
 *
 * Configure ECA to send MSI to embedded soft-core LM32:
 * - ECA action channel for LM32 is selected and MSI target address of LM32 is
 *   set in the ECA MSI target register
 *
 * @param[in] enable  Enable or disable ECA MSI
 * @param[in] channel The index of the selected ECA action channel
 *
 ******************************************************************************/
void configureEcaMsi(int enable, uint32_t channel) {

  if (enable != 0 && enable != 1) {
    mprintf("Bad enable argument.\n");
    return;
  }

  if (channel > ECAQMAX) {
    mprintf("Bad channel argument.\n");
    return;
  }

  clearActions();   // clean ECA queue and channel from pending actions

  atomic_on();
  *(pEcaCtl + (ECA_CHANNEL_SELECT_RW >> 2)) = channel;            // select channel
  *(pEcaCtl + (ECA_CHANNEL_NUM_SELECT_RW >> 2)) = 0x00;           // set the sub channel index
  *(pEcaCtl + (ECA_CHANNEL_MSI_SET_ENABLE_OWR >> 2)) = 0;         // disable ECA MSI (required to set a target address)
  *(pEcaCtl + (ECA_CHANNEL_MSI_SET_TARGET_OWR >> 2)) = (uint32_t)pMyMsi;  // set MSI destination address as a target address
  *(pEcaCtl + (ECA_CHANNEL_MSI_SET_ENABLE_OWR >> 2)) = enable;    // enable ECA MSI
  atomic_off();

  mprintf("MSI path (ECA -> LM32)           : %s\n\tECA channel = %d\n\tdestination = 0x%08x\n",
      enable == 1 ? "enabled" : "disabled", channel, (uint32_t)pMyMsi);
}

/*******************************************************************************
 *
 * Pop pending embedded CPU actions from an ECA queue and handle them
 *
 * @param[in] cnt The number of pending valid actions
 *
 *******************************************************************************/
void ecaHandler(uint32_t cnt)
{
  uint32_t flag;                // flag for the next action
  uint32_t evtIdHigh;           // event id (high 32bit)
  uint32_t evtIdLow;            // event id (low 32bit)
  uint32_t evtDeadlHigh;        // deadline (high 32bit)
  uint32_t evtDeadlLow;         // deadline (low 32bit)
  uint32_t actTag;              // tag of action
  uint32_t paramHigh;           // event parameter (high 32bit)
  uint32_t paramLow;            // event parameter (low 32bit)
  uint32_t i;

  for (i = 0; i < cnt; ++i) {
    // read flag and check if there was an action
    flag         = *(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2));
    if (flag & (0x0001 << ECA_VALID)) {
      // read data
      evtIdHigh    = *(pECAQ + (ECA_QUEUE_EVENT_ID_HI_GET >> 2));
      evtIdLow     = *(pECAQ + (ECA_QUEUE_EVENT_ID_LO_GET >> 2));
      evtDeadlHigh = *(pECAQ + (ECA_QUEUE_DEADLINE_HI_GET >> 2));
      evtDeadlLow  = *(pECAQ + (ECA_QUEUE_DEADLINE_LO_GET >> 2));
      actTag       = *(pECAQ + (ECA_QUEUE_TAG_GET >> 2));
      paramHigh    = *(pECAQ + (ECA_QUEUE_PARAM_HI_GET >> 2));
      paramLow     = *(pECAQ + (ECA_QUEUE_PARAM_LO_GET >> 2));

      // pop action from channel
      *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;

      // here: do s.th. according to action
      switch (actTag) {
	case MY_ACT_TAG:
	  mprintf("ecaHandler: id: 0x%08x%08x; deadline: 0x%08x%08x; param: 0x%08x%08x; flag: 0x%08x\n",
	      evtIdHigh, evtIdLow, evtDeadlHigh, evtDeadlLow, paramHigh, paramLow, flag);
	  break;
	default:
	  mprintf("ecaHandler: unknown tag\n");
      } // switch

    } // if data is valid
  }
} // ecaHandler

/*******************************************************************************
 *
 * Handle pending valid actions
 *
 ******************************************************************************/
void handleValidActions()
{
  uint32_t valCnt;
  *(pEcaCtl + (ECA_CHANNEL_SELECT_RW >> 2)) = gEcaChECPU;    // select ECA channel for LM32
  *(pEcaCtl + (ECA_CHANNEL_NUM_SELECT_RW >> 2)) = 0x00;      // set the sub channel index
  valCnt = *(pEcaCtl + (ECA_CHANNEL_VALID_COUNT_GET >> 2));  // read and clear valid counter
  mprintf("valid=%d\n", valCnt);

  if (valCnt != 0)
    ecaHandler(valCnt);                             // pop pending valid actions
}

/*******************************************************************************
 *
 * Handle MSIs sent by ECA
 *
 * If interrupt was caused by a valid action, then MSI has value of (4<<16|num).
 * Both ECA action channel and ECA queue connected to that channel must be handled:
 * - read and clear the valid counter value of ECA action channel for LM32 and,
 * - pop pending actions from ECA queue connected to this action channel
 *
 ******************************************************************************/
void irqHandler() {

  mprintf("MSI:\t%08x\nAdr:\t%08x\nSel:\t%01x\n", global_msi.msg, global_msi.adr, global_msi.sel);

  if ((global_msi.msg & ECA_VALID_ACTION) == ECA_VALID_ACTION) // valid actions are pending
    handleValidActions();                                      // ECA MSI handling
}

/*******************************************************************************
 *
 * Initialize interrupt table
 * - set up an interrupt handler
 * - enable interrupt generation globally
 *
 ******************************************************************************/
void initIrqTable() {
  isr_table_clr();
  isr_ptr_table[0] = &irqHandler;
  irq_set_mask(0x01);
  irq_enable();
  mprintf("Init IRQ table is done.\n");
}

/*******************************************************************************
 *
 * Find WB address of ECA queue connect to ECA channel for LM32
 *
 * - ECA queue address is set to "pECAQ"
 * - index of ECA channel for LM32 is set to "gEcaChECPU"
 *
 * /return Return OK if a queue is found, otherwise return ERROR
 *
 ******************************************************************************/
uint32_t findEcaQueue()
{
  sdb_location EcaQ_base[ECAQMAX];
  uint32_t EcaQ_idx = 0;
  uint32_t *tmp;
  int i;

  // get list of ECA queues
  find_device_multi(EcaQ_base, &EcaQ_idx, ECAQMAX, ECA_QUEUE_SDB_VENDOR_ID, ECA_QUEUE_SDB_DEVICE_ID);
  pECAQ = 0x0;

  // find ECA queue connected to ECA channel for LM32
  for (i=0; i < EcaQ_idx; i++) {
    tmp = (uint32_t *)(getSdbAdr(&EcaQ_base[i]));
    //mprintf("-- found ECA queue 0x%08x, idx %d\n", (uint32_t)tmp, i);
    if ( *(tmp + (ECA_QUEUE_QUEUE_ID_GET >> 2)) == ECACHANNELFORLM32) {
      pECAQ = tmp;    // update global variables
      gEcaChECPU = ECACHANNELFORLM32 +1; // refer to eca_queue_regs.h
      i = EcaQ_idx;   // break loop
    }
  }

  return (pECAQ ? STATUS_OK : STATUS_ERR);
}

/*******************************************************************************
 *
 * Initialization
 * - discover WB devices
 * - init UART
 * - detect ECA control unit
 * - detect ECA queues
 *
 ******************************************************************************/
void init()
{
  discoverPeriphery();    // mini-sdb: get info on important Wishbone infrastructure, such as (this) CPU, flash, ...

  uart_init_hw();         // init UART, required for printf... . To view print message, you may use 'eb-console' from the host

  mprintf("--- Demo for ECA MSI handling ---\n");

  if (pEca)
    mprintf("ECA event input                  @ 0x%08x\n", (uint32_t) pEca);
  else {
    mprintf("Could not find the ECA event input. Exit!\n");
    return;
  }

  mprintf("MSI destination addr for LM32    : 0x%08x\n", (uint32_t)pMyMsi);

  pEcaCtl = find_device_adr(ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID);

  if (pEcaCtl)
    mprintf("ECA channel control              @ 0x%08x\n", (uint32_t) pEcaCtl);
  else {
    mprintf("Could not find the ECA channel control. Exit!\n");
    return;
  }

  if (findEcaQueue() == STATUS_OK)
    mprintf("ECA queue to eCPU action channel @ 0x%08x\n", (uint32_t) pECAQ);
  else {
    mprintf("Could not find the ECA queue connected to eCPU action channel. Exit!\n");
    return;
  }

  timer_init(1);          // needed by usleep_init()
  usleep_init();          // needed by scu_mil.c

  isr_table_clr();        // clear interrupt table
  irq_set_mask(0x01);
  irq_disable();          // disable interrupts
}

void main(void) {

  init();           // get own MSI target addr, ECA event input and ECA queue for LM32 channel

  configureEcaMsi(1, gEcaChECPU); // configure ECA for generating MSIs

  initIrqTable();                 // set up MSI handler

  mprintf("waiting for MSI ...\n");

  /* main loop */
  while(1);

}
