/*!
 * @file scu_mil_fg_handler.c
 * @brief Module for handling all MIL function generators and MIL DAQs
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 04.02.2020
 * Outsourced from scu_main.c
 */

#ifndef __DOCFSM__
  #include "scu_fg_macros.h"
  #include "scu_fg_list.h"
  #ifdef CONFIG_MIL_DAQ_USE_RAM
    #include <daq_main.h>
  #endif
  #include <scu_syslog.h>
#endif

#include "scu_mil_fg_handler.h"

#ifdef CONFIG_MIL_DAQ_USE_RAM
extern DAQ_ADMIN_T g_scuDaqAdmin;
#endif

extern volatile uint16_t*     g_pScub_base;
extern volatile unsigned int* g_pScu_mil_base;

#ifdef _CONFIG_VARIABLE_MIL_GAP_READING
   unsigned int g_gapReadingTime = DEFAULT_GAP_READING_INTERVAL;
#endif

/*!
 * @brief Slot-value when no slave selected yet.
 */
#define INVALID_SLAVE_NR ((unsigned int)~0)

#ifdef CONFIG_READ_MIL_TIME_GAP
  typedef struct
  {
     uint64_t         timeInterval;
     MIL_TASK_DATA_T* pTask;
  } MIL_GAP_READ_T;

  MIL_GAP_READ_T mg_aReadGap[ ARRAY_SIZE( g_aMilTaskData[0].aFgChannels ) ];
#endif

QUEUE_CREATE_STATIC( g_queueMilFg,  MAX_FG_CHANNELS, MIL_QEUE_T );

#ifdef CONFIG_MIL_WAIT
   #define INIT_WAITING_TIME .waitingTime      = 0LL,
#else
   #define INIT_WAITING_TIME
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Initializer of a single MIL task
 * @see g_aMilTaskData
 */
#define MIL_TASK_DATA_ITEM_INITIALIZER      \
{                                           \
   .state            = ST_WAIT,             \
   .lastMessage.slot = INVALID_SLAVE_NR,    \
   .lastChannel      = 0,                   \
   .timeoutCounter   = 0,                   \
   INIT_WAITING_TIME                        \
   .aFgChannels =                           \
   {{                                       \
      .irqFlags         = 0,                \
      .setvalue         = 0,                \
      .daqTimestamp     = 0LL               \
   }}                                       \
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Memory space and pre-initializing of MIL-task data.
 */
MIL_TASK_DATA_T g_aMilTaskData[5] =
{
   MIL_TASK_DATA_ITEM_INITIALIZER,
   MIL_TASK_DATA_ITEM_INITIALIZER,
   MIL_TASK_DATA_ITEM_INITIALIZER,
   MIL_TASK_DATA_ITEM_INITIALIZER,
   MIL_TASK_DATA_ITEM_INITIALIZER
};

STATIC_ASSERT( TASKMAX >= (ARRAY_SIZE( g_aMilTaskData ) + MAX_FG_CHANNELS-1 + TASKMIN));

/*
 * Mil-library uses "short" rather than "uint16_t"! :-(
 */
STATIC_ASSERT( sizeof( short ) == sizeof( int16_t ) );

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Prints a error message happened in the device-bus respectively
 *        MIL bus.
 * @param status return status of the MIL-driver module.
 * @param slot Slot-number in the case the mil connection is established via
 *             SCU-Bus
 * @param msg String containing additional message text.
 */
STATIC void milPrintDeviceError( const int status, const int slot, const char* msg )
{
  static const char* pText = ESC_ERROR "dev bus access in slot ";
  char* pMessage;
  #define __MSG_ITEM( status ) case status: pMessage = #status; break
  switch( status )
  {
     __MSG_ITEM( OKAY );
     __MSG_ITEM( TRM_NOT_FREE );
     __MSG_ITEM( RCV_ERROR );
     __MSG_ITEM( RCV_TIMEOUT );
     __MSG_ITEM( RCV_TASK_ERR );
     __MSG_ITEM( RCV_PARITY );
     __MSG_ITEM( ERROR );
     __MSG_ITEM( RCV_TASK_BSY );
     default:
     {
     #ifdef CONFIG_USE_LM32LOG
        lm32Log( LM32_LOG_ERROR, "%s%d failed with code %d, message: %s" ESC_NORMAL "\n",
                                 pText, slot, status, msg);
     #else
        mprintf( "%s%d failed with code %d, message: %s" ESC_NORMAL "\n",
                 pText, slot, status);
     #endif
        return;
     }
  }
  #undef __MSG_ITEM
#ifdef CONFIG_USE_LM32LOG
  lm32Log( LM32_LOG_ERROR, "%s%d failed with message %s, %s" ESC_NORMAL "\n",
                           pText, slot, pMessage, msg);
#else
  mprintf( "%s%d failed with message %s, %s" ESC_NORMAL "\n",
           pText, slot, pMessage, msg);
#endif
}


#define IFA_ID_VAL         0xFA00
#define IFA_MIN_VERSION    0x1900
#define FG_MIN_VERSION     2

/*! ---------------------------------------------------------------------------
 * @see scu_mil_fg_handler.h
 */
unsigned int milGetNumberOfFg( void )
{
   unsigned int numOfMilFg = 0;
   for( unsigned int i = 0; i < ARRAY_SIZE( g_shared.oSaftLib.oFg.aMacros ); i++ )
   {
      if( isMilFg( g_shared.oSaftLib.oFg.aMacros[i].socket ) )
         numOfMilFg++;
   }
   return numOfMilFg;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil_fg_handler.h
 */
void scanScuBusFgsViaMil( volatile uint16_t *scub_adr, FG_MACRO_T* pFgList )
{
   const SCUBUS_SLAVE_FLAGS_T slotFlags =
               scuBusFindSpecificSlaves( (void*)scub_adr, SYS_CSCO, GRP_SIO2 )
             | scuBusFindSpecificSlaves( (void*)scub_adr, SYS_CSCO, GRP_SIO3 );

   if( slotFlags == 0 )
      return;

   /*
    * At least one SIO- slave was found.
    */
   SCU_BUS_FOR_EACH_SLAVE( slot, slotFlags )
   {
   #ifndef _CONFIG_IRQ_ENABLE_IN_START_FG
      scuBusEnableSlaveInterrupt( (void*)scub_adr, slot );
   #endif

      /*
       * MIL-bus adapter respectively SIO- slave has been found in current slot.
       * Proofing whether MIL function generators connected to this adapter.
       */

      /*
       * Resetting all task-slots of this SCU-bus slave.
       */
      scub_reset_mil( scub_adr, slot );

      for( uint32_t ifa_adr = 0; ifa_adr < IFK_MAX_ADR; ifa_adr++ )
      {
         uint16_t ifa_id, ifa_vers, fg_vers;
         STATIC_ASSERT( sizeof( short ) == sizeof( ifa_id ) );
         if( scub_read_mil( scub_adr, slot, (short*)&ifa_id, IFA_ID << 8 | ifa_adr ) != OKAY )
            continue;
         if( ifa_id != IFA_ID_VAL )
            continue;

         STATIC_ASSERT( sizeof( short ) == sizeof( ifa_vers ) );
         if( scub_read_mil( scub_adr, slot, (short*)&ifa_vers, IFA_VERS << 8 | ifa_adr ) != OKAY )
            continue;
         if( ifa_vers < IFA_MIN_VERSION )
            continue;

         STATIC_ASSERT( sizeof( short ) == sizeof( fg_vers ) );
         if( scub_read_mil( scub_adr, slot, (short*)&fg_vers, 0xA6 << 8 | ifa_adr ) != OKAY )
            continue;
         if( (fg_vers < FG_MIN_VERSION) || (fg_vers > 0x00FF) )
            continue;

         /*
          * All three proves has been passed, so it can add it to the FG-list.
          */
         fgListAdd( DEV_SIO | slot, ifa_adr, SYS_CSCO, GRP_IFA8, fg_vers, pFgList );
         //scub_write_mil(scub_adr, slot, 0x100, 0x12 << 8 | ifa_adr); // clear PUR
      }
   } /* SCU_BUS_FOR_EACH_SLAVE( slot, slotFlags ) */
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil_fg_handler.h
 */
void scanExtMilFgs( volatile unsigned int *mil_addr,
                    FG_MACRO_T* pFgList, uint64_t *ext_id )
{
  /*
   * Check only for "ifks", if there is a macro found and a mil extension
   * attached to the baseboard.
   * + mil extension is recognized by a valid 1wire id
   * + mil extension has a 1wire temp sensor with family if 0x42
   */
   if( !(((int)mil_addr != ERROR_NOT_FOUND) && (((int)*ext_id & 0xff) == 0x42)) )
      return;

   /*
    * reset all task-slots by reading value back
    */
   reset_mil( mil_addr );

   /*
    * Probing of all potential MIL-function-generatirs.
    */
   for( uint32_t ifa_adr = 0; ifa_adr < IFK_MAX_ADR; ifa_adr++ )
   {
      uint16_t ifa_id, ifa_vers, fg_vers;

      STATIC_ASSERT( sizeof( short ) == sizeof( ifa_id ) );
      if( read_mil( mil_addr, (short*)&ifa_id, IFA_ID << 8 | ifa_adr ) != OKAY )
         continue;
      if( ifa_id != IFA_ID_VAL )
         continue;

      STATIC_ASSERT( sizeof( short ) == sizeof( ifa_vers ) );
      if( read_mil( mil_addr, (short*)&ifa_vers, IFA_VERS << 8 | ifa_adr ) != OKAY )
         continue;
      if( ifa_vers < IFA_MIN_VERSION )
         continue;

      STATIC_ASSERT( sizeof( short ) == sizeof( fg_vers ) );
      if( read_mil( mil_addr, (short*)&fg_vers, 0xA6 << 8 | ifa_adr ) != OKAY )
         continue;
      if( (fg_vers < FG_MIN_VERSION) || (fg_vers > 0x00FF) )
         continue;

      /*
       * All three proves has been passed, so it can add it to the FG-list.
       */
      fgListAdd( DEV_MIL_EXT, ifa_adr, SYS_CSCO, GRP_IFA8, fg_vers, pFgList );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Initializes the register set for MIL function generator.
 */
STATIC inline void setMilFgRegs( FG_MIL_REGISTER_T* pFgRegs,
                                  const FG_PARAM_SET_T* pPset,
                                  const uint16_t controlReg )
{
   STATIC_ASSERT( sizeof( pFgRegs->coeff_c_low_reg )  * 2 == sizeof( pPset->coeff_c ) );
   STATIC_ASSERT( sizeof( pFgRegs->coeff_c_high_reg ) * 2 == sizeof( pPset->coeff_c ) );

   pFgRegs->cntrl_reg.i16     = controlReg;
   pFgRegs->coeff_a_reg       = pPset->coeff_a;
   pFgRegs->coeff_b_reg       = pPset->coeff_b;
   pFgRegs->shift_reg         = getFgShiftRegValue( pPset );
   pFgRegs->coeff_c_low_reg   = GET_LOWER_HALF( pPset->coeff_c );
   pFgRegs->coeff_c_high_reg  = GET_UPPER_HALF( pPset->coeff_c );
}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 */
inline bool milHandleClearHandlerState( const void* pScuBus,
                                        const void* pMilBus,
                                        const unsigned int socket )
{
   uint16_t dreq_status = 0;
   SCUBUS_SLAVE_FLAGS_T slaveFlags = 0;

   #if !defined( CONFIG_GSI ) && !defined( __DOCFSM__ )
    #warning Maybe old Makefile is used, this could be erroneous in using local static variables!
   #endif
   static SCUBUS_SLAVE_FLAGS_T s_clearIsActive = 0;
   STATIC_ASSERT( BIT_SIZEOF( s_clearIsActive ) >= (MAX_SCU_SLAVES + 1) );

   if( isMilScuBusFg( socket ) )
   {
      const unsigned int slot = getFgSlotNumber( socket );
      scub_status_mil( (volatile unsigned short*) pScuBus, slot, &dreq_status );
      slaveFlags = scuBusGetSlaveFlag( slot );
   }
   else if( isMilExtentionFg( socket ) )
   {
      status_mil( g_pScu_mil_base, &dreq_status );
      /*
       * Setting a flag outside of all existing SCU-bus slots.
       */
      slaveFlags = (1 << MAX_SCU_SLAVES);
   }


   /*
    * If data request (dreq) is active?
    */
   if( (dreq_status & MIL_DATA_REQ_INTR) != 0 )
   {
      if( (s_clearIsActive & slaveFlags) == 0 )
      {
         s_clearIsActive |= slaveFlags;
         fgMilClearHandlerState( socket );
         hist_addx( HISTORY_XYZ_MODULE, __func__, socket );
         return true;
      }
   }
   else
   { /*
      * reset clear
      */
      s_clearIsActive &= ~slaveFlags;
   }

   return false;
}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 */
inline void milFgPrepare( const void* pScuBus,
                          const void* pMilBus,
                          const unsigned int socket,
                          const unsigned int dev )
{
   FG_ASSERT( !isAddacFg( socket ) );

   if( isMilScuBusFg( socket ) )
   {
      const unsigned int slot = getFgSlotNumber( socket );
      FG_ASSERT( slot > 0 );
    #ifdef _CONFIG_IRQ_ENABLE_IN_START_FG
      scuBusEnableSlaveInterrupt( pScuBus, slot );
    #endif
     /*
      * Enable receiving of data request.
      */
      *scuBusGetInterruptEnableFlagRegPtr( pScuBus, slot ) |= DREQ;

     /*
      * Enable sending of data request.
      */
      scub_write_mil( (volatile unsigned short*) pScuBus, slot, 1 << 13, FC_IRQ_MSK | dev );

     /*
      * Set MIL-DAC in FG mode
      */
      scub_write_mil( (volatile unsigned short*) pScuBus, slot, 0x1, FC_IFAMODE_WR | dev );

      return;
   }

   FG_ASSERT( isMilExtentionFg( socket ) );

  /*
   * Enable data request
   */
   write_mil( (volatile unsigned int*) pMilBus, 1 << 13, FC_IRQ_MSK | dev );

   /*
    * Set MIL-DAC in FG mode
    */
   write_mil( (volatile unsigned int*) pMilBus, 0x1, FC_IFAMODE_WR | dev);
}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 */
inline void milFgStart( const void* pScuBus,
                        const void* pMilBus,
                        const FG_PARAM_SET_T* pPset,
                        const unsigned int socket,
                        const unsigned int dev,
                        const unsigned int channel )
{
   FG_ASSERT( !isAddacFg( socket ) );

   const uint16_t cntrl_reg_wr = getFgControlRegValue( pPset, channel );

   FG_MIL_REGISTER_T milFgRegs;
   setMilFgRegs( &milFgRegs, pPset, cntrl_reg_wr );

   /*
    * Save the coeff_c as set-value for MIL-DAQ
    */
   g_aFgChannels[channel].last_c_coeff = pPset->coeff_c;
   #if __GNUC__ >= 9
     #pragma GCC diagnostic push
     #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
   #endif
   if( isMilScuBusFg( socket ) )
   {
      const unsigned int slot = getFgSlotNumber( socket );

      scub_write_mil_blk( (volatile unsigned short*) pScuBus,
                          slot,
                          (short*)&milFgRegs,
                          FC_BLK_WR | dev );

     /*
      * Still in block mode !
      */
      scub_write_mil( (volatile unsigned short*) pScuBus,
                      slot,
                      cntrl_reg_wr, FC_CNTRL_WR | dev);

      scub_write_mil( (volatile unsigned short*) pScuBus,
                      slot,
                      cntrl_reg_wr | FG_ENABLED, FC_CNTRL_WR | dev );

      return;
   }

   FG_ASSERT( isMilExtentionFg( socket ) );

   write_mil_blk( (volatile unsigned int*)pMilBus,
                  (short*)&milFgRegs,
                  FC_BLK_WR | dev );

   /*
    * Still in block mode !
    */
   write_mil( (volatile unsigned int*)pMilBus,
              cntrl_reg_wr,
              FC_CNTRL_WR | dev );

   write_mil( (volatile unsigned int*)pMilBus,
              cntrl_reg_wr | FG_ENABLED, FC_CNTRL_WR | dev );

   #if __GNUC__ >= 9
     #pragma GCC diagnostic pop
   #endif

   return;
}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 */
inline void milFgDisableIrq( const void* pScuBus,
                             const void* pMilBus,
                             const unsigned int socket,
                             const unsigned int dev )
{
   FG_ASSERT( !isAddacFg( socket ) );


   if( isMilScuBusFg( socket ) )
   {
      scub_write_mil( (volatile unsigned short*)pScuBus,
                      getFgSlotNumber( socket ),
                      0x0, FC_IRQ_MSK | dev);
   }
   else
   {
      //write_mil((volatile unsigned int* )pMilBus, 0x0, FC_COEFF_A_WR | dev);  //ack drq
      write_mil( (volatile unsigned int* )pMilBus,
                  0x0, FC_IRQ_MSK | dev);
   }
}

/*! ---------------------------------------------------------------------------
 * scu_fg_macros.h
 */
inline int milFgDisable( const void* pScuBus,
                         const void* pMilBus,
                         unsigned int socket,
                         unsigned int dev )
{
   FG_ASSERT( !isAddacFg( socket ) );

   int status;
   int16_t data;

   if( isMilScuBusFg( socket ) )
   {
      const unsigned int slot = getFgSlotNumber( socket );

      if( (status = scub_read_mil( (volatile unsigned short*) pScuBus, slot,
           &data, FC_CNTRL_RD | dev)) != OKAY )
      {
         milPrintDeviceError( status, slot, __func__ );
         return status;
      }

      scub_write_mil( (volatile unsigned short*) pScuBus, slot,
                       data & ~(0x2), FC_CNTRL_WR | dev);
      return status;
   }

   FG_ASSERT( isMilExtentionFg( socket ) );

   if( (status = read_mil( (volatile unsigned int*)pMilBus, &data,
                           FC_CNTRL_RD | dev)) != OKAY )
   {
      milPrintDeviceError( status, 0, __func__ );
      return status;
   }

   write_mil( (volatile unsigned int*)pMilBus,
               data & ~(0x2),
              FC_CNTRL_WR | dev );

   return status;
}

/*! ----------------------------------------------------------------------------
 * @@brief Initialization of the memora for all MIL-tasks.
 */
void milInitTasks( void )
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_aMilTaskData ); i++ )
   {
      g_aMilTaskData[i].state             = ST_WAIT;
      g_aMilTaskData[i].lastMessage.slot  = INVALID_SLAVE_NR;
      g_aMilTaskData[i].lastMessage.time  = 0LL;
      g_aMilTaskData[i].lastChannel       = 0;
      g_aMilTaskData[i].timeoutCounter    = 0;
   #ifdef CONFIG_MIL_WAIT
      g_aMilTaskData[i].waitingTime       = 0LL;
   #endif
   #ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
      g_aMilTaskData[i].irqDurationTime   = 0LL;
   #endif
      for( unsigned int j = 0; j < ARRAY_SIZE( g_aMilTaskData[0].aFgChannels ); j++ )
      {
         g_aMilTaskData[i].aFgChannels[j].irqFlags     = 0;
         g_aMilTaskData[i].aFgChannels[j].setvalue     = 0;
         g_aMilTaskData[i].aFgChannels[j].daqTimestamp = 0LL;
      }
   }
#ifdef CONFIG_MIL_DAQ_USE_RAM
   ramRingSharedReset( &g_shared.mDaq.memAdmin );
#endif
#ifdef CONFIG_READ_MIL_TIME_GAP
   for( unsigned int i = 0; i < ARRAY_SIZE( mg_aReadGap ); i++ )
   {
      mg_aReadGap[i].timeInterval = 0LL;
      mg_aReadGap[i].pTask        = NULL;
   }
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Converts the states of the FSM in strings.
 * @note For debug purposes only!
 */
STATIC const char* state2string( const FG_STATE_T state )
{
   #define __CASE_RETURN( s ) case s: return #s
   switch( state )
   {
      __CASE_RETURN( ST_WAIT );
   #ifdef CONFIG_MIL_WAIT
      __CASE_RETURN( ST_PREPARE );
   #endif
      __CASE_RETURN( ST_FETCH_STATUS );
      __CASE_RETURN( ST_HANDLE_IRQS );
      //__CASE_RETURN( ST_DATA_AQUISITION );
      __CASE_RETURN( ST_FETCH_DATA );
   }
   return "unknown";
   #undef __CASE_RETURN
}


#ifdef _CONFIG_DBG_MIL_TASK
/*! ---------------------------------------------------------------------------
 * @brief For debug purposes only!
 */
void dbgPrintMilTaskData( void )
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_aMilTaskData ); i++ )
   {
      mprintf( "FSM-state[%u]: %s\n",          i, state2string( g_aMilTaskData[i].state ));
      mprintf( "slave_nr[%u]: 0x%08X\n",       i, g_aMilTaskData[i].slave_nr );
      mprintf( "lastChannel[%u]: %u\n",        i, g_aMilTaskData[i].lastChannel );
      mprintf( "timeoutCounter[%u]: %u\n",   i, g_aMilTaskData[i].timeoutCounter );
      mprintf( "waitingTime[%u]: 0x%08X%08X\n", i, (uint32_t)GET_UPPER_HALF(g_aMilTaskData[i].waitingTime),
                                                  (uint32_t)GET_LOWER_HALF(g_aMilTaskData[i].waitingTime) );
   #ifdef CONFIG_READ_MIL_TIME_GAP
      mprintf( "gapReadingTime[%u]: %08X%08X\n", i, (uint32_t)GET_UPPER_HALF(g_aMilTaskData[i].gapReadingTime),
                                                    (uint32_t)GET_LOWER_HALF(g_aMilTaskData[i].gapReadingTime) );
   #endif
      for( unsigned int j = 0; j < ARRAY_SIZE( g_aMilTaskData[0].aFgChannels ); j++ )
      {
         mprintf( "\tirqFlags[%u][%u]: 0x%04X\n", i, j, g_aMilTaskData[i].aFgChannels[j].irqFlags );
         mprintf( "\tsetvalue[%u][%u]: %u\n", i, j, g_aMilTaskData[i].aFgChannels[j].setvalue );
         mprintf( "\tdaqTimestamp[%u][%u]: 0x%08X%08X\n", i, j,
                   (uint32_t)GET_UPPER_HALF(g_aMilTaskData[i].aFgChannels[j].daqTimestamp),
                   (uint32_t)GET_LOWER_HALF(g_aMilTaskData[i].aFgChannels[j].daqTimestamp) );
      }
   }
}
#endif /* ifdef _CONFIG_DBG_MIL_TASK */


/*! ---------------------------------------------------------------------------
 * @see scu_mil_fg_handler.h
 */
void fgMilClearHandlerState( const unsigned int socket )
{
   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "%s( %u )\n" ESC_NORMAL, __func__, socket );
   if( isMilScuBusFg( socket ) )
   {
      FG_ASSERT( getFgSlotNumber( socket ) > 0 );
      const MIL_QEUE_T milMsg = 
      {
         .slot = getFgSlotNumber( socket ),
         .time = getWrSysTimeSafe()
      };
     /*
      * Triggering of a software pseudo interrupt.
      */
      ATOMIC_SECTION() pushInQueue( &g_queueMilFg, &milMsg );
      return;
   }

   if( isMilExtentionFg( socket ) )
   {
      const MIL_QEUE_T milMsg =
      {
         .slot = 0,
         .time = getWrSysTimeSafe()
      };
     /*
      * Triggering of a software pseudo interrupt.
      */
      ATOMIC_SECTION() pushInQueue( &g_queueMilFg, &milMsg );
   }
}

#if defined( CONFIG_READ_MIL_TIME_GAP ) && !defined(__DOCFSM__)
/*! ---------------------------------------------------------------------------
 * @see scu_mil_fg_handler.h.h
 */
bool isMilFsmInST_WAIT( void )
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_aMilTaskData ); i++ )
   {
      if( g_aMilTaskData[i].state != ST_WAIT )
         return false;
   }
   return true;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil_fg_handler.h.h
 */
void suspendGapReading( void )
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_aMilTaskData ); i++ )
      g_aMilTaskData[i].lastMessage.slot = INVALID_SLAVE_NR;
}
#endif // if defined( CONFIG_READ_MIL_TIME_GAP ) && !defined(__DOCFSM__)

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Returns the task-id-number of the given MIL-task data object.
 * @param pMilTaskData Pointer to the currently MIL-task-data object.
 * @return ID-number in the range of 0 to maximum task objects minus one.
 */
inline STATIC unsigned int getMilTaskId( const MIL_TASK_DATA_T* pMilTaskData )
{
   return (((unsigned int)pMilTaskData) - ((unsigned int)g_aMilTaskData))
                                             / sizeof( MIL_TASK_DATA_T );
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Returns the task number of a mil device.
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/PerfOpt
 * @param pMilTaskData Pointer to the currently running system task.
 * @param channel Channel number
 */
inline STATIC 
unsigned int getMilTaskNumber( const MIL_TASK_DATA_T* pMilTaskData,
                                const unsigned int channel )
{
#ifndef __DOXYGEN__
   STATIC_ASSERT( (TASKMIN + ARRAY_SIZE( g_aMilTaskData ) * ARRAY_SIZE( g_aMilTaskData[0].aFgChannels )) <= TASKMAX );
#endif
   // TODO At the moment that isn't a clean solution it wastes to much task numbers in the task-RAM.
   //      The goal is a maximum of 16 possible task numbers in the range from 1 to 16 per SIO device.
  // return TASKMIN + channel + getMilTaskId( pMilTaskData );
   return TASKMIN + channel + getMilTaskId( pMilTaskData ) * ARRAY_SIZE( g_aMilTaskData[0].aFgChannels );
}


//#define CONFIG_LAGE_TIME_DETECT
#if defined( CONFIG_MIL_DAQ_USE_RAM ) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
  STATIC_ASSERT( sizeof( FG_MACRO_T ) == sizeof(uint32_t) );
  IMPLEMENT_CONVERT_BYTE_ENDIAN( FG_MACRO_T )
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @ingroup TASK
 * @brief Writes the data set coming from one of the MIL-DAQs in the
 *        ring-buffer.
 * @see daq_eb_ram_buffer.hpp
 * @see daq_eb_ram_buffer.cpp
 * @param channel DAQ-channel where the data come from.
 * @param timestamp White-Rabbit time-stamp.
 * @param actValue Actual value.
 * @param setValue Set-value.
 * @param setValueInvalid If true, the set value is invalid.
 * @todo Storing the MIL-DAQ data in the DDR3-RAM instead wasting of
 *       shared memory.
 */
STATIC inline void pushDaqData( FG_MACRO_T fgMacro,
                                const uint64_t timestamp,
                                const uint16_t actValue,
                                const uint32_t setValue
                              #ifdef CONFIG_READ_MIL_TIME_GAP
                                , const bool setValueInvalid
                              #endif
                              )
{
#ifdef CONFIG_LAGE_TIME_DETECT
   static uint64_t lastTime = 0;
   static unsigned int count = 0;
   if( lastTime > 0 )
   {
      if( (timestamp - lastTime) > 100000000ULL )
      {
      #ifdef CONFIG_USE_LM32LOG
         lm32Log( LM32_LOG_WARNING, ESC_WARNING "Time-gap: %d" ESC_NORMAL "\n", count++ );
      #else
         mprintf( ESC_WARNING "Time-gap: %d" ESC_NORMAL "\n", count++ );
      #endif
      }
   }
   lastTime = timestamp;
#endif

#ifdef CONFIG_READ_MIL_TIME_GAP
   if( setValueInvalid )
      fgMacro.outputBits |= SET_VALUE_NOT_VALID_MASK;
#endif

#ifdef CONFIG_MIL_DAQ_USE_RAM
   MIL_DAQ_RAM_ITEM_PAYLOAD_T pl;
 #if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__DOXYGEN__)
   /*
    * In this case the LM32-server and Linux-client doesn't have the same
    * byte order.
    * The Linux library "libetherbone" will made a byte swapping of 32-bit
    * segments during reading out the DDR3-RAM.
    * Therefore the appropriate byte order has to be prepared here.
    * E.g. the 16 bit set- and actual values are reversed, that is correct.
    * See also daq_eb_ram_buffer.hpp and daq_eb_ram_buffer.cpp.
    */
   pl.item.timestamp = MERGE_HIGH_LOW( GET_LOWER_HALF( timestamp ), (uint32_t)GET_UPPER_HALF( timestamp ) );
   pl.item.setValue  = actValue;                   /* yes, it's swapped with setValue */
   pl.item.actValue  = GET_UPPER_HALF( setValue ); /* yes, it's swapped with actValue */
   pl.item.fgMacro   = convertByteEndian_FG_MACRO_T( fgMacro );
 #else
   /*
    * In this case the server and client have the same byte order.
    */
   pl.item.timestamp = timestamp;
   pl.item.setValue  = GET_UPPER_HALF( setValue );
   pl.item.actValue  = actValue;
   pl.item.fgMacro   = fgMacro;
 #endif

   /*
    * A local copy of the read and write indexes will prevent a possible
    * race condition with the Linux client.
    */
   RAM_RING_INDEXES_T indexes = g_shared.mDaq.memAdmin.indexes;

   /*
    * Is the circular buffer full?
    */
   if( ramRingGetRemainingCapacity( &indexes ) < ARRAY_SIZE(pl.ramPayload) )
   { /*
      * Yes, removing the oldest item. 
      * Maybe the Linux-client doesn't run or is to slow.  
      */
      ramRingAddToReadIndex( &indexes, ARRAY_SIZE(pl.ramPayload) );
   }

   /*
    * Writing the prepared data set into the DDR3-RAM organized
    * as circular buffer.
    */
   for( unsigned int i = 0; i < ARRAY_SIZE(pl.ramPayload); i++ )
   {
      ramWriteItem( &g_scuDaqAdmin.oRam, ramRingGetWriteIndex( &indexes ), &pl.ramPayload[i] );
      ramRingIncWriteIndex( &indexes );
   }

   /*
    * Making the modified memory indexes known for the Linux client.
    */
   //TODO Check as its better to actualize the write index only.
   g_shared.mDaq.memAdmin.indexes = indexes;

#else /* ifdef CONFIG_MIL_DAQ_USE_RAM */
   #warning Deprecated: MIL-DAQ data will stored in the LM32 shared memory!
   MIL_DAQ_OBJ_T d;

   d.actvalue = actValue;
   d.tmstmp_l = GET_LOWER_HALF( timestamp );
   d.tmstmp_h = GET_UPPER_HALF( timestamp );
   d.fgMacro  = fgMacro;
   d.setvalue = setValue;
   add_daq_msg( &g_shared.daq_buf, d );
#endif /* else ifdef CONFIG_MIL_DAQ_USE_RAM */
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Helper function printing a timeout message.
 */
STATIC void printTimeoutMessage( register MIL_TASK_DATA_T* pMilTaskData )
{
#ifdef CONFIG_USE_LM32LOG
   lm32Log( LM32_LOG_ERROR, ESC_ERROR
            "ERROR: Timeout %s: state %s, taskid %d index %d" ESC_NORMAL"\n",
            (pMilTaskData->lastMessage.slot != 0)? "dev_sio_handler" : "dev_bus_handler",
            state2string( pMilTaskData->state ),
            getMilTaskId( pMilTaskData ),
            pMilTaskData->lastChannel );
#else
   mprintf( ESC_ERROR "ERROR: Timeout %s: state %s, taskid %d index %d" ESC_NORMAL"\n",
            (pMilTaskData->lastMessage.slot != 0)? "dev_sio_handler" : "dev_bus_handler",
            state2string( pMilTaskData->state ),
            getMilTaskId( pMilTaskData ),
            pMilTaskData->lastChannel );
#endif
}

/*
 * A little bit of paranoia doesn't hurt too much. ;-)
 */
STATIC_ASSERT( MAX_FG_CHANNELS == ARRAY_SIZE( g_aMilTaskData[0].aFgChannels ) );
STATIC_ASSERT( MAX_FG_CHANNELS == ARRAY_SIZE( g_aFgChannels ));

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief function returns true when no interrupt of the given channel
 *        is pending.
 *
 * Helper-function for function milDeviceHandler
 * @param pMilTaskData pointer to the current MIL-task-data object
 * @param channel channel number of function generator
 * @retval true No interrupt pending
 * @retval false Any interrupt pending
 * @see milDeviceHandler
 * @see milReqestStatus milGetStatus
 */
ALWAYS_INLINE STATIC inline
bool isNoIrqPending( register const MIL_TASK_DATA_T* pMilTaskData,
                                                 const unsigned int channel )
{
   return
   (pMilTaskData->aFgChannels[channel].irqFlags & (DEV_STATE_IRQ | DEV_DRQ)) == 0;
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Requests the current status of the MIL device.
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/PerfOpt
 * @param pMilTaskData pointer to the current MIL-task-data object
 * @param channel channel number of function generator
 * @return MIL-device status
 * @see milDeviceHandler
 */
STATIC inline
int milReqestStatus( register MIL_TASK_DATA_T* pMilTaskData,
                     const unsigned int channel )
{
   FG_ASSERT( pMilTaskData->lastMessage.slot != INVALID_SLAVE_NR );
   const unsigned int socket     = getSocket( channel );
   const unsigned int devAndMode = getDevice( channel ) | FC_IRQ_ACT_RD;
   const unsigned int milTaskNo  = getMilTaskNumber( pMilTaskData, channel );

   /*
    * Is trades as a SIO device? 
    */
   if( pMilTaskData->lastMessage.slot != 0 )
   {
      if( getFgSlotNumber( socket ) != pMilTaskData->lastMessage.slot )
         return OKAY;

      if( !isMilScuBusFg( socket ) )
         return OKAY;

      return scub_set_task_mil( g_pScub_base, pMilTaskData->lastMessage.slot,
                                                    milTaskNo, devAndMode );
   }

   if( !isMilExtentionFg( socket ) )
       return OKAY;

   return set_task_mil( g_pScu_mil_base, milTaskNo, devAndMode );
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Reads the currently status of the MIL device back.
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/PerfOpt
 * @param pMilTaskData pointer to the current MIL-task-data object
 * @param channel channel number of function generator
 * @return MIL-device status
 * @see milDeviceHandler
 */
STATIC inline
int milGetStatus( register MIL_TASK_DATA_T* pMilTaskData,
                                                  const unsigned int channel )
{
   FG_ASSERT( pMilTaskData->lastMessage.slot != INVALID_SLAVE_NR );

   const unsigned int  socket = getSocket( channel );
   const unsigned char milTaskNo = getMilTaskNumber( pMilTaskData, channel );

   STATIC_ASSERT( sizeof(short) == sizeof( pMilTaskData->aFgChannels[0].irqFlags ) );

   short* const pIrqFlags = &pMilTaskData->aFgChannels[channel].irqFlags;

   /*
    * Reset old IRQ-flags
    */
   *pIrqFlags = 0;

   /*
    * Is trades as a SIO device? 
    */
   if( pMilTaskData->lastMessage.slot != 0 )
   {
      if( getFgSlotNumber( socket ) != pMilTaskData->lastMessage.slot )
         return OKAY;
      if( !isMilScuBusFg( socket ) )
         return OKAY;

      return scub_get_task_mil( g_pScub_base, pMilTaskData->lastMessage.slot,
                                milTaskNo, pIrqFlags );
   }

   if( !isMilExtentionFg( socket ) )
      return OKAY;

   return get_task_mil( g_pScu_mil_base, milTaskNo, pIrqFlags );
}

/*! ---------------------------------------------------------------------------
 * @brief Supplies the by "devNum" and "socket" addressed MIL function
 *        generator with new polynomial data.
 */
STATIC inline void feedMilFg( const unsigned int socket,
                              const unsigned int devNum,
                              const FG_CTRL_RG_T cntrl_reg,
                              signed int* pSetvalue )
{
   const unsigned int channel = cntrl_reg.bv.number;

   FG_ASSERT( channel < ARRAY_SIZE( g_aFgChannels ) );

   FG_PARAM_SET_T pset;
   /*
    * Reading circular buffer with new FG-data.
    */
   if( !cbRead( &g_shared.oSaftLib.oFg.aChannelBuffers[0],
                &g_shared.oSaftLib.oFg.aRegs[0], channel, &pset ) )
   {
      hist_addx(HISTORY_XYZ_MODULE, "MIL-FG buffer empty, no parameter sent", socket);
      lm32Log( LM32_LOG_ERROR, ESC_ERROR
              "ERROR: Buffer of fg-%u-%u is empty! Channel: %u\n"
              ESC_NORMAL, socket, devNum, channel );
      return;
   }

   /*
    * Setting the set value of MIL-DAQ
    */
   *pSetvalue = pset.coeff_c;

   FG_MIL_REGISTER_T milFgRegs;
   /*
    * clear freq, step select, fg_running and fg_enabled
    */
  // setMilFgRegs( &milFgRegs, &pset, (cntrl_reg.i16 & ~(0xfc07)) |
  //                                  (pset.control.i32 & (PSET_STEP | PSET_FREQU)) << 10) ;
   setMilFgRegs( &milFgRegs, &pset, (cntrl_reg.i16 & FG_NUMBER) |
                                    ((pset.control.i32 & (PSET_STEP | PSET_FREQU)) << 10) );
 #if __GNUC__ >= 9
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
 #endif
   if( isMilExtentionFg( socket ) )
   { /*
      * Send FG-data via MIL-extention adapter.
      */
       write_mil_blk( g_pScu_mil_base, (short*)&milFgRegs,
                              FC_BLK_WR | devNum );
   }
   else
   { /*
      * Send FG-data via SCU-bus-slave MIL adapter "SIO"
      */
      scub_write_mil_blk( g_pScub_base, getFgSlotNumber( socket ),
                                   (short*)&milFgRegs, FC_BLK_WR | devNum );
   }
 #if __GNUC__ >= 9
   #pragma GCC diagnostic pop
 #endif

#ifdef CONFIG_USE_FG_MSI_TIMEOUT
   wdtReset( channel );
#endif
#ifdef CONFIG_USE_SENT_COUNTER
   g_aFgChannels[channel].param_sent++;
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Handling of a MIL function generator and send the next polynomial
 *        date set.
 * @see handleAdacFg
 */
STATIC inline
void handleMilFg( const unsigned int socket,
                  const unsigned int devNum,
                  const uint16_t irqFlags,
                  signed int* pSetvalue )
{
   FG_ASSERT( !isAddacFg( socket ) );

   const FG_CTRL_RG_T ctrlReg = { .i16 = irqFlags };
   const unsigned int channel = ctrlReg.bv.number;

   //mprintf( "%02b\n", irqFlags );
   FG_ASSERT( ctrlReg.bv.devStateIrq || ctrlReg.bv.devDrq );
  // FG_ASSERT( ctrlReg.bv.devStateIrq == ctrlReg.bv.devDrq );

   if( channel >= ARRAY_SIZE( g_shared.oSaftLib.oFg.aRegs ) )
   {
   #ifdef CONFIG_USE_LM32LOG
      lm32Log( LM32_LOG_ERROR, ESC_ERROR
               "ERROR: %s: Channel of MIL-FG out of range: %d\n" ESC_NORMAL,
               __func__, channel );
   #else
      mprintf( ESC_ERROR 
               "ERROR: %s: Channel of MIL-FG out of range: %d\n" ESC_NORMAL,
               __func__, channel );
   #endif
      return;
   }

   if( !ctrlReg.bv.isRunning )
   { /*
      * Function generator has stopped.
      * Sending a appropriate stop-message including the reason
      * to the SAFT-lib.
      */
      makeStop( channel );
      return;
   }

   /*
    * The hardware of the MIL function generators doesn't have a ramp-counter
    * integrated, so this task will made by the software here.
    */
   g_shared.oSaftLib.oFg.aRegs[channel].ramp_count++;

   if( !ctrlReg.bv.devDrq )
   { /*
      * The concerned function generator has received the
      * timing- tag or the broadcast message.
      * Sending a start-message to the SAFT-lib.
      */
     // mprintf( "*\n" );
      makeStart( channel );
   }

   /*
    * Send a refill-message to the SAFT-lib if
    * the buffer has reached a critical level.
    */
   sendRefillSignalIfThreshold( channel );

   /*
    * Send next polynomial data via MIL-bus to function generator
    * and fetches the C- coefficient which will used as set-data
    * of the MIL-DAQ.
    */
   feedMilFg( socket, devNum, ctrlReg, pSetvalue );
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Writes data to the MIL function generator
 * @param pMilTaskData pointer to the current MIL-task-data object
 * @param channel channel number of function generator
 * @return MIL-device status
 * @see milDeviceHandler
 */
STATIC inline
void milHandleAndWrite( register MIL_TASK_DATA_T* pMilTaskData,
                       const unsigned int channel )
{
   FG_ASSERT( pMilTaskData->lastMessage.slot != INVALID_SLAVE_NR );

   const unsigned int dev = getDevice( channel );

   /*
    * Writes the next polynomial data set to the concerning
    * function generator.
    */
   handleMilFg( getSocket( channel ),
                dev,
                pMilTaskData->aFgChannels[channel].irqFlags,
                &(pMilTaskData->aFgChannels[channel].setvalue) );

  // mprintf( "%02b\n", pMilTaskData->aFgChannels[channel].irqFlags );
   /*
    * Clear IRQ pending and end block transfer.
    */
#if 1
   if( pMilTaskData->lastMessage.slot != 0 )
   {
      scub_write_mil( g_pScub_base, pMilTaskData->lastMessage.slot,
                                    0,  dev | FC_IRQ_ACT_WR );
      return;
   }
   write_mil( g_pScu_mil_base, 0, dev | FC_IRQ_ACT_WR );
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Set the read task of the MIL device
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/PerfOpt
 * @param pMilTaskData pointer to the current MIL-task-data object
 * @param channel channel number of function generator
 * @return MIL-device status
 * @see milDeviceHandler
 */
STATIC inline
int milSetTask( register MIL_TASK_DATA_T* pMilTaskData,
                const unsigned int channel )
{
   FG_ASSERT( pMilTaskData->lastMessage.slot != INVALID_SLAVE_NR );
   const unsigned int  devAndMode = getDevice( channel ) | FC_ACT_RD;
   const unsigned char milTaskNo  = getMilTaskNumber( pMilTaskData, channel );
   if( pMilTaskData->lastMessage.slot != 0 )
   {
      return scub_set_task_mil( g_pScub_base, pMilTaskData->lastMessage.slot,
                                                      milTaskNo, devAndMode );
   }
   return set_task_mil( g_pScu_mil_base, milTaskNo, devAndMode );
}

#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof(short) == sizeof(int16_t) );
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Reads the actual ADC value from MIL-device
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/PerfOpt
 * @param pMilTaskData pointer to the current MIL-task-data object
 * @param channel channel number of function generator
 * @param pActAdcValue Pointer in which shall the ADC-value copied
 * @return MIL-device status
 * @see milDeviceHandler
 */
STATIC inline
int milGetTask( register MIL_TASK_DATA_T* pMilTaskData,
                const unsigned int channel, int16_t* pActAdcValue )
{
   FG_ASSERT( pMilTaskData->lastMessage.slot != INVALID_SLAVE_NR );
   const unsigned char milTaskNo = getMilTaskNumber( pMilTaskData, channel );
   if( pMilTaskData->lastMessage.slot != 0 )
   {
      return scub_get_task_mil( g_pScub_base, pMilTaskData->lastMessage.slot,
                                                   milTaskNo, pActAdcValue );
   }
   return get_task_mil( g_pScu_mil_base, milTaskNo, pActAdcValue );
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Macro performs a FSM transition. \n
 *        Helper macro for documenting the FSM by the FSM-visualizer DOCFSM.
 * @see milDeviceHandler
 * @see https://github.com/UlrichBecker/DocFsm
 */
#define FSM_TRANSITION( newState, attr... ) \
{                                           \
   pMilData->state = newState;              \
   break;                                   \
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Macro performs a FSM transition and performers the next state
 *        immediately. \n
 *        Helper macro for documenting the FSM by the FSM-visualizer DOCFSM.
 * @see milDeviceHandler
 * @see https://github.com/UlrichBecker/DocFsm
 */
#define FSM_TRANSITION_NEXT( newState, attr... ) \
{                                                \
   pMilData->state = newState;                   \
   next = true;                                  \
   break;                                        \
}

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Macro documenting a FSM transition to the same state. \n
 *        Helper dummy macro for documenting the FSM by the FSM-visualizer
 *        DOCFSM.
 * @see milDeviceHandler
 * @see https://github.com/UlrichBecker/DocFsm
 */
#define FSM_TRANSITION_SELF( attr... ) break;

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Initializer for Finite-State-Machines. \n
 *        Helper macro for documenting the FSM by the FSM-visualizer DOCFSM.
 * @see milDeviceHandler
 * @see https://github.com/UlrichBecker/DocFsm
 */
#define FSM_INIT_FSM( init, attr... )       pMilData->state = init

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Loop control macro for all present function generator channels
 *        started form given channel in parameter "start"
 *
 * Helper-macro for function milDeviceHandler
 *
 * It works off the FG-list initialized by function scan_all_fgs() in file
 * scu_function_generator.c from the in parameter start given channel number.
 *
 * @see isFgPresent
 * @see milDeviceHandler
 * @param channel current channel number, shall be of type unsigned int.
 * @param start Channel number to start.
 */
#define FOR_EACH_FG_CONTINUING( channel, start ) \
   for( channel = start; isFgPresent( channel ); channel++ )

/*! ---------------------------------------------------------------------------
 * @ingroup MIL_FSM
 * @brief Loop control macro for all present function generator channels.
 *
 * It works off the entire FG-list initialized by function scan_all_fgs() in
 * file scu_function_generator.c
 *
 * Helper-macro for function milDeviceHandler
 *
 * @see isFgPresent
 * @see milDeviceHandler
 * @param channel current channel number, shall be of type unsigned int.
 */
#define FOR_EACH_FG( channel ) FOR_EACH_FG_CONTINUING( channel, 0 )


//#define IRQ_WAITING_TIME INTERVAL_100US
#define IRQ_WAITING_TIME 200000ULL

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @ingroup MIL_FSM
 * @brief Wrapper for read queue function, depends on using timer-interrupt
 *        or FreeRTOS or neither nor...
 * @param  pMilData Pointer to the current task-data.
 * @retval false No data in queue.
 * @retval true New data has been copied in &pMilData->lastMessage
 */
ALWAYS_INLINE STATIC inline
bool milQueuePop( MIL_TASK_DATA_T* pMilData  )
{
#ifdef CONFIG_RTOS
   return (xQueueReceive( g_queueMilFg, &pMilData->lastMessage, 0 ) == pdPASS);
#elif  CONFIG_MIL_IN_TIMER_INTERRUPT
   return queuePop( &g_queueMilFg, &pMilData->lastMessage );
#else
   return queuePopSave( &g_queueMilFg, &pMilData->lastMessage );
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @ingroup MIL_FSM
 * @brief FSM of a single MIL-task.
 * @see g_aMilTaskData
 * @see https://github.com/UlrichBecker/DocFsm
 * @dotfile scu_mil_fg_handler.gv State graph for this function
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/ScuSio
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/PerfOpt
 *
 *
 * @todo When gap-reading is activated (compiler switch CONFIG_READ_MIL_TIME_GAP
 *       is defined) so the danger of jittering could be exist! \n
 *       Solution proposal: Linux-host resp. SAFTLIB shall send a
 *       "function-generator-announcement-signal", before the function generator
 *       issued a new analog signal.
 *
 * @param pMilData Pointer to a single element of array: g_aMilTaskData
 */
STATIC inline ALWAYS_INLINE void milTask( MIL_TASK_DATA_T* pMilData  )
{
   bool next;
   do
   {  /*!
       * @brief Flag becomes true within the macro FSM_TRANSITION_SELF().
       */
      next = false;

      /*!
       * @brief Holds the actual state of the FSM.
       */
      const FG_STATE_T lastState = pMilData->state;

     /*
      * Performing the FSM state-do activities.
      */
      switch( lastState )
      {
         case ST_WAIT:
         { /*
            * Did the interrupt handler put a message in the queue?
            */
            if( milQueuePop( pMilData ) )
            { /*
               * Yes, the message has been moved from the queue.
               */
            #ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
               pMilData->irqDurationTime = irqGetTimeSinceLastInterrupt();
            #endif
            #ifdef CONFIG_MIL_WAIT
               FSM_TRANSITION( ST_PREPARE, label='Message received', color=green );
            #else
               FSM_TRANSITION_NEXT( ST_FETCH_STATUS, label='Message received', color=green );
            #endif
            }
         #ifdef CONFIG_READ_MIL_TIME_GAP
           /*
            * Only a task which has already served a function generator
            * can read a time-gap. That means its slave number has to be valid.
            */
            if(
              #ifdef _CONFIG_VARIABLE_MIL_GAP_READING
                ( g_gapReadingTime != 0 ) &&
              #endif
                ( pMilData->lastMessage.slot != INVALID_SLAVE_NR )
              )
            {
               const uint64_t time = getWrSysTimeSafe();
               bool isInGap = false;
               unsigned int channel;
               FOR_EACH_FG( channel )
               {
                  if( !fgIsStarted( channel ) )
                     continue;
                  if( mg_aReadGap[channel].pTask != NULL )
                     continue;
                  if( mg_aReadGap[channel].timeInterval == 0LL )
                     continue;
                  if( mg_aReadGap[channel].timeInterval > time )
                     continue;

                  mg_aReadGap[channel].pTask = pMilData;
                  isInGap = true;
               }
               if( isInGap )
               {
                 // FSM_TRANSITION( ST_DATA_AQUISITION, label='Gap reading time\nexpired',
                 //                                  color=magenta );
                 FSM_TRANSITION( ST_FETCH_DATA, label='Gap reading time\nexpired',
                                                color=magenta );
               }
            }
         #endif /* ifdef CONFIG_READ_MIL_TIME_GAP */
            FSM_TRANSITION_SELF( label='No message', color=blue );
         } /* end case ST_WAIT */

      #ifdef CONFIG_MIL_WAIT
         case ST_PREPARE:
         { /*
            * wait for IRQ_WAITING_TIME
            */
            if( getWrSysTimeSafe() < pMilData->waitingTime )
               FSM_TRANSITION_SELF( label='IRQ_WAITING_TIME not expired', color=blue );
            #ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
             //  pMilData->irqDurationTime = irqGetTimeSinceLastInterrupt();
            #endif
            FSM_TRANSITION( ST_FETCH_STATUS, color=green );
         }
      #endif

         case ST_FETCH_STATUS:
         { /*
            * if timeout reached, proceed with next task
            */
            if( pMilData->timeoutCounter > TASK_TIMEOUT )
            {
               printTimeoutMessage( pMilData );
            #ifdef CONFIG_GOTO_STWAIT_WHEN_TIMEOUT
               FSM_TRANSITION( ST_WAIT, label='maximum timeout-count\nreached',
                                        color=red );
            #else
               /*
                * skipping the faulty channel
                */
               pMilData->lastChannel++;
               pMilData->timeoutCounter = 0;
            #endif
            }
            /*
             * fetch status from dev bus controller;
             */
            int status = OKAY;
            unsigned int channel;
            FOR_EACH_FG_CONTINUING( channel, pMilData->lastChannel )
            { /*
               * Reset old IRQ-flags
               */
               pMilData->aFgChannels[channel].irqFlags = 0;

               if( fgIsStopped( channel ) )
                  continue;

               status = milGetStatus( pMilData, channel );
               if( status == RCV_TASK_BSY )
                  break; /* break from FOR_EACH_FG_CONTINUING loop */
               if( status != OKAY )
                  milPrintDeviceError( status, pMilData->lastMessage.slot, state2string( lastState ) );
            }
            if( status == RCV_TASK_BSY )
            { /*
               * Start next time from this channel.
               */
               pMilData->lastChannel = channel;
               pMilData->timeoutCounter++;
               FSM_TRANSITION_SELF( label='Receiving busy', color=blue );
            }
            FSM_TRANSITION_NEXT( ST_HANDLE_IRQS, color=green );
         } /* end case ST_FETCH_STATUS*/

         case ST_HANDLE_IRQS:
         { /*
            * handle irqs for ifas with active pending regs; non blocking write
            */
            unsigned int active = 0;
            unsigned int channel;
            FOR_EACH_FG( channel )
            {
               if( isNoIrqPending( pMilData, channel ) )
               { /*
                  * Handle next channel...
                  */
                  continue;
               }
               TRACE_MIL_DRQ( "4 %d\n", channel );
               /*
                * Writing the next polynomial data set to the concerning function
                * generator in non blocking mode.
                */
               milHandleAndWrite( pMilData, channel );
               active++;
            }
            if( active == 0 )
            {
               FSM_TRANSITION( ST_WAIT, color=blue, label='no IRQ pending' );
            }
            if( channel == 0 )
               milPrintDeviceError( OKAY, pMilData->lastMessage.slot, "No interrupt pending!" );
            //FSM_TRANSITION( ST_DATA_AQUISITION, color=green );
            FSM_TRANSITION_NEXT( ST_FETCH_DATA, color=green );
         } /* end case ST_HANDLE_IRQS */
   #if 0
         case ST_DATA_AQUISITION:
         {
            FSM_TRANSITION( ST_FETCH_DATA, color=green );
         } /* end case ST_DATA_AQUISITION */
   #endif
         case ST_FETCH_DATA:
         { /*
            * if timeout reached, proceed with next task
            */
            if( pMilData->timeoutCounter > TASK_TIMEOUT )
            {
               printTimeoutMessage( pMilData );
            #ifdef CONFIG_GOTO_STWAIT_WHEN_TIMEOUT
               FSM_TRANSITION( ST_WAIT, label='maximum timeout-count\nreached',
                                        color=blue );
            #else
               /*
                * skipping the faulty channel
                */
               pMilData->lastChannel++;
               pMilData->timeoutCounter = 0;
            #endif
            }
           /*
            * fetch DAQ data
            */
            int status = OKAY;
            unsigned int channel;
            FOR_EACH_FG_CONTINUING( channel, pMilData->lastChannel )
            {
               if( isNoIrqPending( pMilData, channel ) )
               { /*
                  * Handle next channel...
                  */
                  continue;
               }
               int16_t actAdcValue;
               status = milGetTask( pMilData, channel, &actAdcValue );
               if( status == RCV_TASK_BSY )
                  break; /* break from FOR_EACH_FG_CONTINUING loop */

               if( status != OKAY )
               {
                  milPrintDeviceError( status, pMilData->lastMessage.slot, state2string( lastState ) );
                 /*
                  * Handle next channel...
                  */
                  continue;
               }
               pushDaqData( getFgMacroViaFgRegister( channel ),
                            pMilData->aFgChannels[channel].daqTimestamp,
                            actAdcValue,
                            g_aFgChannels[channel].last_c_coeff
                          #ifdef CONFIG_READ_MIL_TIME_GAP
                           , mg_aReadGap[channel].pTask == pMilData
                          #endif
                          );
               /*
                * save the setvalue from the tuple sent for the next drq handling
                */
               g_aFgChannels[channel].last_c_coeff = pMilData->aFgChannels[channel].setvalue;
            } /* end FOR_EACH_FG_CONTINUING */

            if( status == RCV_TASK_BSY )
            { /*
               * Start next time from channel
               */
               pMilData->lastChannel = channel;
               pMilData->timeoutCounter++;
               FSM_TRANSITION_SELF( label='Receiving busy', color=blue );
            }
            FSM_TRANSITION( ST_WAIT, color=green );
         } /* end case ST_FETCH_DATA */

         default: /* Should never be reached! */
         {
         #ifdef CONFIG_USE_LM32LOG
            lm32Log( LM32_LOG_ERROR, ESC_ERROR
                                     "ERROR: Unknown FSM-state of %s(): %d !\n" ESC_NORMAL,
                                     __func__, pMilData->state );

         #else
            mprintf( ESC_ERROR "ERROR: Unknown FSM-state of %s(): %d !\n" ESC_NORMAL,
                     __func__, pMilData->state );
         #endif
            FSM_INIT_FSM( ST_WAIT, label='Initializing', color=blue );
            break;
         }
      } /* End of state-do activities */

      /*
       * Has the FSM-state changed?
       */
      if( lastState == pMilData->state )
      { /*
         * No, there is nothing more to do, leave this function.
         */
         return;
      }

      /*
       *    *** The FSM-state has changed! ***
       *
       * Performing FSM-state-transition activities if necessary,
       * respectively here the state-entry activities.
       */
      switch( pMilData->state )
      {
      #ifdef CONFIG_READ_MIL_TIME_GAP
         case ST_WAIT:
         {
         #ifdef _CONFIG_VARIABLE_MIL_GAP_READING
            if( g_gapReadingTime == 0 )
               break;
         #endif
            unsigned int channel;
            FOR_EACH_FG( channel )
            {
               if( (mg_aReadGap[channel].pTask != pMilData) && isNoIrqPending( pMilData, channel ))
                  continue;

               mg_aReadGap[channel].pTask = NULL;
               mg_aReadGap[channel].timeInterval = pMilData->aFgChannels[channel].daqTimestamp +
            #ifdef _CONFIG_VARIABLE_MIL_GAP_READING
               INTERVAL_1MS * g_gapReadingTime;
            #else
               INTERVAL_10MS;
            #endif
            }
            break;
         }
      #endif /* ifdef CONFIG_READ_MIL_TIME_GAP */

      #ifdef CONFIG_MIL_WAIT
         case ST_PREPARE:
         {
            pMilData->waitingTime = getWrSysTimeSafe() + IRQ_WAITING_TIME;
             //pMilData->waitingTime = pMilData->lastMessage.time + IRQ_WAITING_TIME;
            break;
         }
   #endif

         case ST_FETCH_STATUS:
         { /*
            * Requesting of all IRQ-pending registers.
            */
            int status = OKAY;
            unsigned int channel;
            FOR_EACH_FG( channel )
            {
               if( fgIsStopped( channel ) )
                  continue;

               status = milReqestStatus( pMilData, channel );
               if( status != OKAY )
                  milPrintDeviceError( status, pMilData->lastMessage.slot, state2string( pMilData->state ) );
            }
           /*
            * start next time from channel 0
            */
            pMilData->lastChannel = 0;
            pMilData->timeoutCounter = 0;
            break;
         }

         case ST_FETCH_DATA:
         {
            unsigned int channel;
            FOR_EACH_FG( channel )
            {
               if( isNoIrqPending( pMilData, channel ) )
               { /*
                  * Handle next channel...
                  */
                  continue;
               }
               /*
                * Store the sample timestamp of DAQ.
                */
            #ifdef CONFIG_READ_MIL_TIME_GAP
               if( mg_aReadGap[channel].pTask == pMilData )
                  pMilData->aFgChannels[channel].daqTimestamp = getWrSysTimeSafe();
               else
            #endif
                  pMilData->aFgChannels[channel].daqTimestamp = pMilData->lastMessage.time;

               const int status = milSetTask( pMilData, channel );
               if( status != OKAY )
                  milPrintDeviceError( status, pMilData->lastMessage.slot, state2string( pMilData->state ) );
            }
           /*
            * start next time from channel 0
            */
            pMilData->lastChannel = 0;
            pMilData->timeoutCounter = 0;
            break;
         }

         default: break;
      } /* End of state entry activities */
   }
   while( next );
} /* End function milTask */


/*-----------------------------------------------------------------------------
 * @see scu_mil_fg_handler.h
 */
void milExecuteTasks( void )
{
#ifdef CONFIG_MIL_DAQ_USE_RAM
   /*
    * Removing old data which has been possibly read and evaluated by the
    * Linux client
    * NOTE: This has to be made in any cases here independently whether one or more
    *       MIL FG are active or not.
    *       Because only in this way it becomes possible to continuing the
    *       handshake transfer at reading the possible remaining data from
    *       the DDR3 memory by the Linux client.
    * See daq_base_interface.cpp  function: DaqBaseInterface::getNumberOfNewData
    * See daq_base_interface.cpp  function: DaqBaseInterface::sendWasRead
    * See mdaq_administration.cpp function: DaqAdministration::distributeDataNew
    */
   ramRingSharedSynchonizeReadIndex( &g_shared.mDaq.memAdmin );
#endif

   for( unsigned int i = 0; i < ARRAY_SIZE(g_aMilTaskData); i++ )
   {
      milTask( &g_aMilTaskData[i] );
   }
}

/*================================== EOF ====================================*/
