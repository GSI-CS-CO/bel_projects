/*!
 *  @file scu_main.c
 *  @brief Main module of SCU function generators in LM32.
 *
 *  @date 10.07.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *  Origin Stefan Rauch
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

#include <stack.h>
#include "scu_main.h"
#include "scu_eca_handler.h"
#include "scu_command_handler.h"
#include "scu_fg_macros.h"
#ifdef CONFIG_MIL_FG
#include "scu_mil_fg_handler.h"
#endif
#include "scu_fg_handler.h"
#include "scu_temperature.h"
#ifdef CONFIG_SCU_DAQ_INTEGRATION
 #include "daq_main.h"
#endif

typedef enum
{
   MIL_INL = 0x00,
   MIL_DRY = 0x01,
   MIL_DRQ = 0x02
} MIL_T;

extern ONE_WIRE_T g_oneWireBase;

/*====================== Begin of shared memory area ========================*/
/*!
 * @brief Memory space of shared memory for communication with Linux-host
 *        and initializing of them.
 */
SCU_SHARED_DATA_T SHARED g_shared = SCU_SHARED_DATA_INITIALIZER;
/*====================== End of shared memory area ==========================*/

/*!
 * @brief Base pointer of SCU bus.
 * @see initializeGlobalPointers
 */
volatile uint16_t*     g_pScub_base       = NULL;

/*!
 * @brief Base pointer of irq controller for SCU bus
 * @see initializeGlobalPointers
 */
volatile uint32_t*     g_pScub_irq_base   = NULL;

#ifdef CONFIG_MIL_FG
/*!
 * @brief Base pointer of MIL extension macro
 * @see initializeGlobalPointers
 */
volatile unsigned int* g_pScu_mil_base    = NULL;

/*!
 * @brief Base pointer of IRQ controller for dev bus extension
 * @see initializeGlobalPointers
 */
volatile uint32_t*     g_pMil_irq_base    = NULL;
#endif /* CONFIG_MIL_FG */

/*!
 * @brief  Memory space of message queue.
 */
volatile FG_MESSAGE_BUFFER_T g_aMsg_buf[QUEUE_CNT] = {{0, 0}};

/*===========================================================================*/
/*! ---------------------------------------------------------------------------
 * @brief Initializing of all global pointers accessing the hardware.
 */
STATIC inline void initializeGlobalPointers( void )
{
   discoverPeriphery();
   uart_init_hw();
   initOneWire();
   /* additional periphery needed for scu */
   g_pScub_base       = (volatile uint16_t*)find_device_adr(GSI, SCU_BUS_MASTER);
   g_pScub_irq_base   = (volatile uint32_t*)find_device_adr(GSI, SCU_IRQ_CTRL);
#ifdef CONFIG_MIL_FG
   g_pScu_mil_base    = (unsigned int*)find_device_adr(GSI, SCU_MIL);
   g_pMil_irq_base    = (volatile uint32_t*)find_device_adr(GSI, MIL_IRQ_CTRL);
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief enables msi generation for the specified channel. \n
 * Messages from the scu bus are send to the msi queue of this cpu with the offset 0x0. \n
 * Messages from the MIL extension are send to the msi queue of this cpu with the offset 0x20. \n
 * A hardware macro is used, which generates msis from legacy interrupts. \n
 * @param channel number of the channel between 0 and MAX_FG_CHANNELS-1
 * @see disable_slave_irq
 */
void enable_scub_msis( const unsigned int channel )
{
   if( channel >= MAX_FG_CHANNELS )
      return;

   //FG_ASSERT( pMyMsi != NULL );

   const uint8_t socket = getSocket( channel );

   if( isNonMilFg( socket ) || isMilScuBusFg( socket ) )
   {
      //SCU Bus Master
      FG_ASSERT( getFgSlotNumber( socket ) > 0 );
      const uint16_t slot = getFgSlotNumber( socket ) - 1;
      g_pScub_base[GLOBAL_IRQ_ENA] = 0x20;
      g_pScub_irq_base[8]  = slot;            // channel select
      g_pScub_irq_base[9]  = slot;            // msg: socket number
      g_pScub_irq_base[10] = (uint32_t)pMyMsi + 0x0;    // msi queue destination address of this cpu
      g_pScub_irq_base[2]  = (1 << slot); // enable slave
      //mprintf("IRQs for slave %d enabled.\n", (socket & SCU_BUS_SLOT_MASK));
      return;
   }

#ifdef CONFIG_MIL_FG
   if( !isMilExtentionFg( socket ) )
      return;

   g_pMil_irq_base[8]   = MIL_DRQ;
   g_pMil_irq_base[9]   = MIL_DRQ;
   g_pMil_irq_base[10]  = (uint32_t)pMyMsi + 0x20; //TODO Who the fuck is 0x20?!?
   g_pMil_irq_base[2]   = (1 << MIL_DRQ);
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief delay in multiples of one millisecond
 *  uses the system timer
 *  @param ms delay value in milliseconds
 */
STATIC void msDelayBig( const uint64_t ms )
{
   const uint64_t later = getSysTime() + ms * 1000000ULL / 8;
   while( getSysTime() < later )
   {
      asm volatile ("nop");
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_main.h
 */
void clear_handler_state( const uint8_t socket )
{
   MSI_T m;

   if( isMilScuBusFg( socket ) )
   {
      // create swi
      FG_ASSERT( getFgSlotNumber( socket ) > 0 );
      m.msg = getFgSlotNumber( socket ) - 1;
      m.adr = 0;
      irq_disable();
      add_msg( &g_aMsg_buf[0], DEVSIO, m );
      irq_enable();
      return;
   }

   if( isMilExtentionFg( socket ) )
   {
      m.msg = 0;
      m.adr = 0;
      irq_disable();
      add_msg(&g_aMsg_buf[0], DEVBUS, m );
      irq_enable();
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Prints all found function generators.
 */
STATIC inline void printFgs( void )
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_shared.fg_macros ); i++ )
   {
      if( g_shared.fg_macros[i].outputBits == 0 )
         break;
      mprintf( "fg-%d-%d\tver: %d output-bits: %d\n",
               g_shared.fg_macros[i].socket,
               g_shared.fg_macros[i].device,
               g_shared.fg_macros[i].version,
               g_shared.fg_macros[i].outputBits
             );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Print the values and states of all channel registers.
 */
inline STATIC void print_regs( void)
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_shared.fg_regs ); i++ )
   {
      mprintf("channel[%d].wr_ptr %d\n", i, g_shared.fg_regs[i].wr_ptr);
      mprintf("channel[%d].rd_ptr %d\n", i, g_shared.fg_regs[i].rd_ptr);
      mprintf("channel[%d].mbx_slot 0x%x\n", i, g_shared.fg_regs[i].mbx_slot);
      mprintf("channel[%d].macro_number %d\n", i, g_shared.fg_regs[i].macro_number);
      mprintf("channel[%d].ramp_count %d\n", i, g_shared.fg_regs[i].ramp_count);
      mprintf("channel[%d].tag 0x%x\n", i, g_shared.fg_regs[i].tag);
      mprintf("channel[%d].state %d\n", i, g_shared.fg_regs[i].state);
      mprintf("\n");
   }
}

/*! ---------------------------------------------------------------------------
 * @brief as short as possible, just pop the msi queue of the cpu and
 *         push it to the message queue of the main loop
 * @see init_irq_table
 * @see _irq_entry
 * @see irq_pop_msi
 * @see dispatch
 */
void irq_handler( void )
{
   MSI_T m;

  // send msi threadsafe to main loop
   m.msg = global_msi.msg;
   m.adr = global_msi.adr;
   add_msg( &g_aMsg_buf[0], IRQ, m );
}

/*! ---------------------------------------------------------------------------
 * @brief initialize the irq table and set the irq mask
 */
STATIC void init_irq_table( void )
{
   isr_table_clr();
   isr_ptr_table[0] = &irq_handler;
   irq_set_mask(0x01);
   g_aMsg_buf[IRQ].ring_head = g_aMsg_buf[IRQ].ring_tail; // clear msg buffer
   irq_enable();
   mprintf("IRQ table configured. 0x%x\n", irq_get_mask());
}

/*! ---------------------------------------------------------------------------
 * @brief initialize procedure at startup
 */
STATIC void initAndScan( void )
{
   hist_init(HISTORY_XYZ_MODULE);
   for( int i = 0; i < ARRAY_SIZE(g_shared.fg_regs); i++ )
      g_shared.fg_regs[i].macro_number = SCU_INVALID_VALUE;     //no macros assigned to channels at startup
   updateTemperature();                       //update 1Wire ID and temperatures
   scanFgs();                        //scans for slave cards and fgs
}

/*! ---------------------------------------------------------------------------
 * @brief segfault handler, not used at the moment
 */
void _segfault( void )
{
   mprintf(ESC_ERROR"KABOOM!"ESC_NORMAL"\n");
  //while (1) {}
}

/*! ---------------------------------------------------------------------------
 * @brief Scans for fgs on mil extension and scu bus.
 */
void scanFgs( void )
{
#ifdef CONFIG_USE_RESCAN_FLAG
   g_shared.fg_rescan_busy = 1; //signal busy to saftlib
#endif
#if defined( CONFIG_READ_MIL_TIME_GAP ) && defined( CONFIG_MIL_FG )
   suspendGapReading();
#endif
#if __GNUC__ >= 9
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
   scan_all_fgs( g_pScub_base,
              #ifdef CONFIG_MIL_FG
                 g_pScu_mil_base,
              #endif
                 &g_shared.fg_macros[0],
                 &g_shared.ext_id );
#if __GNUC__ >= 9
  #pragma GCC diagnostic pop
#endif
#ifdef CONFIG_USE_RESCAN_FLAG
   g_shared.fg_rescan_busy = 0; //signal done to saftlib
#endif
   printFgs();
}

/* task prototypes */
#ifndef __DOXYGEN__
#ifdef CONFIG_SCU_DAQ_INTEGRATION
static void scuBusDaqTask( register TASK_T* FG_UNUSED );
#endif
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Task configuration table.
 * @see schedule
 */
STATIC TASK_T g_aTasks[] =
{
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   { NULL,               ALWAYS, 0, scuBusDaqTask   },
#endif
#ifdef CONFIG_MIL_FG
   { &g_aMilTaskData[0], ALWAYS, 0, dev_sio_handler }, // sio task 1
 //!!  { &g_aMilTaskData[1], ALWAYS, 0, dev_sio_handler }, // sio task 2
 //!!  { &g_aMilTaskData[2], ALWAYS, 0, dev_sio_handler }, // sio task 3
 //!!  { &g_aMilTaskData[3], ALWAYS, 0, dev_sio_handler }, // sio task 4
   { &g_aMilTaskData[4], ALWAYS, 0, dev_bus_handler },
#endif
   { NULL,               ALWAYS, 0, scu_bus_handler },
   { NULL,               ALWAYS, 0, ecaHandler      },
   { NULL,               ALWAYS, 0, commandHandler  }
};

/*! ---------------------------------------------------------------------------
 * @brief Move messages to the correct queue, depending on source
 * @see irq_handler
 * @see schedule
 */
STATIC inline void dispatch( void )
{
   const MSI_T m = remove_msg( &g_aMsg_buf[0], IRQ );
   switch( m.adr & 0xFF )
   { //TODO remove these naked numbers asap!
      case 0x00: add_msg( &g_aMsg_buf[0], SCUBUS, m ); return; // message from scu bus
      case 0x10: add_msg( &g_aMsg_buf[0], SWI,    m ); return; // software message from saftlib
      case 0x20: add_msg( &g_aMsg_buf[0], DEVBUS, m ); return; // message from dev bus
   }
}

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Scheduler for all SCU-tasks defined in g_aTasks. \n
 *        Performing of a cooperative multitasking.
 * @see TASK_T
 * @see g_aTasks
 * @see dev_sio_handler
 * @see dev_bus_handler
 * @see scu_bus_handler
 * @see ecaHandler
 * @see commandHandler
 * @see scuBusDaqTask
 */
STATIC inline void schedule( void )
{
   const uint64_t tick = getSysTime();

   for( unsigned int i = 0; i < ARRAY_SIZE( g_aTasks ); i++ )
   {
      // call the dispatch task before every other task
      dispatch();
      TASK_T* pCurrent = &g_aTasks[i];
      if( (tick - pCurrent->lasttick) < pCurrent->interval )
      {
         continue;
      }
      pCurrent->func( pCurrent );
      pCurrent->lasttick = tick;
   }
}

#ifdef CONFIG_SCU_DAQ_INTEGRATION
/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @ingroup TASK
 * @brief Handles all detected non-MIL DAQs
 * @see schedule
 */
STATIC void scuBusDaqTask( register TASK_T* pThis FG_UNUSED )
{
   FG_ASSERT( pThis->pTaskData == NULL );
   forEachScuDaqDevice();
}
#endif /* ifdef CONFIG_SCU_DAQ_INTEGRATION */

/*! ---------------------------------------------------------------------------
 * @brief Helper function for printing the CPU-ID and the number of
 *        MSI endpoints.
 */
STATIC inline void printCpuId( void )
{
   unsigned int* cpu_info_base;
   cpu_info_base = (unsigned int*)find_device_adr(GSI, CPU_INFO_ROM);
   if((int)cpu_info_base == ERROR_NOT_FOUND)
   {
      mprintf(ESC_ERROR"no CPU INFO ROM found!"ESC_NORMAL"\n");
      return;
   }
   mprintf("CPU ID: 0x%x\n", cpu_info_base[0]);
   mprintf("number MSI endpoints: %d\n", cpu_info_base[1]);
}

/*! ---------------------------------------------------------------------------
 * @brief Tells saftlib the mailbox slot for sw irqs
 */
STATIC inline void tellMailboxSlot( void )
{
   const int slot = getMsiBoxSlot(0x10); //TODO Where does 0x10 come from?
   if( slot == -1 )
      mprintf( ESC_ERROR"No free slots in MsgBox left!"ESC_NORMAL"\n" );
   else
      mprintf( "Configured slot %d in MsgBox\n", slot );
   g_shared.fg_mb_slot = slot;
}

/*================================ MAIN =====================================*/
void main( void )
{
   initializeGlobalPointers();
   mprintf("Compiler: "COMPILER_VERSION_STRING"\n"
           "Git revision: "TO_STRING(GIT_REVISION)"\n"
           "Found MsgBox at 0x%08x. MSI Path is 0x%08x\n",
           (uint32_t)pCpuMsiBox, (uint32_t)pMyMsi);
#if defined( CONFIG_MIL_FG ) && defined( CONFIG_READ_MIL_TIME_GAP )
   mprintf( ESC_WARNING"CAUTION! Time gap reading activated!"ESC_NORMAL"\n" );
#endif
   tellMailboxSlot();
   init_irq_table();

   msDelayBig(1500); //wait for wr deamon to read sdbfs

   if( (int)BASE_SYSCON == ERROR_NOT_FOUND )
      mprintf( ESC_ERROR"no SYS_CON found!"ESC_NORMAL"\n" );
   else
      mprintf( "SYS_CON found on adr: 0x%x\n", BASE_SYSCON );

   timer_init(1); //needed by usleep_init()
   usleep_init();

   printCpuId();
   mprintf("g_oneWireBase.pWr is:   0x%08x\n", g_oneWireBase.pWr);
   mprintf("g_oneWireBase.pUser is: 0x%08x\n", g_oneWireBase.pUser);
   mprintf("g_pScub_irq_base is:    0x%08x\n", g_pScub_irq_base);
#ifdef CONFIG_MIL_FG
   mprintf("g_pMil_irq_base is:     0x%08x\n", g_pMil_irq_base);
#endif
   initEcaQueue();

   initAndScan(); // init and scan for fgs

#ifdef CONFIG_SCU_DAQ_INTEGRATION
   scuDaqInitialize( &g_scuDaqAdmin ); // Init and scan for DAQs
   mprintf( "SCU-DAQ initialized\n" );
#endif
   //print_regs();
   while( true )
   {
      check_stack();
      schedule();
   }
}

/*================================== EOF ====================================*/
