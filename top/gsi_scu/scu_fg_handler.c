/*!
 * @file scu_fg_handler.c
 * @brief  Module for handling all SCU-BUS function generators
 *         (non MIL ADAC function generators)
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 04.02.2020
 * Outsourced from scu_main.c
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/AdcDacScu
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/AdcDac2Scu
 */
#include "scu_fg_macros.h"
#include "scu_fg_handler.h"
#ifdef CONFIG_SCU_DAQ_INTEGRATION
  #include "daq_main.h"
#endif

/*!
 * @see scu_main.c
 */
extern volatile uint16_t*     g_pScub_base;
extern FG_CHANNEL_T           g_aFgChannels[MAX_FG_CHANNELS];

#define DAC_FG_MODE   0x0010

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

STATIC_ASSERT( ARRAY_SIZE(mg_devTab) == MAX_FG_PER_SLAVE );

#ifndef CONFIG_SCU_DAQ_INTEGRATION
/*! ---------------------------------------------------------------------------
 * @see scu_fg_handler.h
 */
void scanScuBusFgsDirect( const void* pScuBusBase, FG_MACRO_T* pFGlist )
{
   const SCUBUS_SLAVE_FLAGS_T slotFlags = scuBusFindSpecificSlaves( pScuBusBase,
                                                                    SYS_CSCO,
                                                                    GRP_ADDAC2 )
                                        | scuBusFindSpecificSlaves( pScuBusBase,
                                                                    SYS_CSCO,
                                                                    GRP_ADDAC1 );

   if( slotFlags == 0 )
      return;

   SCU_BUS_FOR_EACH_SLAVE( slot, slotFlags )
   {
      addAddacToFgList( pScuBusBase, slot, pFGlist );
   #ifndef _CONFIG_IRQ_ENABLE_IN_START_FG
      scuBusEnableSlaveInterrupt( pScuBusBase, slot );
   #endif
   }
}
#endif /* ifndef CONFIG_SCU_DAQ_INTEGRATION */

#ifdef CONFIG_NON_DAQ_FG_SUPPORT
/*! ---------------------------------------------------------------------------
 * @brief Scans the whole SCU bus for specific slaves having a
 *        function generator and add it to the function generator list if
 *        any found.
 * @param pScuBusBase Base address of SCU bus
 * @param pFgList Start pointer of function generator list.
 * @param systemAddr System address
 * @param groupAddr  Group address
 */
STATIC void scanScuBusForFg( void* pScuBus, FG_MACRO_T* pFgList,
                      SLAVE_SYSTEM_T systemAddr, SLAVE_GROUP_T groupAddr )
{
   const SCUBUS_SLAVE_FLAGS_T slotFlags = scuBusFindSpecificSlaves( pScuBus,
                                                                    systemAddr,
                                                                    groupAddr );
   if( slotFlags == 0 )
      return;

   SCU_BUS_FOR_EACH_SLAVE( slot, slotFlags )
   {
      fgListAdd( slot,
                 0,
                 systemAddr,
                 groupAddr,
                 getFgFirmwareVersion( pScuBus, slot ),
                 pFgList );
   #ifndef _CONFIG_IRQ_ENABLE_IN_START_FG
      scuBusEnableSlaveInterrupt( pScuBus, slot );
   #endif
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_handler.h
 */
void scanScuBusFgsWithoutDaq( volatile uint16_t *scub_adr, FG_MACRO_T* pFgList )
{
   scanScuBusForFg( (void*)scub_adr, pFgList, SYS_PBRF, GRP_FIB_DDS );
 #ifndef CONFIG_DIOB_WITH_DAQ
   scanScuBusForFg( (void*)scub_adr, pFgList, SYS_CSCO, GRP_DIOB );
 #endif
}
#endif /* ifdef CONFIG_NON_DAQ_FG_SUPPORT */

/*! ---------------------------------------------------------------------------
 * @brief Sets the registers of a ADAC function generator.
 */
STATIC inline void setAdacFgRegs( FG_REGISTER_T* pFgRegs,
                                  const FG_PARAM_SET_T* pPset,
                                  const uint16_t controlReg )
{
   STATIC_ASSERT( sizeof( pFgRegs->start_l ) * 2 == sizeof( pPset->coeff_c ));
   STATIC_ASSERT( sizeof( pFgRegs->start_h ) * 2 == sizeof( pPset->coeff_c ));

   ADDAC_FG_ACCESS( pFgRegs, cntrl_reg.i16 ) = controlReg;
   ADDAC_FG_ACCESS( pFgRegs, coeff_a_reg )   = pPset->coeff_a;
   ADDAC_FG_ACCESS( pFgRegs, coeff_b_reg )   = pPset->coeff_b;
   ADDAC_FG_ACCESS( pFgRegs, shift_reg )     = getFgShiftRegValue( pPset );
   ADDAC_FG_ACCESS( pFgRegs, start_l )       = GET_LOWER_HALF( pPset->coeff_c );
   ADDAC_FG_ACCESS( pFgRegs, start_h )       = GET_UPPER_HALF( pPset->coeff_c );
}

/*! ---------------------------------------------------------------------------
 * @todo Replace this function by access via type FG_CTRL_RG_T
 * @see FG_CTRL_RG_T
 */
STATIC inline unsigned int getFgNumberFromRegister( const uint16_t reg )
{
   const FG_CTRL_RG_T ctrlReg = { .i16 = reg };
   return ctrlReg.bv.number;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the pointer of the SCU-function generators control register.
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @param slot Slot number, valid range 1 .. MAX_SCU_SLAVES (12)
 * @param number Number of functions generator macro 0 or 1.
 * @return Pointer to the control-register.
 */
STATIC volatile inline
FG_CTRL_RG_T* getFgCntrlRegPtr( const void* pScuBusBase,
                                const unsigned int slot,
                                const unsigned int number )
{
   return &getFgRegisterPtr( pScuBusBase, slot, number )->cntrl_reg;
}



/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 */
inline BUS_BASE_T getFgOffsetAddress( const unsigned int number )
{
   FG_ASSERT( number < ARRAY_SIZE( mg_devTab ) );
   return mg_devTab[number].fgBaseAddr;
}


/*! ---------------------------------------------------------------------------
 * @see scu_fg_handler.h
 */
inline FG_REGISTER_T* addacFgPrepare( const void* pScuBus,
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

      STATIC_ASSERT( sizeof( pAddagFgRegs->tag_low )  * 2 == sizeof( tag ) );
      STATIC_ASSERT( sizeof( pAddagFgRegs->tag_high ) * 2 == sizeof( tag ) );

      /*
       * Setting of the ECA timing tag.
       */
      ADDAC_FG_ACCESS( pAddagFgRegs, tag_low )  = GET_LOWER_HALF( tag );
      ADDAC_FG_ACCESS( pAddagFgRegs, tag_high ) = GET_UPPER_HALF( tag );
   } /* ATOMIC_SECTION() */

   return pAddagFgRegs;
}


/*! ---------------------------------------------------------------------------
 * @see scu_fg_handler.h
 */
inline void addacFgStart( FG_REGISTER_T* pAddagFgRegs,
                          const FG_PARAM_SET_T* pPset,
                          const unsigned int channel )
{  /*
    * CAUTION: Don't change the order of the following both code lines!
    */
   setAdacFgRegs( pAddagFgRegs, pPset, getFgControlRegValue( pPset, channel ));
   ADDAC_FG_ACCESS( pAddagFgRegs, cntrl_reg.i16 ) |= FG_ENABLED;
}

/*! --------------------------------------------------------------------------
 * @see scu_fg_handler.h
 */
inline void addacFgDisableIrq( const void* pScuBus,
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

/*! --------------------------------------------------------------------------
 * @see scu_fg_handler.h
 */
inline void addacFgDisable( const void* pScuBus,
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

//#if defined( CONFIG_SCU_DAQ_INTEGRATION ) && !defined( CONFIG_DISABLE_FEEDBACK_IN_DISABLE_IRQ )
  /*
   * Disabling of both daq-channels for the feedback of set- and actual values
   * for this function generator channel.
   */
   daqDisableFgFeedback( slot, dev );
//#endif
}


/*! ---------------------------------------------------------------------------
 * @brief Supplies an  ADAC- function generator with data.
 * @param pThis Pointer to the concerning FG-macro register set.
 * @todo Replace this hideous bit-masking and bit-shifting by bit fields
 *       as far as possible!
 * @retval true Polynom successful sent.
 * @retval false Buffer was empty no data sent.
 */
ONE_TIME_CALL bool feedAdacFg( FG_REGISTER_T* pThis )
{
   FG_PARAM_SET_T pset;

   /*!
    * @todo Move the FG-buffer into the DDR3-RAM!
    */
   if( !cbReadSave( &g_shared.oSaftLib.oFg.aChannelBuffers[0],
                    &g_shared.oSaftLib.oFg.aRegs[0],
                    pThis->cntrl_reg.bv.number, &pset ) )
   {
      hist_addx( HISTORY_XYZ_MODULE, "ADDAC-FG buffer empty, no parameter sent",
                 pThis->cntrl_reg.bv.number );
      lm32Log( LM32_LOG_ERROR, ESC_ERROR
               "ERROR: Buffer of ADDAC-FG: fg-%u-%u, channel: %u is empty!\n" ESC_NORMAL,
               getSocket(pThis->cntrl_reg.bv.number),
               getDevice(pThis->cntrl_reg.bv.number),
               pThis->cntrl_reg.bv.number
             );
      return false;
   }

   /*
    * Clear all except the function generator number.
    */
   setAdacFgRegs( pThis,
                  &pset,
                  (pThis->cntrl_reg.i16 & FG_NUMBER) |
                  ((pset.control.i32 & (PSET_STEP | PSET_FREQU)) << 10) );

 #ifdef CONFIG_USE_FG_MSI_TIMEOUT
   wdtReset( pThis->cntrl_reg.bv.number );
 #endif

   return true;
}

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Handles a ADAC-FG
 * @param slot SCU-bus slot number respectively slave number.
 * @param fgAddrOffset Relative address offset of the concerning FG-macro
 *                     till now FG1_BASE or FG2_BASE.
 */
void handleAdacFg( const unsigned int slot,
                   const BUS_BASE_T fgAddrOffset )
{
   FG_REGISTER_T* pFgRegs = getFgRegisterPtrByOffsetAddr( (void*)g_pScub_base,
                                                          slot, fgAddrOffset );
   const unsigned int channel = pFgRegs->cntrl_reg.bv.number;

   if( channel >= ARRAY_SIZE( g_shared.oSaftLib.oFg.aRegs ) )
   {
   #ifdef CONFIG_USE_LM32LOG
      lm32Log( LM32_LOG_ERROR, ESC_ERROR 
               "%s: Channel of ADAC FG out of range: %d\n" ESC_NORMAL,
               __func__, channel );
   #else
      mprintf( ESC_ERROR "%s: Channel of ADAC FG out of range: %d\n" ESC_NORMAL,
               __func__, channel );
   #endif
      return;
   }

   STATIC_ASSERT( sizeof( pFgRegs->ramp_cnt_high ) == sizeof( pFgRegs->ramp_cnt_low ) );
   STATIC_ASSERT( sizeof( g_shared.oSaftLib.oFg.aRegs[0].ramp_count ) >= 2 * sizeof( pFgRegs->ramp_cnt_low ) );

   /*
    * Read the hardware ramp counter respectively polynomial counter
    * from the concerning function generator.
    */
   g_shared.oSaftLib.oFg.aRegs[channel].ramp_count = MERGE_HIGH_LOW( ADDAC_FG_ACCESS( pFgRegs, ramp_cnt_high ),
                                                                     ADDAC_FG_ACCESS( pFgRegs, ramp_cnt_low ));

   const uint16_t controlReg = ADDAC_FG_ACCESS( pFgRegs, cntrl_reg.i16 );
   /*
    * Is function generator running?
    */
   if( (controlReg & FG_RUNNING) == 0 )
   { /*
      * Function generator has stopped.
      * Sending a appropriate stop-message including the reason
      * to the SAFT-lib.
      */
      makeStop( channel );
      return;
   }

   /*
    * Function generator is running.
    */

   if( (controlReg & FG_DREQ) == 0 )
   { /*
      * The concerned function generator has received the
      * timing- tag or the broadcast message.
      * Sending a start-message to the SAFT-lib.
      */
      makeStart( channel );
   }

   /*
    * Send a refill-message to the SAFT-lib if
    * the buffer has reached a critical level.
    */
   sendRefillSignalIfThreshold( channel );

   /*
    * Sending the current polynomial data set to the function generator.
    */
#ifdef CONFIG_USE_SENT_COUNTER
   if( feedAdacFg( pFgRegs ) )
      g_aFgChannels[channel].param_sent++;
#else
   feedAdacFg( pFgRegs );
#endif
}

/*================================== EOF ====================================*/
