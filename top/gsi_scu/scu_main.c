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
#include "scu_main.h"
#include "scu_command_handler.h"
#include <scu_fg_list.h>
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

//  #define _CONFIG_NO_INTERRUPT

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

/*===========================================================================*/
/*! ---------------------------------------------------------------------------
 * @see scu_main.h
 */
void die( const char* pErrorMessage )
{
   mprintf( ESC_ERROR "\nERROR: \"%s\"\n+++ LM32 stopped! +++\n" ESC_NORMAL,
            pErrorMessage );
#ifndef CONFIG_REINCERNATE
   while( true );
#else
   mprintf( "...continued...\n" );
#endif
}

#ifdef CONFIG_QUEUE_ALARM
/*
 * Alarm queue containing the pointer of queues in which has been happend
 * a overflow.
 */
QUEUE_CREATE_STATIC( g_queueAlarm, MAX_FG_CHANNELS, SW_QUEUE_T* );

/*! ---------------------------------------------------------------------------
 * @brief Put a message in the given queue object.
 * 
 * If the concerned queue is full, then a alarm-item will put in the 
 * alarm-queue which becomes evaluated in the function queuePollAlarm().
 *
 * @see queuePollAlarm.
 * @param pThis Pointer to the queue object.
 * @param pItem Pointer to the payload object.
 */
void pushInQueue( SW_QUEUE_T* pThis, const void* pItem )
{
   if( queuePush( pThis, pItem ) )
      return;
   queuePush( &g_queueAlarm, &pThis );
}

/*! ----------------------------------------------------------------------------
 * @brief Checks whether a possible overflow has been happen in one or more
 *        of the used message queues. 
 */
STATIC inline void queuePollAlarm( void )
{
   SW_QUEUE_T* pOverflowedQueue;

   if( !queuePopSave( &g_queueAlarm, &pOverflowedQueue ) )
      return;

   const char* str = "unknown";
   #define QEUE2STRING( name ) if( &name == pOverflowedQueue ) str = #name

   QEUE2STRING( g_queueSaftCmd );
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   QEUE2STRING( g_queueAddacDaq );
#endif
#ifdef CONFIG_MIL_FG
   QEUE2STRING( g_queueMilFg );
#endif

   #undef QEUE2STRING

   mprintf( ESC_ERROR
            "ERROR: Queue \"%s\" has overflowed! Capacity: %d\n"
            ESC_NORMAL,
            str, queueGetMaxCapacity( pOverflowedQueue ) );
}
#else
   #define queuePollAlarm()
#endif /* else if CONFIG_QUEUE_ALARM */

/*! ---------------------------------------------------------------------------
 * @brief Initializing of all global pointers accessing the hardware.
 */
STATIC inline ALWAYS_INLINE
void initializeGlobalPointers( void )
{
   initOneWire();

   /*
    * Additional periphery needed for SCU.
    */

   g_pScub_base = (volatile uint16_t*)find_device_adr( GSI, SCU_BUS_MASTER );
   if( (int)g_pScub_base == ERROR_NOT_FOUND )
      die( "SCU-bus not found!" );

   g_pScub_irq_base = (volatile uint32_t*)find_device_adr( GSI, SCU_IRQ_CTRL );
   if( (int)g_pScub_irq_base == ERROR_NOT_FOUND )
      die( "Interrupt control for SCU-bus not found!" );

#ifdef CONFIG_MIL_FG
   g_pScu_mil_base = (unsigned int*)find_device_adr( GSI, SCU_MIL );
   if( (int)g_pScu_mil_base == ERROR_NOT_FOUND )
      die( "MIL-bus not found!" );

   g_pMil_irq_base = (volatile uint32_t*)find_device_adr( GSI, MIL_IRQ_CTRL );
   if( (int)g_pMil_irq_base == ERROR_NOT_FOUND )
      die( "Interrupt control for MIL-bus not found!" );
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup MAILBOX
 * @brief Tells SAFTLIB the mailbox slot for software interrupts.
 * @see commandHandler
 * @see FG_MB_SLOT saftlib/drivers/aRegs.h
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
      die( "No free slots in MsgBox left!" );

   mprintf( ESC_FG_MAGENTA "Configured slot %d in MsgBox\n" ESC_NORMAL , slot );
   g_shared.oSaftLib.oFg.mailBoxSlot = slot;
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

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Handling of all SCU-bus MSI events.
 */
ONE_TIME_CALL void onScuBusEvent( const MSI_ITEM_T* pMessage )
{
   const unsigned int slot = GET_LOWER_HALF( pMessage->msg ) + SCUBUS_START_SLOT;
   uint16_t pendingIrqs;

   while( (pendingIrqs = scuBusGetAndResetIterruptPendingFlags((void*)g_pScub_base, slot )) != 0)
   {
      if( (pendingIrqs & FG1_IRQ) != 0 )
      {
         handleAdacFg( slot, FG1_BASE );
      }

      if( (pendingIrqs & FG2_IRQ) != 0 )
      {
         handleAdacFg( slot, FG2_BASE );
      }

   #ifdef CONFIG_MIL_FG
      if( (pendingIrqs & DREQ ) != 0 )
      {
         const MIL_QEUE_T milMsg =
         { 
            .slot = slot,
            .time = getWrSysTime()
         };

        /*!
         * @see milDeviceHandler
         */
         pushInQueue( &g_queueMilFg, &milMsg );
      }
   #endif

   #ifdef CONFIG_SCU_DAQ_INTEGRATION
      if( (pendingIrqs & (1 << DAQ_IRQ_DAQ_FIFO_FULL)) != 0 )
      {
       #ifndef __DOXYGEN__
         STATIC_ASSERT( sizeof( slot ) == sizeof( DAQ_QUEUE_SLOT_T ) );
       #endif
         pushInQueue( &g_queueAddacDaq, &slot );
      }
   //TODO (1 << DAQ_IRQ_HIRES_FINISHED)
   #endif
   }
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
   MSI_ITEM_T m;

   while( irqMsiCopyObjectAndRemoveIfActive( &m, intNum ) )
   {
      switch( GET_LOWER_HALF( m.adr )  )
      {
         case ADDR_SCUBUS:
         { 
         #if defined( CONFIG_MIL_FG ) && defined( _CONFIG_ECA_BY_MSI )
            if( (m.msg & ECA_VALID_ACTION) != 0 )
            { /*
               * ECA event received
               */
               ecaHandler();
               break;
            }
         #endif
           /*
            * Message from SCU- bus.
            */
            onScuBusEvent( &m );
            break;
         }

         case ADDR_SWI:
         { /*
            * Command message from SAFT-lib
            */

            STATIC_ASSERT( sizeof( m.msg ) == sizeof( SAFT_CMD_T ) );
            pushInQueue( &g_queueSaftCmd, &m.msg );
            break;
         }

     #ifdef CONFIG_MIL_FG
         case ADDR_DEVBUS:
         { /*
            * Message from MIL-bus respectively device-bus.
            */
            const MIL_QEUE_T milMsg =
            {
               .slot = 0,
               .time = getWrSysTime()
            };

           /*!
            * @see milDeviceHandler
            */
            pushInQueue( &g_queueMilFg, &milMsg );
            break;
         }
     #endif /* ifdef CONFIG_MIL_FG */

         default:
         {
            FG_ASSERT( false );
            break;
         }
      }
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
   initCommandHandler();
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   queueReset( &g_queueAddacDaq );
#endif
#ifdef CONFIG_MIL_FG
   queueReset( &g_queueMilFg );
#endif
#ifdef CONFIG_QUEUE_ALARM
   queueReset( &g_queueAlarm );
#endif
#ifndef _CONFIG_NO_INTERRUPT
   irqRegisterISR( ECA_INTERRUPT_NUMBER, NULL, onScuMSInterrupt );
   irqEnable();
   mprintf( "IRQ table configured: 0b%08b\n", irqGetMaskRegister() );
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief initialize procedure at startup
 */
STATIC void initAndScan( void )
{
   /*
    *  No function generator macros assigned to channels at startup!
    */
   for( unsigned int channel = 0; channel < ARRAY_SIZE(g_shared.oSaftLib.oFg.aRegs); channel++ )
      g_shared.oSaftLib.oFg.aRegs[channel].macro_number = SCU_INVALID_VALUE;

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
   fgListFindAll( g_pScub_base,
              #ifdef CONFIG_MIL_FG
                 g_pScu_mil_base,
              #endif
                 g_shared.oSaftLib.oFg.aMacros,
                 &g_shared.oSaftLib.oTemperatures.ext_id );
#if __GNUC__ >= 9
  #pragma GCC diagnostic pop
#endif
   printFgs();
}

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
   { &g_aMilTaskData[0], ALWAYS, 0, milDeviceHandler },
 //!!  { &g_aMilTaskData[1], ALWAYS, 0, milDeviceHandler },
 //!!  { &g_aMilTaskData[2], ALWAYS, 0, milDeviceHandler },
 //!!  { &g_aMilTaskData[3], ALWAYS, 0, milDeviceHandler },
 //!!  { &g_aMilTaskData[4], ALWAYS, 0, milDeviceHandler },
 #ifndef _CONFIG_ECA_BY_MSI
   { NULL,               ALWAYS, 0, ecaHandler      },
 #endif
#endif
   { NULL,               ALWAYS, 0, commandHandler  }
};

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Scheduler for all SCU-tasks defined in g_aTasks. \n
 *        Performing of a cooperative multitasking.
 * @see TASK_T
 * @see g_aTasks
 * @see milDeviceHandler
 * @see ecaHandler
 * @see commandHandler
 * @see addacDaqTask
 */
ONE_TIME_CALL void schedule( void )
{

#ifdef __DOXYGEN__
   /*
    * For Doxygen only, making visible in caller graph.
    */
   milDeviceHandler();
   ecaHandler();
   commandHandler();
   addacDaqTask();
#endif
#ifdef _CONFIG_NO_INTERRUPT
   #warning "Testversion with no interrupts!!!"
   onScuMSInterrupt( ECA_INTERRUPT_NUMBER, NULL );
#endif
   const uint64_t tick = getWrSysTime();
   for( unsigned int i = 0; i < ARRAY_SIZE( g_aTasks ); i++ )
   {
      TASK_T* pCurrent = &g_aTasks[i];
      if( (tick - pCurrent->lasttick) < pCurrent->interval )
      {
         continue;
      }
      pCurrent->func( pCurrent );
      pCurrent->lasttick = tick;
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Helper function for printing the CPU-ID and the number of
 *        MSI endpoints.
 */
ONE_TIME_CALL void printCpuId( void )
{
   unsigned int* cpu_info_base;
   cpu_info_base = (unsigned int*)find_device_adr( GSI, CPU_INFO_ROM );
   if( (int)cpu_info_base == ERROR_NOT_FOUND )
      die( "No CPU INFO ROM found!" );

   mprintf( "CPU ID: 0x%04X\n", cpu_info_base[0] );
   mprintf( "Number MSI endpoints: %d\n", cpu_info_base[1] );
}

/*================================ MAIN =====================================*/
void main( void )
{
   mprintf( ESC_BOLD "\nStart of \"" TO_STRING(TARGET_NAME) "\"\n" ESC_NORMAL
           "Compiler: "COMPILER_VERSION_STRING"\n"
           "Git revision: "TO_STRING(GIT_REVISION)"\n"
           "Resets: %u\n"
           "Found MsgBox at 0x%p. MSI Path is 0x%p\n"
           "Shared memory size: %u bytes\n"
       #if defined( CONFIG_MIL_FG ) && defined( CONFIG_READ_MIL_TIME_GAP )
            ESC_WARNING
            "CAUTION! Time gap reading for MIL FGs activated!\n"
            ESC_NORMAL
       #endif
           , __reset_count,
             pCpuMsiBox,
             pMyMsi,
             sizeof( SCU_SHARED_DATA_T )
          );

#ifdef CONFIG_MIL_FG
   milInitTasks();
   dbgPrintMilTaskData();
#endif
   initializeGlobalPointers();
   tellMailboxSlot();

  /*
   * Wait for wr deamon to read sdbfs
   */
   msDelayBig( 1500 );

   if( (int)BASE_SYSCON == ERROR_NOT_FOUND )
      die( "No SYS_CON found!" );
   mprintf( "SYS_CON found on addr: 0x%p\n", BASE_SYSCON );

  /*!
   * Will need by usleep_init()
   */
   timer_init(1);
   usleep_init();

   printCpuId();
   mprintf( "g_oneWireBase.pWr is:   0x%p\n", g_oneWireBase.pWr );
   mprintf( "g_oneWireBase.pUser is: 0x%p\n", g_oneWireBase.pUser  );
   mprintf( "g_pScub_irq_base is:    0x%p\n", g_pScub_irq_base );
#ifdef CONFIG_MIL_FG
   mprintf( "g_pMil_irq_base is:     0x%p\n", g_pMil_irq_base );
   initEcaQueue();
#endif

   hist_init(HISTORY_XYZ_MODULE);
   /*
    * Scanning and initializing all FG's and DAQ's
    */
   initAndScan();
   //print_regs();
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   mprintf( "ADDAC-DAQ buffer offset:   %u item\n", g_shared.sDaq.ramIndexes.ringIndexes.offset );
   mprintf( "ADDAC-DAQ buffer capacity: %u item\n", g_shared.sDaq.ramIndexes.ringIndexes.capacity );
#endif
#if defined( CONFIG_MIL_FG ) && defined( CONFIG_MIL_DAQ_USE_RAM )
   mprintf( "MIL-DAQ buffer offset:     %u item\n", g_shared.mDaq.indexes.offset );
   mprintf( "MIL-DAQ buffer capacity:   %u item\n", g_shared.mDaq.indexes.capacity );
#endif
   initInterrupt();

#ifdef CONFIG_MIL_DAQ_USE_RAM__
   ramRingAddToWriteIndex( &g_shared.mDaq.indexes, 4 ); //!!!!!
   RAM_RING_INDEX_T r = g_shared.mDaq.indexes.start;
#endif

   while( true )
   {
#if CONFIG_MIL_DAQ_USE_RAM__
      if( r != g_shared.mDaq.indexes.start )
      {
         r = g_shared.mDaq.indexes.start;
         mprintf( "read: %u\nwrite: %u\n", g_shared.mDaq.indexes.start, g_shared.mDaq.indexes.end );
      }
#endif
      if( _endram != STACK_MAGIC )
         die( "Stack overflow!" );
      schedule();
      queuePollAlarm();
   }
}

/*================================== EOF ====================================*/
