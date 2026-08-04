#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "dbg.h"
#include "ftm_common.h"
#include "dm.h"
#include "dm_hal.h"
// #include "config.h"
#include "uart.h"

/** \mainpage DM Firmware Documentation
 *
 * \section intro_sec Introduction
 * This document describes the firmware of the Data Master (DM) module. The firmware is responsible for timing message generation to control all WR timing receiver platforms within the GSI/FAIR facility. For more in depth information,
 * see FAIR the tech note F-TN-C-0015e 'CarpeDM - Programming language for the DataMaster'.
 *
 * \section desc_sec Description
 * \subsection env Environment
 * This firmware runs on LM32 cpus within the Data Master (DM) gateware. The difference to standard timing receiver images
 * lies in the lack of an Event Condition Action (ECA) unit, 4 or more LM32 CPU instances with dual port memories accessible from the host controller
 * and a dedicated hardware priority queue (PQ). The PQ aggregates and sorts timing messages by urgency before forwarding them to the Etherbone Master (EBM) module for dispatch to the White Rabbit (WR) network.
 *
 * \subsection func Functionality
 * The DM firmware processes timing schedules, which are loaded into the CPU's shared memory area by the host controller. These schedules are linked lists
 * of data nodes with differing functions and properties. Their main purpose is dynamic generation of timing messages for broadcasts within the WR network.
 *
 * \subsubsection sched Timing Schedules
 * Schedules are organised as sequences of 0..n functional nodes followed by a block node. The block has a duration (period),
 * which is added to the running time sum of the associated thread. All other node types have relative time offset. Any node's absolute deadline is calculated by adding its offset to its thread's current time sum.
 * (see dlEvt(), dlBlock() ...)
 *
 * \subsubsection edf Scheduler
 * Deadlines, along with a pointer to the corresponding nodes, are fed into the Earliest Deadline First (EDF) scheduler running in the main loop. It always chooses the most urgent deadline/node for processing.
 * While a standard EDF does not have idle behaviour if there is work pending, the DM only processes nodes with deadlines falling into a 1ms window from the current time.
 * The scheduler loop also handles thread Start, Stop and Abort commands from the host.
 * (see main(), heapReplace() ...)
 *
 * \subsubsection proc Node Handlers
 * For each node type, an appropriate handler function is supplied (see tmsg(), block(), ...). Upon execution, the handler provides the specific node function and then returns a successor node with a new deadline to the scheduler.
 * Block nodes are a special case, as they have individual command queues. These can be imagined as parallel inboxes, which can receive asynchronous commands from the host.
 * The content of these commands influences the schedule runtime behaviour, in particular the block's specificfunction, returned successor and deadline. (see execFlow(), execFlush() ...)
 */

uint8_t cpuId;
uint8_t cpuQty;

/// Debug Interrupt console output
/** Shows and MSI's msg, address and byte select words */
void show_msi()
{
  halShowMsi();
}

/// Interrupt Handler 0 (not used)
/** IRQ handler 0, shows handler number and msi content on console. Not used in DM */
void isr0()
{
  pp_printf("ISR0\n");
  show_msi();
}

/// Interrupt Handler 1 (not used)
/** IRQ handler 1, shows handler number and msi content on console. Not used in DM */
void isr1()
{
  pp_printf("ISR1\n");
  show_msi();
}

/// Etherbone Master init routine
/** EBM init. Waits for WR core to receive IP from bootp and then sets src & dst MAC and IP addresses in EBM. */
void ebmInit()
{
  pp_printf("#%02u: DM cores Waiting for IP from WRC...\n", cpuId);
  halEbmWaitForIp();

  halEbmInit();
  halEbmConfigMeta(1500, 42, HAL_EBM_NOREPLY);                              // MTU, max EB msgs, flags
  halEbmConfigIf(HAL_EBM_DESTINATION, 0xffffffffffff, 0xffffffff, 0xebd0);  // Dst: EB broadcast
  halEbmConfigIf(HAL_EBM_SOURCE, 0xd15ea5edbeef, halEbmGetSrcIp(), 0xebd0); // Src: bogus mac (will be replaced by WR), WR IP
}

/// Global init. Discovers periphery and inits all modules.
/** Global init. Discovers periphery, initialises EBM and PQ, checks WR, inits DM and diagnostics and signals readiness on console. */
void init()
{
  dmInitSharedMemPointers();

  *status = 0;
  *count = 0;

  halInitPeriphery();

  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_THR_STA] = SHCTL_THR_STA;
  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_THR_DAT] = SHCTL_THR_DAT;
  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_HEAP] = SHCTL_HEAP;
  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_REGS] = SHCTL_REGS;
  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_END] = _SHCTL_END_;

  if (cpuId == 0)
  {
    // TODO replace bogus system status flags by real ones
    halUartInitHw();
    *status |= SHCTL_STATUS_UART_INIT_SMSK;
    ebmInit();
    *status |= SHCTL_STATUS_EBM_INIT_SMSK;
    halPrioQueueInit();
    *status |= SHCTL_STATUS_PQ_INIT_SMSK;
    // mprintf("#%02u: Got IP from WRC. Configured EBM and PQ\n", cpuId);
  }
  else
  {
    *status |= SHCTL_STATUS_UART_INIT_SMSK;
    *status |= SHCTL_STATUS_EBM_INIT_SMSK;
    *status |= SHCTL_STATUS_PQ_INIT_SMSK;
  }

  int j;

  while (!halWrTimeValid())
  {
    for (j = 0; j < (125000000 / 2); ++j)
    {
      asm("nop");
    }
    if (cpuId == 0)
      pp_printf("#%02u: DM cores Waiting for WRC synchronisation...\n", cpuId);
  }
  if (cpuId == 0)
    pp_printf("#%02u: WR time now in sync\n", cpuId);

  halIrqSetup(0x01);

  dmInit();
  *status |= SHCTL_STATUS_DM_INIT_SMSK;
  *boottime = halGetSysTime();
}

/// Data master main routine. Inits everything and then runs EDF scheduler loop forever.
/** Data master main routine. Inits and the runs EDF scheduler loop, while heeding thread control bits.

    Main loop priorities are 1. Abort 2. Thread processing, including stops 3. start new threads.

    Abort works as follows:
    1. Abort bits set corresponding threads' running bits to 0
    2. Abort bits set corresponding threads' deadlines to MAX_INT
    3. Whole EDF heap is sorted

    Thread processing is done in four steps:
    1. Check if top element is within due time window (current time + 1ms. This is sufficient lead time for processing, network lag, etc)
    2. If so, get node type and call appropriate handler. Process all side effects and return successor node.
    3. Calculate new deadline for succesor node
    4. Create temporary heap element from new deadline and successor node
    5. Replace heap top element by temp element and sort
    (if thread reached a stop, its deadline is now MAX_INT and it ends up at the bottom of the heap)

    Start works as follows:
    1. Start bits sets threads' running bits to 1
    2. Start bits calculate threads' deadlines
    3. Threads cursors are set to their corresponding origins
    4. Whole EDF heap is sorted
    */

void main(void)
{

  int j;

  init();

  // FIXME why is uart_hw_init here twice ???
  //  wait 1s + cpuIdx * 1/10s
  for (j = 0; j < ((125000000 / 4) + (cpuId * 2500000)); ++j)
  {
    asm("nop");
  }
  if (cpuId != 0)
    halUartInitHw();
  *status |= SHCTL_STATUS_UART_INIT_SMSK;

  halAtomicOn();

  pp_printf("#%02u: Rdy\n", cpuId);
#if DEBUGLEVEL != 0
  pp_printf("#%02u: Debuglevel %u. Don't expect timeley delivery with console outputs on!\n", cpuId, DEBUGLEVEL);
#endif
#if DEBUGTIME == 1
  pp_printf("#%02u: Debugtime mode ON. Par Field of Msgs will be overwritten be dispatch time at lm32\n", cpuId);
#endif
#if DEBUGPRIOQ == 1
  pp_printf("#%02u: Priority Queue Debugmode ON, timestamps will be written to 0x%08x on receivers", cpuId, DEBUGPRIOQDST);
#endif
  // mprintf("Found MsgBox at 0x%08x. MSI Path is 0x%08x\n", (uint32_t)pCpuMsiBox, (uint32_t)pMyMsi);
  pp_printf("#%02u: This is %s DM FW %s \n", cpuId, DM_RELEASE, DM_VERSION);

  halAtomicOff();

  if (halGetMsiBoxCpuSlot(cpuId, 0) == -1)
  {
    pp_printf("#%02u: Mail box slot acquisition failed\n", cpuId);
  }

  DBPRINT1("#%02u: Base shared ram 0x%08x\n", cpuId, halGetSharedMemBase());

  for (;;)
  {
    dmSchedulerStep();
    halOnlyMockDoesStuff();
  }
}