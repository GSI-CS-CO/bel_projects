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
#ifdef CONFIG_MIL_FG
   #include "scu_mil_fg_handler.h"
#endif
#ifdef CONFIG_SCU_DAQ_INTEGRATION
  #include "daq_main.h"
#endif

extern volatile uint16_t*     g_pScub_base;
#ifdef CONFIG_MIL_FG
extern volatile unsigned int* g_pScu_mil_base;
STATIC_ASSERT( sizeof( *g_pScu_mil_base ) == sizeof( uint32_t ) );
#endif

/*!
 * @brief Memory space of sent function generator data.
 *        Non shared memory part for each function generator channel.
 */
FG_CHANNEL_T g_aFgChannels[MAX_FG_CHANNELS] =
#ifdef CONFIG_USE_SENT_COUNTER
   {{0,0}};//,0}};
#else
   {{0}};
#endif

/*! ---------------------------------------------------------------------------
 * @brief Prints a error message happened in the device-bus respectively
 *        MIL bus.
 * @param status return status of the MIL-driver module.
 * @param slot Slot-number in the case the mil connection is established via
 *             SCU-Bus
 * @param msg String containing additional message text.
 */
void printDeviceError( const int status, const int slot, const char* msg )
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

/*! --------------------------------------------------------------------------
 */
ONE_TIME_CALL uint16_t getFgControlRegValue( const FG_PARAM_SET_T* pPset,
                                             const unsigned int channel )
{
   return ((pPset->control & 0x3F) << 10) | (channel << 4);
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
   FG_ASSERT( dev < MAX_FG_PER_SLAVE );

   unsigned int dacControlIndex;
   BUS_BASE_T   fg_base;
   uint16_t     irqMask;
   switch( dev )
   {
      case 0:
      {
         irqMask         = FG1_IRQ;
         fg_base         = FG1_BASE;
         dacControlIndex = DAC1_BASE + DAC_CNTRL;
         break;
      }
      case 1:
      {
         irqMask         = FG2_IRQ;
         fg_base         = FG2_BASE;
         dacControlIndex = DAC2_BASE + DAC_CNTRL;
         break;
      }
      default:
      {
         FG_ASSERT( false );
         return NULL;
      }
   }

#ifdef CONFIG_SCU_DAQ_INTEGRATION
  /*
   * Enabling of both daq-channels for the feedback of set- and actual values
   * for this function generator channel.
   */
   daqEnableFgFeedback( slot, dev );
#endif

  /*
   * Enable interrupts for the slave
   */
#ifdef _CONFIG_IRQ_ENABLE_IN_START_FG
   scuBusEnableSlaveInterrupt( pScuBus, slot );
#endif

   *scuBusGetInterruptActiveFlagRegPtr( pScuBus, slot ) |= irqMask;
   *scuBusGetInterruptEnableFlagRegPtr( pScuBus, slot ) |= irqMask;


  /*
   * Set ADDAC-DAC in FG mode
   */
   scuBusSetSlaveValue16( scuBusGetAbsSlaveAddr( pScuBus, slot ), dacControlIndex, 0x10 );

   FG_REGISTER_T* pAddagFgRegs = getFgRegisterPtrByOffsetAddr( pScuBus, slot, fg_base );

   /*
    * Resetting of the ramp-counter
    */
   ADDAC_FG_ACCESS( pAddagFgRegs, ramp_cnt_low )  = 0;
   ADDAC_FG_ACCESS( pAddagFgRegs, ramp_cnt_high ) = 0;

   /*
    * Setting of the ECA timing tag.
    */
   ADDAC_FG_ACCESS( pAddagFgRegs, tag_low )  = GET_LOWER_HALF( tag );
   ADDAC_FG_ACCESS( pAddagFgRegs, tag_high ) = GET_UPPER_HALF( tag );

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
{
   const uint16_t cntrl_reg_wr = getFgControlRegValue( pPset, channel );
   /*
    * CAUTION: Don't change the order of the following both code lines!
    */
   setAdacFgRegs( pAddagFgRegs, pPset, cntrl_reg_wr );
   ADDAC_FG_ACCESS( pAddagFgRegs, cntrl_reg.i16 ) |= FG_ENABLED;
}

#ifdef CONFIG_MIL_FG
/*! ---------------------------------------------------------------------------
 * @brief Helper function of configure_fg_macro handles the handler state
 *        of MIL devices.
 * @see configure_fg_macro
 * @param pScuBus Base address of SCU-bus.
 * @param pMilBus Base address of MIL-bus.
 * @param socket Socket number containing location information and FG-typ
 * @retval true leave the function configure_fg_macro
 * @retval false continue the function configure_fg_macro
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
         clear_handler_state( socket );
         hist_addx( HISTORY_XYZ_MODULE, "clear_handler_state", socket );
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
         printDeviceError( status, slot, "enable dreq");
         return status;
      }

     /*
      * Set MIL-DAC in FG mode
      */
      if( (status = scub_write_mil((volatile unsigned short*) pScuBus, slot, 0x1, FC_IFAMODE_WR | dev)) != OKAY)
      {
         printDeviceError( status, slot, "set FG mode");
      }
      return status;
   }

   FG_ASSERT( isMilExtentionFg( socket ) );

  /*
   * Enable data request
   */
   if( (status = write_mil((volatile unsigned int*) pMilBus, 1 << 13, FC_IRQ_MSK | dev)) != OKAY)
   {
      printDeviceError( status, 0, "enable dreq" );
      return status;
   }

   /*
    * Set MIL-DAC in FG mode
    */
   if( (status = write_mil((volatile unsigned int*) pMilBus, 0x1, FC_IFAMODE_WR | dev)) != OKAY)
      printDeviceError( status, 0, "set FG mode");

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
         printDeviceError( status, slot, "blk trm");
         return status;
      }

     /*
      * Still in block mode !
      */
      if( (status = scub_write_mil( (volatile unsigned short*) pScuBus,
                                    slot,
                                    cntrl_reg_wr, FC_CNTRL_WR | dev)) != OKAY)
      {
         printDeviceError( status, slot, "end blk trm");
         return status;
      }

      if( (status = scub_write_mil( (volatile unsigned short*) pScuBus,
                                    slot,
                                    cntrl_reg_wr | FG_ENABLED, FC_CNTRL_WR | dev ) ) != OKAY )
      {
         printDeviceError( status, slot, "end blk mode");
      }
      return status;
   }

   FG_ASSERT( isMilExtentionFg( socket ) );

   if((status = write_mil_blk( (volatile unsigned int*)pMilBus,
                               (short*)&milFgRegs,
                               FC_BLK_WR | dev)) != OKAY)
   {
      printDeviceError( status, 0, "blk trm");
      return status;
   }
   /*
    * Still in block mode !
    */
   if((status = write_mil( (volatile unsigned int*)pMilBus,
                           cntrl_reg_wr,
                           FC_CNTRL_WR | dev)) != OKAY)
   {
      printDeviceError( status, 0, "end blk trm");
      return status;
   }

   if( (status = write_mil( (volatile unsigned int*)pMilBus,
                            cntrl_reg_wr | FG_ENABLED, FC_CNTRL_WR | dev)) != OKAY)
      printDeviceError( status, 0, "end blk mode");

   #if __GNUC__ >= 9
     #pragma GCC diagnostic pop
   #endif

   return status;
}

#endif /* ifdef  CONFIG_MIL_FG */

/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 * @todo Split this in two separate functions: MIL and non-MIL.
 */
void configure_fg_macro( const unsigned int channel )
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
      STATIC_ASSERT( sizeof( g_shared.fg_regs[0].tag ) == sizeof( uint32_t ) );
      STATIC_ASSERT( sizeof( pAddagFgRegs->tag_low ) == sizeof( g_shared.fg_regs[0].tag ) / 2 );
      STATIC_ASSERT( sizeof( pAddagFgRegs->tag_high ) == sizeof( g_shared.fg_regs[0].tag ) / 2 );

      /*
       * Note: In the case of ADDAC/ACU-FGs the socket-number is equal
       *       to the slot number.
       */
      pAddagFgRegs = addacFgPrepare( (void*)g_pScub_base,
                                     socket, dev,
                                     g_shared.fg_regs[channel].tag );
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
   if( cbReadSave( &g_shared.fg_buffer[0], &g_shared.fg_regs[0], channel, &pset ) != 0 )
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
   g_shared.fg_regs[channel].state = STATE_ARMED;
   sendSignal( IRQ_DAT_ARMED, channel );
   return;
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
   unsigned int fgControlIndex, dacControlIndex;
   switch( dev )
   {
      case 0:
      {
         fgControlIndex  = FG1_BASE  + FG_CNTRL;
         dacControlIndex = DAC1_BASE + DAC_CNTRL;
         break;
      }
      case 1:
      {
         fgControlIndex  = FG2_BASE  + FG_CNTRL;
         dacControlIndex = DAC2_BASE + DAC_CNTRL;
         break;
      }
      default:
      {
         FG_ASSERT( false );
         return;
      }
   }
   /*
    * Disarm hardware
    */
   const void* pAbsSlaveAddr = scuBusGetAbsSlaveAddr( pScuBus, slot );
   *scuBusGetSlaveRegisterPtr16( pAbsSlaveAddr, fgControlIndex ) &= ~(0x2);

   /*
    * Unset FG mode in ADC
    */
   *scuBusGetSlaveRegisterPtr16( pAbsSlaveAddr, dacControlIndex ) &= ~(0x10);

#ifdef CONFIG_SCU_DAQ_INTEGRATION
  /*
   * Disabling of both daq-channels for the feedback of set- and actual values
   * for this function generator channel.
   */
   daqDisableFgFeedback( slot, dev );
#endif
}

#ifdef CONFIG_MIL_FG
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
         printDeviceError( status, slot, "disarm hw 3" );
         return status;
      }

      if( (status = scub_write_mil( (volatile unsigned short*) pScuBus, slot,
           data & ~(0x2), FC_CNTRL_WR | dev)) != OKAY )
         printDeviceError( status, slot, "disarm hw 4" );

      return status;
   }

   FG_ASSERT( isMilExtentionFg( socket ) );

   if( (status = read_mil( (volatile unsigned int*)pMilBus, &data,
                           FC_CNTRL_RD | dev)) != OKAY )
   {
      printDeviceError( status, 0, "disarm hw 1" );
      return status;
   }

   if( (status = write_mil( (volatile unsigned int*)pMilBus,
                            data & ~(0x2),
                            FC_CNTRL_WR | dev)) != OKAY )
      printDeviceError( status, 0, "disarm hw 2" );

   return status;
}
#endif /* ifdef  CONFIG_MIL_FG */

/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 */
void disable_channel( const unsigned int channel )
{
   FG_ASSERT( channel < ARRAY_SIZE( g_shared.fg_regs ) );

   FG_CHANNEL_REG_T* pFgRegs = &g_shared.fg_regs[channel];

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

/*! --------------------------------------------------------------------------
 * @ingroup INTERRUPT
 */
ONE_TIME_CALL void addacFgDisableIrq( const void* pScuBus,
                                      const unsigned int slot,
                                      const unsigned int dev )
{
   unsigned int invIrqMask;
   switch( dev )
   {
      case 0:
      {
         invIrqMask = ~FG1_IRQ;
         break;
      }
      case 1:
      {
         invIrqMask = ~FG2_IRQ;
         break;
      }
      default:
      {
         FG_ASSERT( false );
         return;
      }
   }
   *scuBusGetInterruptEnableFlagRegPtr( pScuBus, slot ) &= invIrqMask;
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief disables the generation of irqs for the specified channel
 *  SIO and MIL extension stop generating irqs
 *  @param channel number of the channel from 0 to MAX_FG_CHANNELS-1
 * @see enable_scub_msis
 */
void disable_slave_irq( const unsigned int channel )
{
   if( channel >= MAX_FG_CHANNELS )
      return;

   const unsigned int socket = getSocket( channel );
   const unsigned int dev    = getDevice( channel );

   //mprintf("IRQs for slave %d disabled.\n", socket);

   if( isAddacFg( socket ) )
   { /*
      * In the case of ADDAC/ACU-FGs the socket-number is equal to the
      * slot number, so it's not necessary to extract the slot number here.
      */
      addacFgDisableIrq( (void*)g_pScub_base, socket, dev );
      return;
   }

#ifdef CONFIG_MIL_FG
   int status = OKAY;

   if( isMilExtentionFg( socket ) )
   {
      //write_mil(g_pScu_mil_base, 0x0, FC_COEFF_A_WR | dev);            //ack drq
      status = write_mil( g_pScu_mil_base, 0x0, FC_IRQ_MSK | dev);
   }
   else if( isMilScuBusFg( socket ) )
   {
      status = scub_write_mil( g_pScub_base, getFgSlotNumber( socket ),
                                   0x0, FC_IRQ_MSK | dev);
   }
   if( status != OKAY )
      printDeviceError( status, getFgSlotNumber( socket ), __func__);
#endif /* ifdef CONFIG_MIL_FG */
}

/*! ---------------------------------------------------------------------------
 * @brief Send signal REFILL to the SAFTLIB when the fifo level has
 *        the threshold reached. Helper function of function handleMacros().
 * @see handleMacros
 * @param channel Channel of concerning function generator.
 */
void sendRefillSignalIfThreshold( const unsigned int channel )
{
   if( cbgetCountSave( &g_shared.fg_regs[0], channel ) == FG_REFILL_THRESHOLD )
   {
     // mprintf( "*" ); //!!
      sendSignal( IRQ_DAT_REFILL, channel );
   }
}

/*================================== EOF ====================================*/
