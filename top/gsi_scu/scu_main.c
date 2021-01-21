/*!
 *  @file scu_main.c
 *  @brief Main module of SCU function generators in LM32.
 *
 *  @date 10.07.2019
 *
 *  @see https://www-acc.gsi.de/wiki/Hardware/Intern/ScuFgDoc
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
#include "scu_command_handler.h"
#include "scu_fg_macros.h"
#ifdef CONFIG_MIL_FG
 #include "scu_eca_handler.h"
 #include "scu_mil_fg_handler.h"
#endif
#include "scu_fg_handler.h"
#include "scu_temperature.h"
#ifdef CONFIG_SCU_DAQ_INTEGRATION
 #include "daq_main.h"
#endif
#include "lm32signal.h"

#ifdef CONFIG_DBG_MEASURE_IRQ_TIME
TIME_MEASUREMENT_T g_irqTimeMeasurement = TIME_MEASUREMENT_INITIALIZER;
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
STATIC inline ALWAYS_INLINE
void initializeGlobalPointers( void )
{
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
 * @ingroup MAILBOX
 * @brief Tells SAFTLIB the mailbox slot for software interrupts.
 * @see commandHandler
 * @see FG_MB_SLOT saftlib/drivers/fg_regs.h
 * @see FunctionGeneratorFirmware::ScanFgChannels() in
 *      saftlib/drivers/FunctionGeneratorFirmware.cpp
 * @see FunctionGeneratorFirmware::ScanMasterFg() in
 *      saftlib/drivers/FunctionGeneratorFirmware.cpp
 */
STATIC inline ALWAYS_INLINE
void tellMailboxSlot( void )
{
   const int slot = getMsiBoxSlot(0x10); //TODO Where does 0x10 come from?
   if( slot == -1 )
      mprintf( ESC_ERROR"No free slots in MsgBox left!"ESC_NORMAL"\n" );
   else
      mprintf( ESC_FG_MAGENTA
               "Configured slot %d in MsgBox\n"
               ESC_NORMAL , slot );
   g_shared.fg_mb_slot = slot;
}

/*! ---------------------------------------------------------------------------
 * @brief enables MSI generation for the specified channel.
 *
 * Messages from the SCU bus are send to the MSI queue of this CPU with the
 * offset 0x0. \n
 * Messages from the MIL extension are send to the MSI queue of this CPU with
 * the offset 0x20. \n
 * A hardware macro is used, which generates MSIs from legacy interrupts.
 *
 * @param channel number of the channel between 0 and MAX_FG_CHANNELS-1
 * @see disable_slave_irq
 */
void enable_scub_msis( const unsigned int channel )
{
   const unsigned int socket = getSocket( channel );
#ifdef CONFIG_MIL_FG
   if( isAddacFg( socket ) || isMilScuBusFg( socket ) )
   {
#endif
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
#ifdef CONFIG_MIL_FG
   }

   if( !isMilExtentionFg( socket ) )
      return;

   g_pMil_irq_base[8]   = MIL_DRQ;
   g_pMil_irq_base[9]   = MIL_DRQ;
   g_pMil_irq_base[10]  = (uint32_t)pMyMsi + 0x20;
   g_pMil_irq_base[2]   = (1 << MIL_DRQ);
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief delay in multiples of one millisecond
 *  uses the system timer
 *  @param ms delay value in milliseconds
 */
OPTIMIZE( "-O2"  )
STATIC void msDelayBig( const uint64_t ms )
{
   const uint64_t later = getWrSysTime() + ms * 1000000ULL / 8;
   while( getWrSysTime() < later )
   {
      NOP();
   }
}


// #define __MURKS

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Handling of all SCU-bus MSI events.
 */
ONE_TIME_CALL void onScuBusEvent( MSI_T* pMessage )
{
   const unsigned int slot = pMessage->msg + 1;
   const uint16_t pendingIrqs =
      scuBusGetAndResetIterruptPendingFlags((const void*)g_pScub_base, slot );

#ifdef __MURKS
#warning MURKS!
   static unsigned int X = 0;
   X++;
   if( (X % 10000) == 0 )
      return;
#endif

   if( (pendingIrqs & FG1_IRQ) != 0 )
   {
    #ifdef CONFIG_DBG_MEASURE_IRQ_TIME
      timeMeasure( &g_irqTimeMeasurement );
    #endif
      handleAdacFg( slot, FG1_BASE );
   }

   if( (pendingIrqs & FG2_IRQ) != 0 )
      handleAdacFg( slot, FG2_BASE );

#ifdef CONFIG_MIL_FG
   if( (pendingIrqs & DREQ ) != 0 )
      add_msg( &g_aMsg_buf[0], DEVSIO, *pMessage );
#endif
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   if( (pendingIrqs & (1 << DAQ_IRQ_DAQ_FIFO_FULL)) != 0 )
      add_msg( &g_aMsg_buf[0], DAQ, *pMessage );

   //TODO (1 << DAQ_IRQ_HIRES_FINISHED)
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Interrupt callback function for each Message Signaled Interrupt
 * @param intNum Interrupt number.
 * @param pContext Value of second parameter from irqRegisterISR not used
 *                 in this case.
 * @see initInterrupt
 * @see irqRegisterISR
 * @see irq_pop_msi
 * @see dispatch
 */
STATIC void onScuMSInterrupt( const unsigned int intNum,
                              const void* pContext UNUSED )
{
   MSI_T m;
   /*!
    * @todo Use MSI_ITEM_T instead of MSI_T in future!
    */

   while( irqMsiCopyObjectAndRemoveIfActive( (MSI_ITEM_T*)&m, intNum ) )
   {
   #ifndef _CONFIG_NO_DISPATCHER
      #warning with deispatcher...
      add_msg( &g_aMsg_buf[0], IRQ, m );
   #else
      switch( m.adr & 0xFF )
      {
       #ifdef _CONFIG_ADDAC_FG_IN_INTERRUPT
         case ADDR_SCUBUS: onScuBusEvent( &m ); break;
       #else
         case ADDR_SCUBUS: add_msg( &g_aMsg_buf[0], SCUBUS, m ); break; // message from scu bus
       #endif
         case ADDR_SWI:    add_msg( &g_aMsg_buf[0], SWI,    m ); break; // software message from saftlib
       #ifdef CONFIG_MIL_FG
         case ADDR_DEVBUS: add_msg( &g_aMsg_buf[0], DEVBUS, m ); break; // message from dev bus
       #endif
      }
   #endif // ifndef _CONFIG_NO_DISPATCHER
   }
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Installs the interrupt callback function and clears the interrupt
 *        message buffer.
 * @see onScuMSInterrupt
 */
ONE_TIME_CALL void initInterrupt( void )
{
#ifdef _CONFIG_NO_DISPATCHER
 #ifndef _CONFIG_ADDAC_FG_IN_INTERRUPT
   cbReset( &g_aMsg_buf[0], SCUBUS );
 #endif
   cbReset( &g_aMsg_buf[0], SWI );
 #ifdef CONFIG_SCU_DAQ_INTEGRATION
   cbReset( &g_aMsg_buf[0], DAQ );
 #endif
 #ifdef CONFIG_MIL_FG
   cbReset( &g_aMsg_buf[0], DEVSIO );
   cbReset( &g_aMsg_buf[0], DEVBUS );
 #endif
#else
   cbReset( &g_aMsg_buf[0], IRQ );
#endif

   irqRegisterISR( ECA_INTERRUPT_NUMBER, NULL, onScuMSInterrupt );
   irqEnable();
   mprintf( "IRQ table configured: 0b%08b\n", irqGetMaskRegister() );
}

/*! ---------------------------------------------------------------------------
 * @brief initialize procedure at startup
 */
STATIC void initAndScan( void )
{
   /*
    *  No function generator macros assigned to channels at startup!
    */
   for( unsigned int i = 0; i < ARRAY_SIZE(g_shared.fg_regs); i++ )
      g_shared.fg_regs[i].macro_number = SCU_INVALID_VALUE;

   /*
    * Update one wire ID and temperatures.
    */
   updateTemperature();

   /*
    * Scans for SCU-bus slave cards and function generators.
    */
   scanFgs();
}

#define CONFIG_STOP_ON_LM32_EXCEPTION

/*! ---------------------------------------------------------------------------
 * @brief Callback function becomes invoked by LM32 when an exception has
 *        been appeared.
 */
void _onException( const uint32_t sig )
{
   char* str;
   #define _CASE_SIGNAL( S ) case S: str = #S; break;
   switch( sig )
   {
      _CASE_SIGNAL( SIGINT )
      _CASE_SIGNAL( SIGTRAP )
      _CASE_SIGNAL( SIGFPE )
      _CASE_SIGNAL( SIGSEGV )
      default: str = "unknown"; break;
   }
   mprintf( ESC_ERROR "Exception occurred: %d -> %s\n"
#ifdef CONFIG_STOP_ON_LM32_EXCEPTION
                      "System stopped!\n"
#endif
                      ESC_NORMAL, sig, str );
#ifdef CONFIG_STOP_ON_LM32_EXCEPTION
   irqDisable();
   while( true );
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Scans for fgs on mil extension and scu bus.
 */
void scanFgs( void )
{
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
   printFgs();
}

/* task prototypes */
#ifndef __DOXYGEN__
#ifndef _CONFIG_ADDAC_FG_IN_INTERRUPT
STATIC void scu_bus_handler( register TASK_T* pThis FG_UNUSED );
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
   { NULL,               ALWAYS, 0, addacDaqTask   },
#endif
#ifdef CONFIG_MIL_FG
   { &g_aMilTaskData[0], ALWAYS, 0, dev_sio_handler }, // sio task 1
 //!!  { &g_aMilTaskData[1], ALWAYS, 0, dev_sio_handler }, // sio task 2
 //!!  { &g_aMilTaskData[2], ALWAYS, 0, dev_sio_handler }, // sio task 3
 //!!  { &g_aMilTaskData[3], ALWAYS, 0, dev_sio_handler }, // sio task 4
   { &g_aMilTaskData[4], ALWAYS, 0, dev_bus_handler },
#endif
#ifndef _CONFIG_ADDAC_FG_IN_INTERRUPT
   { NULL,               ALWAYS, 0, scu_bus_handler },
#endif
#ifdef CONFIG_MIL_FG
   { NULL,               ALWAYS, 0, ecaHandler      },
#endif
   { NULL,               ALWAYS, 0, commandHandler  }
};

#ifndef _CONFIG_NO_DISPATCHER

/*! ---------------------------------------------------------------------------
 * @brief Move messages to the correct queue, depending on source
 * @see onScuMSInterrupt
 * @see schedule
 * @todo Remove this function and do this in the interrupt handler direct.
 */
ONE_TIME_CALL void dispatch( void )
{
   criticalSectionEnter();
   const MSI_T m = remove_msg( &g_aMsg_buf[0], IRQ );
   criticalSectionExit();
   switch( m.adr & 0xFF )
   {
      case ADDR_SCUBUS: add_msg( &g_aMsg_buf[0], SCUBUS, m ); return; // message from scu bus
      case ADDR_SWI:    add_msg( &g_aMsg_buf[0], SWI,    m ); return; // software message from saftlib
   #ifdef CONFIG_MIL_FG
      case ADDR_DEVBUS: add_msg( &g_aMsg_buf[0], DEVBUS, m ); return; // message from dev bus
   #endif
   }
}
#endif

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
ONE_TIME_CALL void schedule( void )
{

#ifdef __DOXYGEN__
   /*
    * For Doxygen only, making visible in caller graph.
    */
   dev_sio_handler();
   dev_bus_handler();
   scu_bus_handler();
   ecaHandler();
   commandHandler();
   addacDaqTask();
#endif

   const uint64_t tick = getWrSysTime();
   for( unsigned int i = 0; i < ARRAY_SIZE( g_aTasks ); i++ )
   {
#ifndef _CONFIG_NO_DISPATCHER
      dispatch();
#endif
      TASK_T* pCurrent = &g_aTasks[i];
      if( (tick - pCurrent->lasttick) < pCurrent->interval )
      {
         continue;
      }
      pCurrent->func( pCurrent );
      pCurrent->lasttick = tick;
   }
}

#ifndef _CONFIG_ADDAC_FG_IN_INTERRUPT
/*! ---------------------------------------------------------------------------
 * @ingroup TASK INTERRUPT
 * @brief task definition of scu_bus_handler
 * called by the scheduler in the main loop
 * decides which action for a scu bus interrupt is suitable
 * @param pThis pointer to the current task object (not used)
 * @see schedule
 */
STATIC void scu_bus_handler( register TASK_T* pThis FG_UNUSED )
{
   FG_ASSERT( pThis->pTaskData == NULL );

   MSI_T m;
   if( getMessageSave( &m, &g_aMsg_buf[0], SCUBUS ) )
     onScuBusEvent( &m );
}
#endif /* ifndef _CONFIG_ADDAC_FG_IN_INTERRUPT */

/*! ---------------------------------------------------------------------------
 * @brief Helper function for printing the CPU-ID and the number of
 *        MSI endpoints.
 */
ONE_TIME_CALL void printCpuId( void )
{
   unsigned int* cpu_info_base;
   cpu_info_base = (unsigned int*)find_device_adr( GSI, CPU_INFO_ROM );
   if((int)cpu_info_base == ERROR_NOT_FOUND)
   {
      mprintf(ESC_ERROR"no CPU INFO ROM found!"ESC_NORMAL"\n");
      return;
   }
   mprintf("CPU ID: 0x%04X\n", cpu_info_base[0]);
   mprintf("number MSI endpoints: %d\n", cpu_info_base[1]);
}

/*================================ MAIN =====================================*/
void main( void )
{
   mprintf( ESC_BOLD "Start of \"" TO_STRING(TARGET_NAME) "\"\n" ESC_NORMAL
           "Compiler: "COMPILER_VERSION_STRING"\n"
           "Git revision: "TO_STRING(GIT_REVISION)"\n"
           "Found MsgBox at 0x%p. MSI Path is 0x%p\n"
#if defined( CONFIG_MIL_FG ) && defined( CONFIG_READ_MIL_TIME_GAP )
            ESC_WARNING
            "CAUTION! Time gap reading for MIL FGs activated!\n"
            ESC_NORMAL
#endif
           , pCpuMsiBox, pMyMsi );
#ifdef CONFIG_MIL_FG
   dbgPrintMilTaskData();
#endif
   initializeGlobalPointers();
   initInterrupt();
   tellMailboxSlot();

  /*!
   * Wait for wr deamon to read sdbfs
   */
   msDelayBig(1500);

   if( (int)BASE_SYSCON == ERROR_NOT_FOUND )
      mprintf( ESC_ERROR"no SYS_CON found!"ESC_NORMAL"\n" );
   else
      mprintf( "SYS_CON found on addr: 0x%p\n", BASE_SYSCON );

  /*!
   * Will need by usleep_init()
   */
   timer_init(1);
   ATOMIC_SECTION() usleep_init();

   printCpuId();
   mprintf("g_oneWireBase.pWr is:   0x%p\n", g_oneWireBase.pWr);
   mprintf("g_oneWireBase.pUser is: 0x%p\n", g_oneWireBase.pUser);
   mprintf("g_pScub_irq_base is:    0x%p\n", g_pScub_irq_base);
#ifdef CONFIG_MIL_FG
   mprintf("g_pMil_irq_base is:     0x%p\n", g_pMil_irq_base);
   initEcaQueue();
#endif

   hist_init(HISTORY_XYZ_MODULE);
   /*!
    * Scanning and initializing all FG's and DAQ's
    */
   initAndScan();

   //print_regs();
   while( true )
   {
      check_stack();
      schedule();
   }
}

/*================================== EOF ====================================*/
