#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <pp-printf.h>
#include "mprintf.h"
#include "mini_sdb.h"
#include "irq.h"
#include "ebm.h"
#include "aux.h"
#include "dbg.h"
#include "ftm_common.h"
#include "dm.h"


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
  mprintf(" Msg:\t%08x\nAdr:\t%08x\nSel:\t%01x\n", global_msi.msg, global_msi.adr, global_msi.sel);

}

/// Interrupt Handler 0 (not used)
/** IRQ handler 0, shows handler number and msi content on console. Not used in DM */
void isr0()
{
   mprintf("ISR0\n");
   show_msi();
}

/// Interrupt Handler 1 (not used)
/** IRQ handler 1, shows handler number and msi content on console. Not used in DM */
void isr1()
{
   mprintf("ISR1\n");
   show_msi();
}


/// Etherbone Master init routine
/** EBM init. Waits for WR core to receive IP from bootp and then sets src & dst MAC and IP addresses in EBM. */
void ebmInit()
{

   int j;

   while (*(pEbCfg + (EBC_SRC_IP>>2)) == EBC_DEFAULT_IP) {
     for (j = 0; j < (125000000/2); ++j) { asm("nop"); }
     pp_printf("#%02u: DM cores Waiting for IP from WRC...\n", cpuId);
   }

   ebm_init();
   ebm_config_meta(1500, 42, EBM_NOREPLY );                                         //MTU, max EB msgs, flags
   ebm_config_if(DESTINATION, 0xffffffffffff, 0xffffffff,                0xebd0);   //Dst: EB broadcast
   ebm_config_if(SOURCE,      0xd15ea5edbeef, *(pEbCfg + (EBC_SRC_IP>>2)), 0xebd0); //Src: bogus mac (will be replaced by WR), WR IP

}

/// Global init. Discovers periphery and inits all modules.
/** Global init. Discovers periphery, initialises EBM and PQ, checks WR, inits DM and diagnostics and signals readiness on console. */
void init()
{
  *status = 0;
  *count  = 0;


  discoverPeriphery();
  cpuId = getCpuIdx();

  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_THR_STA] = SHCTL_THR_STA;
  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_THR_DAT] = SHCTL_THR_DAT;
  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_HEAP]    = SHCTL_HEAP;
  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_REGS]    = SHCTL_REGS;
  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_END]     = _SHCTL_END_;


  if (cpuId == 0) {
    //TODO replace bogus system status flags by real ones
    uart_init_hw();   *status |= SHCTL_STATUS_UART_INIT_SMSK;
    ebmInit();        *status |= SHCTL_STATUS_EBM_INIT_SMSK ;
    prioQueueInit();  *status |= SHCTL_STATUS_PQ_INIT_SMSK;
    //mprintf("#%02u: Got IP from WRC. Configured EBM and PQ\n", cpuId);
  } else {
    *status |= SHCTL_STATUS_UART_INIT_SMSK;
    *status |= SHCTL_STATUS_EBM_INIT_SMSK ;
    *status |= SHCTL_STATUS_PQ_INIT_SMSK;
  }

  int j;


  while(!wrTimeValid()) {
    for (j = 0; j < (125000000/2); ++j) { asm("nop"); }
    if (cpuId == 0) mprintf("#%02u: DM cores Waiting for WRC synchronisation...\n", cpuId);
  }
  if (cpuId == 0) mprintf("#%02u: WR time now in sync\n", cpuId);

  isr_table_clr();
  irq_set_mask(0x01);
  irq_disable();

  dmInit();
  *status  |= SHCTL_STATUS_DM_INIT_SMSK;
  *boottime = getSysTime();

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






void main(void) {


  int i,j;

  uint32_t* tp;
  uint32_t** np;
  uint32_t backlog = 0;


  init();

  //FIXME why is uart_hw_init here twice ???
  // wait 1s + cpuIdx * 1/10s
  for (j = 0; j < ((125000000/4)+(cpuId*2500000)); ++j) { asm("nop"); }
  if (cpuId != 0) uart_init_hw();   *status |= SHCTL_STATUS_UART_INIT_SMSK;

  atomic_on();

  mprintf("#%02u: Rdy\n", cpuId);
  #if DEBUGLEVEL != 0
    mprintf("#%02u: Debuglevel %u. Don't expect timeley delivery with console outputs on!\n", cpuId, DEBUGLEVEL);
  #endif
  #if DEBUGTIME == 1
    mprintf("#%02u: Debugtime mode ON. Par Field of Msgs will be overwritten be dispatch time at lm32\n", cpuId);
  #endif
  #if DEBUGPRIOQ == 1
    mprintf("#%02u: Priority Queue Debugmode ON, timestamps will be written to 0x%08x on receivers", cpuId, DEBUGPRIOQDST);
  #endif
  //mprintf("Found MsgBox at 0x%08x. MSI Path is 0x%08x\n", (uint32_t)pCpuMsiBox, (uint32_t)pMyMsi);
  mprintf("#%02u: This is %s DM FW %s \n", cpuId, DM_RELEASE, DM_VERSION);

  atomic_off();

  if (getMsiBoxCpuSlot(cpuId, 0) == -1) {mprintf("#%02u: Mail box slot acquisition failed\n", cpuId);}

  DBPRINT1("#%02u: Base shared ram 0x%08x\n", cpuId, (uint32_t*)&_startshared);


  for (j = 0; j < ((125000000/4)); ++j) { asm("nop"); }



   while (1) {


    // Hard abort is an emergency and gets priority over everything else
    if (*abort1) {
      *running &= ~(*abort1);   // clear all aborted running bits
      for(i=0; i<_THR_QTY_; i++) {
        uint64_t* deadline  = (uint64_t*)&p[( SHCTL_THR_DAT + i * _T_TD_SIZE_ + T_TD_DEADLINE ) >> 2];
        *deadline |= (~((uint64_t)*abort1 >> i) & 1) -1;  // if abort bit was set, move deadline to infinity
      }
      heapify(); // re-sort all threads in schedulder (necessary because multiple threads may have been aborted
      *abort1 = 0; // clear abort bits
    }

    //the workhorse. check if most urgent node is due and process it if this is the case.
    uint8_t thrIdx = *(uint32_t*)(pT(hp) + (T_TD_FLAGS >> 2));
    if (DL(pT(hp))  <= getSysTime() + *(uint64_t*)(p + (( SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_PREPTIME   ) >> 2) )) {
      //node is due. Execute it, then update cursor and deadline, return control to scheduler
      backlog++;

      ///check if the node uses fields with references
      if (!hasNodeDynamicFields(pN(hp))) {
        //no dynamic fields. do go as normal on, nothing to see here
        //FIXME Why not pncN(hp) = nodeFuncs[ ...?
        *pncN(hp)   = (uint32_t)nodeFuncs[getNodeType(pN(hp))](pN(hp), pT(hp));       //process node and return thread's next node
      } else {
        //We got some dynamic fields. Now:
        // do a copy of original node
        // insert all dynamic fields
        // call appropriate node handler
        // write back all changes of immediate/val fields to original
        //
        *pncN(hp)   = (uint32_t)dynamicNodeStaging(pN(hp), pT(hp));  
      }
      DL(pT(hp))  = (uint64_t)deadlineFuncs[getNodeType(pN(hp))](pN(hp), pT(hp));   // return thread's next deadline (returns infinity on upcoming NULL ptr)
      *running   &= ~((DL(pT(hp)) == -1ULL) << thrIdx);                             // clear running bit if deadline is at infinity
      heapReplace(0);                                                               // call scheduler, re-sort only current thread

    } else {
      //nothing due right now. Check for requests of new threads to be started
      *backlogmax   = ((backlog > *backlogmax) ? backlog : *backlogmax);
      backlog = 0;
      uint64_t snapshotSysTime = getSysTime();

      if(*start) { //check start bitfield for any request
        for(i=0;i<_THR_QTY_;i++) { //iterate
          if (*start & (1<<i)) {

            if(*running & (1<<i)){
              //already running, error
              *status |= SHCTL_STATUS_THR_RESTART_ERROR_TYPE_SMSK;
              break;
            }

            //current thread base pointers
            uint8_t* thrStart  = (uint8_t*)&p[( SHCTL_THR_STA + i * _T_TS_SIZE_) >> 2]; // thread Start array
            uint8_t* thrData   = (uint8_t*)&p[( SHCTL_THR_DAT + i * _T_TD_SIZE_) >> 2]; // thread Data array

      //pointers to start fields
            volatile uint64_t* startTime = (uint64_t*)&thrStart[T_TS_STARTTIME];
            volatile uint64_t* prepTime  = (uint64_t*)&thrStart[T_TS_PREPTIME];
            volatile uint32_t* origin    = (uint32_t*)&thrStart[T_TS_NODE_PTR];

      //pointers to data fields
            uint64_t* currTime  = (uint64_t*)&thrData[T_TD_CURRTIME];
            uint64_t* deadline  = (uint64_t*)&thrData[T_TD_DEADLINE];
            uint32_t* cursor    = (uint32_t*)&thrData[T_TD_NODE_PTR];
            uint32_t* msgcnt    = (uint32_t*)&thrData[T_TD_MSG_CNT];

            DBPRINT1("#%02u: ThrIdx %u, Preptime: %s\n", cpuId, i, print64(*prepTime, 0));

            //init fields
            uint64_t snapshotStartTime = *startTime;
            if (!(snapshotStartTime)) { *currTime = snapshotSysTime + (*prepTime << 1); } // if 0, set to now + 2 * preptime
            else                        *currTime = snapshotStartTime;

            if(*currTime < snapshotSysTime) {
              //late start detected
              *status |= SHCTL_STATUS_LATE_START_ERROR_TYPE_SMSK;
              break;
            }

            *cursor   = *origin;          // Set cursor to origin node
            *deadline = *currTime;        // Set the deadline to first blockstart
            //if first node is an event, starttime must be increment by its offset. Call deadline update function to handle this
            *deadline = (uint64_t)deadlineFuncs[getNodeType((uint32_t*)*cursor)]((uint32_t*)*cursor, (uint32_t*)thrData);


            *running |= *start & (1<<i);  // copy this start bit to running bits
            *start   &= ~(1 << i);        // clear this start bit
            *msgcnt   = 0;                // clear msg counter
          }
        }

        heapify(); // re-sort all threads in schedulder (necessary because multiple threads may have been started)
      }
    }

  }
}
