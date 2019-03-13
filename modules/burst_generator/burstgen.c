/*******************************************************************************
 *  burstgen.c (derived from lm32 example)
 *
 *  created : 2019, GSI Darmstadt
 *  author  : Dietrich Beck, Enkhbold Ochirsuren
 *  version : 11-Mar-2019
 *
 *  This example demonstrates handling of message-signaled interrupts (MSI)
 *  caused by ECA channel.
 *  ECA is capable to send MSIs on certain conditions such as producing actions
 *  on reception of timing messages.
 *
 *  build: make clean && make TARGET=burstgen
 *  deploy: scp burstgen.bin root@scuxl0304.acc:.
 *  load: eb-fwload dev/wbm0 u 0x0 burstgen.bin
 *  run: eb-reset dev/wbm0 cpureset 0 (assume only one MLM32 is instantiated)
 *  debug: eb-console dev/wbm0
 *
 *  set ECA rules for LM32 action channel:
 *    saft-ecpu-ctl tr0 -d -c 0x1122334455667788 0xFFFFFFFFFFFFFFFF 0 0x42
 *  inject timing message (triggers MSI):
 *    saft-ctl -p tr0 inject 0x1122334455667788 0x8877887766556642 0
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
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"
#include "../../ip_cores/saftlib/drivers/eca_flags.h"
#include "../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"
#include "../wr-unipz/include/wr-unipz.h" // findEcaQueue()

const unsigned char fwName[] = {"burstgen\0"};
const unsigned char errMsgEcaMsi[] = {"Cannot en/disable ECA MSI path to mailbox.\0"};

/* definitions of ECA channels */
#define ECAQMAX           4     // max number of ECA channels in the system
#define ECACHANNELFORLM32 2     // the id of an ECA channel for embedded CPU

/* definitions of timing messages & ECA actions */
#define ECA_VALID_ACTION  0x00040000  // ECA valid action
#define MY_ACT_TAG  0x42              // ECA actions tagged for me
#define LEN_TIM_MSG 0x8               // length of timing message in bytes
uint32_t gEvtId = 0xEEEE0000;         // id of timing message

uint32_t myTimingMsg[LEN_TIM_MSG];  // timing message for IO action channel (will be sent by this LM32)

/* function prototypes */
void injectTimingMsg(uint32_t *msg);  // inject timing message to ECA event input
void ecaHandler(uint32_t);            // pop pending actions from ECA queue

volatile int32_t gBurstCnt = 0;   // flag to start/stop burst generation: stop = 0, start != 0 (positive = number of burst series, negative = endless)

/* ECA conditions for bursts at IO channel */
typedef struct {
  uint32_t nConditions; // number of ECA conditions
  uint32_t tOffset;     // and their offsets
} ecaIoRules_s;

/* pulse frequencies */
enum {
  M_12_5 = 0, // 12,5 MHz
  M_10,      // 10 MHz
  M_2,       // 2 MHz
  M_1,       // 1 MHz
  K_500,     // 500 Khz
  K_100,     // 100 KHz
  K_10,      // 10 KHz
  K_1,       // 1 KHz
  N_FREQ
};

/* pulse generation based on ECA rules for bursts */
ecaIoRules_s ioPulses[N_FREQ] = {
 {250,40} /* 12,5 MHz */, {200,50} /* 10 MHz */, {100,250} /* 2 MHz */,
 {100,500} /* 1 MHz*/, {100,1000} /* 500 KHz */, {100,5000} /* 100 KHz */,
 {10,50000} /* 10 KHz */, {10,500000} /* 1 KHz */};

/* shared memory map for communication via Wishbone  */
#include <burstgen_shared_mmap.h>        // autogenerated upon building firmware

#define NWORDS 2048                      // WARNING: don't exceed 'shared size'! A value of 1024 already requires 8k!

/* stuff required for environment */
extern uint32_t*       _startshared[];
unsigned int cpuId, cpuQty;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;

// global variables
volatile uint32_t *pEcaCtl;         // WB address of ECA control
volatile uint32_t *pEca;            // WB address of ECA event input (discoverPeriphery())
volatile uint32_t *pECAQ;           // WB address of ECA queue
volatile uint32_t *pPsram;          // WB address of psram
volatile uint32_t *pPsram1;         // pointer to buffer in pseudo ram
volatile uint32_t *pPsram2;         // pointer to buffer in pseudo ram
volatile uint32_t *pShared;         // pointer to begin of shared memory region
volatile uint32_t *pSharedBuff1;    // pointer to buffer in shared memory
volatile uint32_t *pSharedBuff2;    // pointer to buffer in shared memory
volatile uint32_t *pCpuRamExternal; // external address (seen from host bridge) of this CPU's RAM
int gEcaChLm32 = 0;                 // ECA channel for an embedded CPU (LM32), connected to ECA queue pointed by pECAQ

/*******************************************************************************
 *
 * Configure ECA to send MSI to embedded soft-core LM32:
 * - MSI is sent on production of actions for the ECA action
 *   channel for LM32
 * - ECA action channel is selected and MSI target address of LM32 is set in the
 *   ECA MSI target register
 *
 * @param[in] enable  Enable or disable ECA MSI
 * @param[in] channel The index of the selected ECA action channel
 *
 ******************************************************************************/
void configureEcaMsi(int enable, uint32_t channel) {

  if (enable != 0 && enable != 1) {
    mprintf("%s: Bad enable argument. %s\n", fwName, errMsgEcaMsi);
    return;
  }

  if (channel > ECAQMAX) {
     mprintf("%s: Bad channel argument. %s\n", fwName, errMsgEcaMsi);
    return;
  }

  atomic_on();
  *(pEcaCtl + (ECA_CHANNEL_SELECT_RW >> 2)) = channel;            // select channel
  *(pEcaCtl + (ECA_CHANNEL_MSI_SET_ENABLE_OWR >> 2)) = 0;         // disable ECA MSI (required to set a target address)
  *(pEcaCtl + (ECA_CHANNEL_MSI_SET_TARGET_OWR >> 2)) = (uint32_t)pMyMsi;  // set MSI destination address as a target address
  *(pEcaCtl + (ECA_CHANNEL_MSI_SET_ENABLE_OWR >> 2)) = enable;    // enable ECA MSI
  atomic_off();

  mprintf("%s: ECA MSI path to LM32 is %s (ECA out chan = %d, MSI dest = 0x%08x)\n",
          fwName, enable==1?"enabled":"disabled", channel, (uint32_t)pMyMsi);
}

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
    // read flag and check if there was an action
    flag         = *(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2));
    if (flag & (0x0001 << ECA_VALID)) {

      // pop action from channel
      *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;
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

  *(pEcaCtl + (ECA_CHANNEL_SELECT_RW >> 2)) = gEcaChLm32;    // select ECA channel for LM32
  valCnt = *(pEcaCtl + (ECA_CHANNEL_VALID_COUNT_GET >> 2));  // get/clear valid count
  mprintf("validCnt=%d\n", valCnt);

  valCnt = clearEcaQueue(valCnt);                            // pop pending actions
  if (valCnt != 0)
    mprintf("%d actions cleared!\n", valCnt);
}

/*******************************************************************************
 *
 * Handle pending valid actions
 *
 ******************************************************************************/
void handleValidActions()
{
  uint32_t valCnt;
  *(pEcaCtl + (ECA_CHANNEL_SELECT_RW >> 2)) = gEcaChLm32;    // select ECA channel for LM32
  valCnt = *(pEcaCtl + (ECA_CHANNEL_VALID_COUNT_GET >> 2));  // read and clear valid counter
  mprintf("valCnt=%d\n", valCnt);

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

  mprintf(" MSI:\t%08x\nAdr:\t%08x\nSel:\t%01x\nCnt:\t%d\n", global_msi.msg, global_msi.adr, global_msi.sel, gBurstCnt);

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
 * Demonstrate exchange of data to Wishbone via shared RAM
 * - the data can be accessed via Etherbone->Wishbone
 * - try eb-read/eb-write from the host system
 *
 ******************************************************************************/
void initSharedMem()
{
  uint32_t i,j;
  uint32_t idx;
  const uint32_t c_Max_Rams = 10;
  sdb_location found_sdb[c_Max_Rams];
  sdb_location found_clu;

  // get pointer to shared memory; internal perspective of this LM32
  pShared        = (uint32_t *)_startshared;                // begin of shared mem
  pSharedBuff1   = (uint32_t *)(pShared +      0);          // 1st buffer in shared memory
  pSharedBuff2   = (uint32_t *)(pShared + NWORDS);          // 2nd buffer in shared memory

  // print pointer info to UART
  mprintf("%s: internal shared memory: start            @ 0x%08x\n", fwName, (uint32_t)pShared);

  // get pointer to shared memory; external perspective from host bridge
  idx = 0;
  find_device_multi(&found_clu, &idx, 1, GSI, LM32_CB_CLUSTER);
  idx = 0;
  find_device_multi_in_subtree(&found_clu, &found_sdb[0], &idx, c_Max_Rams, GSI, LM32_RAM_USER);
  if(idx >= cpuId) {
    pCpuRamExternal = (uint32_t*)(getSdbAdr(&found_sdb[cpuId]) & 0x7FFFFFFF); // CPU sees the 'world' under 0x8..., remove that bit to get host bridge perspective
    // print external WB info to UART
    mprintf("%s: external shared memory: start            @ 0x%08x\n", fwName, (uint32_t)(pCpuRamExternal + (SHARED_OFFS >> 2)));
  } else {
    pCpuRamExternal = (uint32_t*)ERROR_NOT_FOUND;
    mprintf("%s: could not find external WB address of my own RAM !\n", fwName);
  }
}

/*******************************************************************************
 *
 * Find WB address of ECA queue connect to ECA channel for LM32
 *
 * - ECA queue address is set to "pECAQ"
 * - index of ECA channel for LM32 is set to "gEcaChLm32"
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
    mprintf("-- found ECA queue 0x%08x, idx %d\n", (uint32_t)tmp, i);
    if ( *(tmp + (ECA_QUEUE_QUEUE_ID_GET >> 2)) == ECACHANNELFORLM32) {
      pECAQ = tmp;    // update global variables
      gEcaChLm32 = ECACHANNELFORLM32 +1; // refer to eca_queue_regs.h
      i = EcaQ_idx;   // break loop
    }
  }

  if (pECAQ)
    return WRUNIPZ_STATUS_OK;
  else
    return WRUNIPZ_STATUS_ERROR;
}

/*******************************************************************************
*
* Pop pending actions from an ECA queue for LM32 and handle them
*
* This example assumes that
* - action conditions for this lm32 are configured by using saft-ecpu-ctl
*   from the host system with a tag value of 0x42
* - get action data and update the burst counter
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
        mprintf("EvtID: 0x%08x%08x; deadline: 0x%08x%08x; param: 0x%08x%08x; flag: 0x%08x\n",
                evtIdHigh, evtIdLow, evtDeadlHigh, evtDeadlLow, paramHigh, paramLow, flag);
        gBurstCnt = paramLow; // update the burst counter
        break;
      default:
        mprintf("ecaHandler: unknown tag\n");
      } // switch

    } // if data is valid
  }
} // ecaHandler

/*******************************************************************************
 *
 * Construct a timing message
 *
 * @param[out] msg The location of message buffer
 * @param[in]  id  The event id
 *
 ******************************************************************************/
void constructTimingMsg(uint32_t *msg, uint32_t id)
{
  *msg = id; // FID+GID*EVTNO+flags
  *(msg +1) = 0x0; // SID+BPID+resrv
  *(msg +2) = 0x0; // param_up
  *(msg +3) = 0x0; // param_lo
  *(msg +4) = 0x0; // resrv
  *(msg +5) = 0x0; // TEF
  *(msg +6) = 0x0;
  *(msg +7) = 0x0;

  mprintf("\nconstructed timing msg:\n");
  mprintf("event: %x-%x\n",msg[0], msg[1]);
  mprintf("param: %x-%x\n",msg[2], msg[3]);
  mprintf("resrv: %x\n",msg[4]);
  mprintf("TEF  : %x\n",msg[5]);
}

/*******************************************************************************
 *
 * Inject the given timing message to the ECA event input
 *
 * @param msg The location of message buffer
 *
 ******************************************************************************/
void injectTimingMsg(uint32_t *msg)
{
  atomic_on();

  for (int i = 0; i < LEN_TIM_MSG; i++)
    *pEca = msg[i];

  atomic_off();
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

  if (pEca)
    mprintf("Found ECA event input: 0x%08x\n", (uint32_t) pEca);
  else {
    mprintf("No ECA event input found. Exit!\n");
    return;
  }

  mprintf("Found mailbox at 0x%08x. My MSI path is 0x%08x\n", (uint32_t)pCpuMsiBox, (uint32_t)pMyMsi);

  uart_init_hw();         // init UART, required for printf... . To view print message, you may use 'eb-console' from the host
  cpuId = getCpuIdx();    // get ID of THIS CPU

  pEcaCtl = find_device_adr(ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID);

  if (pEcaCtl)
    mprintf("Found ECA control: 0x%08x\n", (uint32_t) pEcaCtl);
  else {
    mprintf("No ECA control found. Exit!\n");
    return;
  }

  if (findEcaQueue() == WRUNIPZ_STATUS_OK)
    mprintf("Addr of ECA queue connected to ECA channel for LM32: 0x%08x\n", (uint32_t) pECAQ);
  else {
    mprintf("No ECA queue connected to LM32 channel found\n");
    return;
  }

  timer_init(1);          // needed by usleep_init()
  usleep_init();          // needed by scu_mil.c

  isr_table_clr();        // set MSI IRQ handler
  irq_set_mask(0x01);     // ...
  irq_disable();          // ...
}

void main(void) {

  uint32_t i, j, mbSlot;

  uint64_t t1, t2;
  uint32_t dt1, dt2, dt3, dt4, dt5, dt6, dt7, dt8;

  uint32_t data1, data2, data3, data4, data5, data6, data7, data8;

  data1 = 0xdeadbeef;
  data2 = 0xcafebabe;

  init();                     // discover mailbox, own MSI path, ECA event input, ECA queue for LM32 channel
  initSharedMem();            // init shared memory

  /* burst generation setup */
  uint64_t t, tPeriod, tInject, tWait, tDeadline;

  // set time interval needed for sending timing messages periodically
  ecaIoRules_s *pulse = &ioPulses[K_500]; // generate pulses with the chosen frequency (stable up to 500KHz)
  tPeriod = pulse->nConditions * pulse->tOffset;

  constructTimingMsg(myTimingMsg, gEvtId << 1); // construct a dummy timing message to obtain the injection duration

  // set up timing message injection
  t = getSysTime();
  tDeadline = getSysTime(); // set deadline to late

  *(myTimingMsg +6) = (uint32_t)((tDeadline >> 32) & 0xFFFFFFFF);
  *(myTimingMsg +7) = (uint32_t)(tDeadline & 0xFFFFFFFF);

  // inject the dummy late message
  injectTimingMsg(myTimingMsg);

  tInject = getSysTime() - t; // get injection duration
  tInject <<= 1;
  mprintf("\n%s: duration of injection (hex ns) = 0x%x%08x\n", fwName, (uint32_t) (tInject >> 32), (uint32_t) tInject );

  constructTimingMsg(myTimingMsg, gEvtId); // construct timing message for IO action channel

  /*mbSlot = getMsiBoxSlot(0x0); // mailbox will be used for MSI message delivery from LM32 to host

  if (mbSlot == -1)  {
    mprintf("No free slots in mailbox. Exit!\n");
    return;
  }
  else  {
    mprintf("Configured slot %d in mailbox\n", mbSlot);
  }*/

  // clean ECA queue and channel from previous actions
  clearActions();

  /* MSI hanlder setup */
  configureEcaMsi(1, gEcaChLm32); // allow MSI path from ECA to itself

  initIrqTable();              // set up MSI handler

  gBurstCnt = 0;

  mprintf("waiting for MSI ...\n");

  /* main loop */
  while(1) {

    tDeadline = getSysTime(); // update deadline

    // repeat the timing message injection until the burst counter gets null
    while (gBurstCnt != 0) {

      tDeadline += tPeriod;   // set next deadline
      *(myTimingMsg +6) = (uint32_t)((tDeadline >> 32) & 0xFFFFFFFF);
      *(myTimingMsg +7) = (uint32_t)(tDeadline & 0xFFFFFFFF);

      do {
        t = getSysTime();
      } while ((tDeadline - t) > tInject && tDeadline > t); // wait until setup due or late!

      injectTimingMsg(myTimingMsg);

      if (gBurstCnt > 0) {
        if (--gBurstCnt == 0)
          mprintf("burst gen completed: Cnt=%d\n", gBurstCnt);
      }
    };

    t1 = getSysTime();

    // wait for 10 seconds
    do {
      t2 = getSysTime() - t1;
      if (gBurstCnt != 0)
        break;      // break waiting on new loop value
      else
        asm("nop");
    } while (t2 < 10000000000);

    mprintf("idle: elapsed %d ms\n",(uint32_t)(t2 / 1000000));
  }

#if 0
  // write to shared ram
  t1 = getSysTime();
  for (i=0; i<NWORDS; i++) {
    pSharedBuff1[i] = data1;
  } // for i
  t2 = getSysTime();
  dt1 = (uint32_t)(t2 -t1);

  // copy between shared ram buffers
  t1 = getSysTime();
  for (i=0; i<NWORDS; i++) {
    pSharedBuff2[i] = pSharedBuff1[i];
  } // for i
  t2 = getSysTime();
  dt2 = (uint32_t)(t2 -t1);

  // read from shared ram and check values
  for (i=0; i<NWORDS; i++) {
    if (pSharedBuff2[i] != data1) mprintf("%s: shared mem messed up\n", fwName);
  } // for i

  // read from shared ram
  t1 = getSysTime();
  for (i=0; i<NWORDS; i++) {
    if (data3 = pSharedBuff2[i]);
  } // for i
  t2    = getSysTime();
  dt3   = (uint32_t)(t2 -t1);

  mprintf("writing %d * 0x%x (32bit) took %7u ns or %4u ns/word or %4u Mbit/s (shared ram)\n",       NWORDS, data1, dt1, dt1/(NWORDS), (1000 * NWORDS * 32)/dt1);
  mprintf("copying %d * 0x%x (32bit) took %7u ns or %4u ns/word or %4u Mbit/s (shared -> shared)\n", NWORDS, data1, dt2, dt2/(NWORDS), (1000 * NWORDS * 32)/dt2);
  mprintf("reading %d * 0x%x (32bit) took %7u ns or %4u ns/word or %4u Mbit/s (shared ram)\n",       NWORDS, data3, dt3, dt3/(NWORDS), (1000 * NWORDS * 32)/dt3);
#endif

}  /* main */
