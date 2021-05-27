/*!
 * @file scu_fg_macros.c
 * @brief Module for handling MIL and non MIL
 *        function generator macros
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/ScuFgDoc
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/ScuFgDoc
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 04.02.2020
 * Outsourced from scu_main.c
 */
#include "scu_fg_macros.h"
#include "scu_fg_handler.h"
#include "scu_fg_list.h"
#ifdef CONFIG_MIL_FG
   #include "scu_mil_fg_handler.h"
#endif
#ifdef CONFIG_SCU_DAQ_INTEGRATION
  #include "daq_main.h"
#endif

#define CONFIG_DISABLE_FEEDBACK_IN_DISABLE_IRQ

extern volatile uint16_t*     g_pScub_base;
extern volatile uint32_t*     g_pScub_irq_base;

#ifdef CONFIG_MIL_FG
extern volatile unsigned int* g_pScu_mil_base;
extern volatile uint32_t*     g_pMil_irq_base;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( *g_pScu_mil_base ) == sizeof( uint32_t ) );
#endif

typedef enum
{
   MIL_INL = 0x00,
   MIL_DRY = 0x01,
   MIL_DRQ = 0x02
} MIL_T;


#endif

#define DAC_FG_MODE   0x0010

/*!
 * @brief Memory space of sent function generator data.
 *        Non shared memory part for each function generator channel.
 */
FG_CHANNEL_T g_aFgChannels[MAX_FG_CHANNELS] =
#ifdef CONFIG_USE_SENT_COUNTER
   {{0,0}};
#else
   {{0}};
#endif

#ifndef __DOXYGEN__
STATIC_ASSERT( ARRAY_SIZE(g_aFgChannels) == MAX_FG_CHANNELS );
#endif


/*!
 * @brief Container of device properties of a ADDAC/ACU-device
 */
typedef struct
{  /*!
    * @brief Control register address offset of the digital to analog converter.
    */
   const unsigned int dacControl;

   /*!
    * @brief Interrupt mask of the concerning function generator.
    */
   const uint16_t     fgIrqMask;

   /*!
    * @brief Base address-offset of the concerning function generator.
    */
   const BUS_BASE_T   fgBaseAddr;
} ADDAC_DEV_T;

/*!
 * @brief Property table of a ADDAC/ACU-slave device.
 */
STATIC const ADDAC_DEV_T mg_devTab[MAX_FG_PER_SLAVE] =
{
   {
      .dacControl = DAC1_BASE + DAC_CNTRL,
      .fgIrqMask  = FG1_IRQ,
      .fgBaseAddr = FG1_BASE
   },
   {
      .dacControl = DAC2_BASE + DAC_CNTRL,
      .fgIrqMask  = FG2_IRQ,
      .fgBaseAddr = FG2_BASE
   }
};

#ifndef __DOXYGEN__
STATIC_ASSERT( ARRAY_SIZE(mg_devTab) == MAX_FG_PER_SLAVE );
#endif

/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 */
inline BUS_BASE_T getFgOffsetAddress( const unsigned int number )
{
   FG_ASSERT( number < ARRAY_SIZE( mg_devTab ) );
   return mg_devTab[number].fgBaseAddr;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the control register format for step, frequency select
 *        and channel number.
 * @param pPset Pointer to the polynomial data set.
 * @param channel Channel number of the concerned function generator.
 * @return Value for the function generators control register.
 */
ONE_TIME_CALL uint16_t getFgControlRegValue( const FG_PARAM_SET_T* pPset,
                                             const unsigned int channel )
{
   return ((pPset->control.i32 & (PSET_STEP | PSET_FREQU)) << 10) |
          (channel << 4);
}

/*! ---------------------------------------------------------------------------
 * @brief Prepares the selected ADDAC/ACU- function generator.
 *
 * 1) Enabling the belonging interrupt. \n
 * 2) Starts both belonging DAQ channels for feedback set- and actual- values. \n
 * 3) Sets the digital to analog converter in the function generator mode. \n
 * 4) Resets the ramp-counter \n
 * 5) Sets the ECA-timing tag. \n
 *
 * @param pScuBus Pointer to the SCU-bus base address.
 * @param slot SCU-bus slot number respectively slave- number.
 * @param dev Device-number respectively one of the function generator
 *            belonging to this slave.
 * @param tag ECA- timing- tag (normally 0xDEADBEEF)
 * @return Base pointer of the registers of the selected function generator.
 */
ONE_TIME_CALL FG_REGISTER_T* addacFgPrepare( const void* pScuBus,
                                             const unsigned int slot,
                                             const unsigned int dev,
                                             const uint32_t tag
                                           )
{
   FG_ASSERT( dev < ARRAY_SIZE(mg_devTab) );

   const ADDAC_DEV_T* pAddacObj = &mg_devTab[dev];
   FG_REGISTER_T* pAddagFgRegs = getFgRegisterPtrByOffsetAddr( pScuBus, slot,
                                                               pAddacObj->fgBaseAddr );

#ifdef CONFIG_SCU_DAQ_INTEGRATION
  /*
   * Enabling of both daq-channels for the feedback of set- and actual values
   * for this function generator channel.
   */
   daqEnableFgFeedback( slot, dev );
#endif
   ATOMIC_SECTION()
   { /*
      * Enable interrupts for the slave
      */
   #ifdef _CONFIG_IRQ_ENABLE_IN_START_FG
      scuBusEnableSlaveInterrupt( pScuBus, slot );
   #endif
      *scuBusGetInterruptActiveFlagRegPtr( pScuBus, slot ) |= pAddacObj->fgIrqMask;
      *scuBusGetInterruptEnableFlagRegPtr( pScuBus, slot ) |= pAddacObj->fgIrqMask;


     /*
      * Set ADDAC-DAC in FG mode
      */
      scuBusSetSlaveValue16( scuBusGetAbsSlaveAddr( pScuBus, slot ),
                             pAddacObj->dacControl, DAC_FG_MODE );

      /*
       * Resetting of the ramp-counter
       */
      ADDAC_FG_ACCESS( pAddagFgRegs, ramp_cnt_low )  = 0;
      ADDAC_FG_ACCESS( pAddagFgRegs, ramp_cnt_high ) = 0;

   #ifndef __DOXYGEN__
      STATIC_ASSERT( sizeof( pAddagFgRegs->tag_low )  * 2 == sizeof( tag ) );
      STATIC_ASSERT( sizeof( pAddagFgRegs->tag_high ) * 2 == sizeof( tag ) );
   #endif
      /*
       * Setting of the ECA timing tag.
       */
      ADDAC_FG_ACCESS( pAddagFgRegs, tag_low )  = GET_LOWER_HALF( tag );
      ADDAC_FG_ACCESS( pAddagFgRegs, tag_high ) = GET_UPPER_HALF( tag );
   } /* ATOMIC_SECTION() */

   return pAddagFgRegs;
}


/*! ---------------------------------------------------------------------------
 * @brief Loads the selected ADDAC/ACU-function generator with the first
 *        polynomial data set and enable it.
 * @param pAddagFgRegs Base address of the register set of the selected
 *                     function generator.
 * @param pPset Pointer to the polynomial data set.
 * @param channel Channel number of the concerned function generator.
 */
ONE_TIME_CALL void addacFgStart( FG_REGISTER_T* pAddagFgRegs,
                                 const FG_PARAM_SET_T* pPset,
                                 const unsigned int channel )
{  /*
    * CAUTION: Don't change the order of the following both code lines!
    */
   setAdacFgRegs( pAddagFgRegs, pPset, getFgControlRegValue( pPset, channel ));
   ADDAC_FG_ACCESS( pAddagFgRegs, cntrl_reg.i16 ) |= FG_ENABLED;
}

/*! --------------------------------------------------------------------------
 * @ingroup INTERRUPT
 */
ONE_TIME_CALL void addacFgDisableIrq( const void* pScuBus,
                                      const unsigned int slot,
                                      const unsigned int dev )
{
   FG_ASSERT( dev < ARRAY_SIZE(mg_devTab) );

   *scuBusGetInterruptEnableFlagRegPtr( pScuBus, slot ) &= ~mg_devTab[dev].fgIrqMask;

#if defined( CONFIG_SCU_DAQ_INTEGRATION ) && defined( CONFIG_DISABLE_FEEDBACK_IN_DISABLE_IRQ)
  /*
   * Disabling of both daq-channels for the feedback of set- and actual values
   * for this function generator channel.
   */
   daqDisableFgFeedback( slot, dev );
#endif
}

/*! ---------------------------------------------------------------------------
 *  @brief Disables a running function generator.
 *
 * 1)
 *
 * @param pScuBus Pointer to the SCU- bus.
 * @param solt Scu-bus slot number respectively slave number.
 * @param dev Function generator number of the concerning slave.
 */
ONE_TIME_CALL void addacFgDisable( const void* pScuBus,
                                   const unsigned int slot,
                                   const unsigned int dev )
{
   FG_ASSERT( dev < ARRAY_SIZE(mg_devTab) );

   const ADDAC_DEV_T* pAddacObj = &mg_devTab[dev];
   const void* pAbsSlaveAddr    = scuBusGetAbsSlaveAddr( pScuBus, slot );

   /*
    * Disarm hardware
    */
   *scuBusGetSlaveRegisterPtr16( pAbsSlaveAddr, pAddacObj->fgBaseAddr + FG_CNTRL ) &= ~FG_ENABLED;

   /*
    * Unset FG mode in ADC
    */
   *scuBusGetSlaveRegisterPtr16( pAbsSlaveAddr, pAddacObj->dacControl ) &= ~DAC_FG_MODE;

#if defined( CONFIG_SCU_DAQ_INTEGRATION ) && !defined( CONFIG_DISABLE_FEEDBACK_IN_DISABLE_IRQ )
  /*
   * Disabling of both daq-channels for the feedback of set- and actual values
   * for this function generator channel.
   */
   daqDisableFgFeedback( slot, dev );
#endif
}


#ifdef CONFIG_MIL_FG
/*! ---------------------------------------------------------------------------
 * @brief Prints a error message happened in the device-bus respectively
 *        MIL bus.
 * @param status return status of the MIL-driver module.
 * @param slot Slot-number in the case the mil connection is established via
 *             SCU-Bus
 * @param msg String containing additional message text.
 */
void milPrintDeviceError( const int status, const int slot, const char* msg )
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
     default:
     {
        mprintf( "%s%d failed with code %d" ESC_NORMAL "\n",
                 pText, slot, status);
        return;
     }
  }
  #undef __MSG_ITEM
  mprintf( "%s%d failed with message %s, %s" ESC_NORMAL "\n",
           pText, slot, pMessage, msg);
}

/*! ---------------------------------------------------------------------------
 * @brief Helper function of fgEnableChannel handles the handler state
 *        of MIL devices.
 * @see fgEnableChannel
 * @param pScuBus Base address of SCU-bus.
 * @param pMilBus Base address of MIL-bus.
 * @param socket Socket number containing location information and FG-typ
 * @retval true leave the function fgEnableChannel
 * @retval false continue the function fgEnableChannel
 */
ONE_TIME_CALL bool milHandleClearHandlerState( const void* pScuBus,
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
         hist_addx( HISTORY_XYZ_MODULE, "fgMilClearHandlerState", socket );
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
 * @brief Prepares the selected MIL- function generator.
 *
 * 1) Enabling the belonging interrupt \n
 * 2) Starts both belonging DAQ channels for feedback set- and actual- values. \n
 * 3) Sets the digital to analog converter in the function generator mode. \n
 *
 * @param pScuBus Base address of SCU-bus.
 * @param pMilBus Base address of MIL-bus.
 * @param socket Socket number containing location information and FG-typ
 * @param dev Device number of the concerning FG-device
 * @retval OKAY Action was successful
 */
ONE_TIME_CALL int milFgPrepare( const void* pScuBus,
                                const void* pMilBus,
                                const unsigned int socket,
                                const unsigned int dev )
{
   FG_ASSERT( !isAddacFg( socket ) );

   int status;
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
      if( (status = scub_write_mil( (volatile unsigned short*) pScuBus, slot, 1 << 13, FC_IRQ_MSK | dev)) != OKAY)
      {
         milPrintDeviceError( status, slot, "enable dreq");
         return status;
      }

     /*
      * Set MIL-DAC in FG mode
      */
      if( (status = scub_write_mil((volatile unsigned short*) pScuBus, slot, 0x1, FC_IFAMODE_WR | dev)) != OKAY)
      {
         milPrintDeviceError( status, slot, "set FG mode");
      }
      return status;
   }

   FG_ASSERT( isMilExtentionFg( socket ) );

  /*
   * Enable data request
   */
   if( (status = write_mil((volatile unsigned int*) pMilBus, 1 << 13, FC_IRQ_MSK | dev)) != OKAY)
   {
      milPrintDeviceError( status, 0, "enable dreq" );
      return status;
   }

   /*
    * Set MIL-DAC in FG mode
    */
   if( (status = write_mil((volatile unsigned int*) pMilBus, 0x1, FC_IFAMODE_WR | dev)) != OKAY)
      milPrintDeviceError( status, 0, "set FG mode");

   return status;
}

/*! ---------------------------------------------------------------------------
 * @brief Loads the selected ADDAC/ACU-function generator with the first
 *        polynomial data set and enable it.
 * @param pScuBus Base address of SCU-bus.
 * @param pMilBus Base address of MIL-bus.
 * @param pPset Pointer to the polynomial data set.
 * @param socket Socket number containing location and device type
 * @param dev Device number
 * @param channel Channel number of the concerned function generator.
 * @retval OKAY Action was successful.
 */
ONE_TIME_CALL int milFgStart( const void* pScuBus,
                              const void* pMilBus,
                              const FG_PARAM_SET_T* pPset,
                              const unsigned int socket,
                              const unsigned int dev,
                              const unsigned int channel )
{
   FG_ASSERT( !isAddacFg( socket ) );
   int status;
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

      if( (status = scub_write_mil_blk( (volatile unsigned short*) pScuBus,
                                        slot,
                                        (short*)&milFgRegs,
                                        FC_BLK_WR | dev)) != OKAY )
      {
         milPrintDeviceError( status, slot, "blk trm");
         return status;
      }

     /*
      * Still in block mode !
      */
      if( (status = scub_write_mil( (volatile unsigned short*) pScuBus,
                                    slot,
                                    cntrl_reg_wr, FC_CNTRL_WR | dev)) != OKAY)
      {
         milPrintDeviceError( status, slot, "end blk trm");
         return status;
      }

      if( (status = scub_write_mil( (volatile unsigned short*) pScuBus,
                                    slot,
                                    cntrl_reg_wr | FG_ENABLED, FC_CNTRL_WR | dev ) ) != OKAY )
      {
         milPrintDeviceError( status, slot, "end blk mode");
      }
      return status;
   }

   FG_ASSERT( isMilExtentionFg( socket ) );

   if((status = write_mil_blk( (volatile unsigned int*)pMilBus,
                               (short*)&milFgRegs,
                               FC_BLK_WR | dev)) != OKAY)
   {
      milPrintDeviceError( status, 0, "blk trm");
      return status;
   }
   /*
    * Still in block mode !
    */
   if((status = write_mil( (volatile unsigned int*)pMilBus,
                           cntrl_reg_wr,
                           FC_CNTRL_WR | dev)) != OKAY)
   {
      milPrintDeviceError( status, 0, "end blk trm");
      return status;
   }

   if( (status = write_mil( (volatile unsigned int*)pMilBus,
                            cntrl_reg_wr | FG_ENABLED, FC_CNTRL_WR | dev)) != OKAY)
      milPrintDeviceError( status, 0, "end blk mode");

   #if __GNUC__ >= 9
     #pragma GCC diagnostic pop
   #endif

   return status;
}

#endif /* ifdef  CONFIG_MIL_FG */

/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 */
void fgEnableChannel( const unsigned int channel )
{
   FG_ASSERT( channel < ARRAY_SIZE( g_aFgChannels ) );

   const unsigned int socket = getSocket( channel );
   const unsigned int dev    = getDevice( channel );

   FG_REGISTER_T* pAddagFgRegs = NULL;

#ifdef CONFIG_MIL_FG
   int status = OKAY;

   if( isAddacFg( socket ) )
   {
#endif
   #ifndef __DOXYGEN__
      STATIC_ASSERT( sizeof( g_shared.oSaftLib.oFg.aRegs[0].tag ) == sizeof( uint32_t ) );
      STATIC_ASSERT( sizeof( pAddagFgRegs->tag_low ) == sizeof( g_shared.oSaftLib.oFg.aRegs[0].tag ) / 2 );
      STATIC_ASSERT( sizeof( pAddagFgRegs->tag_high ) == sizeof( g_shared.oSaftLib.oFg.aRegs[0].tag ) / 2 );
   #endif
      /*
       * Note: In the case of ADDAC/ACU-FGs the socket-number is equal
       *       to the slot number.
       */
      pAddagFgRegs = addacFgPrepare( (void*)g_pScub_base,
                                     socket, dev,
                                     g_shared.oSaftLib.oFg.aRegs[channel].tag );
#ifdef CONFIG_MIL_FG
   }
   else
   {
      if( milHandleClearHandlerState( (void*)g_pScub_base, (void*)g_pScu_mil_base, socket ) )
         return;

      status = milFgPrepare( (void*)g_pScub_base, (void*)g_pScu_mil_base, socket, dev );
      if( status != OKAY )
         return;
   }
#endif


   FG_PARAM_SET_T pset;
  /*
   * Fetch first parameter set from buffer
   */
   if( cbReadSave( &g_shared.oSaftLib.oFg.aChannelBuffers[0], &g_shared.oSaftLib.oFg.aRegs[0], channel, &pset ) != 0 )
   {
   #ifdef CONFIG_MIL_FG
      if( pAddagFgRegs != NULL )
      {
   #endif
         addacFgStart( pAddagFgRegs, &pset, channel );
   #ifdef CONFIG_MIL_FG
      }
      else
      {
         status = milFgStart( (void*)g_pScub_base,
                              (void*)g_pScu_mil_base,
                              &pset,
                              socket, dev, channel );
         if( status != OKAY )
            return;
      }
   #endif /* CONFIG_MIL_FG */
   #ifdef CONFIG_USE_SENT_COUNTER
      g_aFgChannels[channel].param_sent++;
   #endif
   } /* if( cbRead( ... ) != 0 ) */

 // reset watchdog
 //  g_aFgChannels[channel].timeout = 0;

   sendSignalArmed( channel );
}

#ifdef CONFIG_MIL_FG
/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Disables the interrupts of a specific MIL- function generator.
 * @param pScuBus Base address of SCU-bus.
 * @param pMilBus Base address of MIL-bus.
 * @param socket Socket number containing location and device type
 * @param dev Device number
 */
ONE_TIME_CALL void milFgDisableIrq( const void* pScuBus,
                                    const void* pMilBus,
                                    const unsigned int socket,
                                    const unsigned int dev )
{
   FG_ASSERT( !isAddacFg( socket ) );

   int status;

   if( isMilScuBusFg( socket ) )
   {
      status = scub_write_mil( (volatile unsigned short*)pScuBus,
                               getFgSlotNumber( socket ),
                               0x0, FC_IRQ_MSK | dev);
   }
   else
   {
      //write_mil((volatile unsigned int* )pMilBus, 0x0, FC_COEFF_A_WR | dev);  //ack drq
      status = write_mil( (volatile unsigned int* )pMilBus,
                          0x0, FC_IRQ_MSK | dev);

   }
   if( status != OKAY )
      milPrintDeviceError( status, getFgSlotNumber( socket ), __func__);
}

/*! ---------------------------------------------------------------------------
 */
ONE_TIME_CALL int milFgDisable( const void* pScuBus,
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
         milPrintDeviceError( status, slot, "disarm hw 3" );
         return status;
      }

      if( (status = scub_write_mil( (volatile unsigned short*) pScuBus, slot,
           data & ~(0x2), FC_CNTRL_WR | dev)) != OKAY )
         milPrintDeviceError( status, slot, "disarm hw 4" );

      return status;
   }

   FG_ASSERT( isMilExtentionFg( socket ) );

   if( (status = read_mil( (volatile unsigned int*)pMilBus, &data,
                           FC_CNTRL_RD | dev)) != OKAY )
   {
      milPrintDeviceError( status, 0, "disarm hw 1" );
      return status;
   }

   if( (status = write_mil( (volatile unsigned int*)pMilBus,
                            data & ~(0x2),
                            FC_CNTRL_WR | dev)) != OKAY )
      milPrintDeviceError( status, 0, "disarm hw 2" );

   return status;
}
#endif /* ifdef  CONFIG_MIL_FG */

/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 */
void fgDisableChannel( const unsigned int channel )
{
   FG_ASSERT( channel < ARRAY_SIZE( g_shared.oFg.aRegs ) );

   FG_CHANNEL_REG_T* pFgRegs = &g_shared.oSaftLib.oFg.aRegs[channel];

   if( pFgRegs->macro_number == SCU_INVALID_VALUE )
      return;

   const unsigned int socket = getSocket( channel );
   const unsigned int dev    = getDevice( channel );

#ifdef CONFIG_MIL_FG
   int status;
   if( isAddacFg( socket ) )
   {
#endif
      /*
       * Note: In the case if ADDAC/ACU- function generator the slot number
       *       is equal to the socket number.
       */
      addacFgDisable( (void*)g_pScub_base, socket, dev );
#ifdef CONFIG_MIL_FG
   }
   else
   {
      status = milFgDisable( (void*)g_pScub_base,
                             (void*)g_pScu_mil_base,
                             socket, dev );
      if( status != OKAY )
         return;

   }
#endif /* CONFIG_MIL_FG */

   if( pFgRegs->state == STATE_ACTIVE )
   {    // hw is running
      hist_addx( HISTORY_XYZ_MODULE, "flush circular buffer", channel );
      pFgRegs->rd_ptr = pFgRegs->wr_ptr;
   }
   else
   {
      pFgRegs->state = STATE_STOPPED;
      sendSignal( IRQ_DAT_DISARMED, channel );
   }
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
 * @todo Replace this awful naked index-numbers by well documented
 *       and meaningful constants!
 *
 * @param channel number of the channel between 0 and MAX_FG_CHANNELS-1
 * @see fgDisableInterrupt
 */
void scuBusEnableMeassageSignaledInterrupts( const unsigned int channel )
{
   const unsigned int socket = getSocket( channel );
#ifdef CONFIG_MIL_FG
   if( isAddacFg( socket ) || isMilScuBusFg( socket ) )
   {
#endif
      FG_ASSERT( getFgSlotNumber( socket ) > 0 );
      const uint16_t slot = getFgSlotNumber( socket ) - 1;
      ATOMIC_SECTION()
      {
         g_pScub_base[GLOBAL_IRQ_ENA] = 0x20;
         g_pScub_irq_base[MSI_CHANNEL_SELECT] = slot;
         g_pScub_irq_base[MSI_SOCKET_NUMBER]  = slot;
         g_pScub_irq_base[MSI_DEST_ADDR]      = (uint32_t)&((MSI_LIST_T*)pMyMsi)[0];
         g_pScub_irq_base[MSI_ENABLE]         = (1 << slot);
      }
#ifdef CONFIG_MIL_FG
      return;
   }

   if( !isMilExtentionFg( socket ) )
      return;

   ATOMIC_SECTION()
   {
      g_pMil_irq_base[MSI_CHANNEL_SELECT] = MIL_DRQ;
      g_pMil_irq_base[MSI_SOCKET_NUMBER]  = MIL_DRQ;
      //g_pMil_irq_base[MSI_DEST_ADDR]      = (uint32_t)pMyMsi + 0x20;
      g_pMil_irq_base[MSI_DEST_ADDR]      = (uint32_t)&((MSI_LIST_T*)pMyMsi)[2];
      g_pMil_irq_base[MSI_ENABLE]         = (1 << MIL_DRQ);
   }
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief disables the generation of irqs for the specified channel
 *  SIO and MIL extension stop generating irqs
 *  @param channel number of the channel from 0 to MAX_FG_CHANNELS-1
 * @see scuBusEnableMeassageSignaledInterrupts
 */
void fgDisableInterrupt( const unsigned int channel )
{
   if( channel >= MAX_FG_CHANNELS )
      return;

   const unsigned int socket = getSocket( channel );
   const unsigned int dev    = getDevice( channel );

   //mprintf("IRQs for slave %d disabled.\n", socket);
#ifdef CONFIG_MIL_FG
   if( isAddacFg( socket ) )
   {
#endif
     /*
      * In the case of ADDAC/ACU-FGs the socket-number is equal to the
      * slot number, so it's not necessary to extract the slot number here.
      */
      addacFgDisableIrq( (void*)g_pScub_base, socket, dev );
#ifdef CONFIG_MIL_FG
      return;
   }

   milFgDisableIrq( (void*)g_pScub_base, (void*)g_pScu_mil_base, socket, dev );
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Send signal REFILL to the SAFTLIB when the fifo level has
 *        the threshold reached. Helper function of function handleMacros().
 * @see handleMacros
 * @param channel Channel of concerning function generator.
 */
void sendRefillSignalIfThreshold( const unsigned int channel )
{
   if( cbgetCountSave( &g_shared.oSaftLib.oFg.aRegs[0], channel ) == FG_REFILL_THRESHOLD )
   {
     // mprintf( "*" ); //!!
      sendSignal( IRQ_DAT_REFILL, channel );
   }
}

/*================================== EOF ====================================*/
