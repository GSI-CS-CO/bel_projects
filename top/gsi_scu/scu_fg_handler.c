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

/*!
 * @see scu_main.c
 */
extern volatile uint16_t*     g_pScub_base;
extern FG_CHANNEL_T           g_aFgChannels[MAX_FG_CHANNELS];

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
      hist_addx( HISTORY_XYZ_MODULE, "buffer empty, no parameter sent",
                 pThis->cntrl_reg.bv.number );
      return false;
   }

   /*
    * Clear all except the function generator number.
    */
   setAdacFgRegs( pThis,
                  &pset,
                  (pThis->cntrl_reg.i16 & FG_NUMBER) |
                  ((pset.control.i32 & (PSET_STEP | PSET_FREQU)) << 10) );
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
      mprintf( ESC_ERROR"%s: Channel of ADAC FG out of range: %d\n"ESC_NORMAL,
               __func__, channel );
      return;
   }

#ifndef __DOXYGEN__
   STATIC_ASSERT( sizeof( pFgRegs->ramp_cnt_high ) == sizeof( pFgRegs->ramp_cnt_low ) );
   STATIC_ASSERT( sizeof( g_shared.oSaftLib.oFg.aRegs[0].ramp_count ) >= 2 * sizeof( pFgRegs->ramp_cnt_low ) );
#endif
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
