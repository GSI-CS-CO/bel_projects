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
extern volatile uint16_t*           g_pScub_base;

/*!
 * @see scu_main.c
 */
extern volatile FG_MESSAGE_BUFFER_T g_aMsg_buf[QUEUE_CNT];

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
timeMeasure( &g_irqTimeMeasurement );
   /*!
    * @todo Move the FG-buffer into the DDR3-RAM!
    */
   if( !cbReadSave( &g_shared.fg_buffer[0], &g_shared.fg_regs[0],
                pThis->cntrl_reg.bv.number, &pset ) )
   {
      hist_addx( HISTORY_XYZ_MODULE, "buffer empty, no parameter sent",
                 pThis->cntrl_reg.bv.number );
      return false;
   }

   timeMeasure( &g_irqTimeMeasurement );
   /*
    * clear freq, step select, fg_running and fg_enabled
    */
   setAdacFgRegs( pThis, &pset, (pThis->cntrl_reg.i16 & ~(0xfc07)) |
                                ((pset.control & 0x3F) << 10) );

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
                   const unsigned int fgAddrOffset )
{
  // timeMeasure( &g_irqTimeMeasurement );
   FG_REGISTER_T* pFgRegs = getFgRegisterPtrByOffsetAddr( (void*)g_pScub_base,
                                                          slot, fgAddrOffset );
   const unsigned int channel = pFgRegs->cntrl_reg.bv.number;
   if( channel >= ARRAY_SIZE( g_shared.fg_regs ) )
   {
      mprintf( ESC_ERROR"%s: Channel of ADAC FG out of range: %d\n"ESC_NORMAL,
               __func__, channel );
      return;
   }

   g_shared.fg_regs[channel].ramp_count =  pFgRegs->ramp_cnt_low;
   g_shared.fg_regs[channel].ramp_count |= pFgRegs->ramp_cnt_high << BIT_SIZEOF( uint16_t );


   //for( unsigned int i = 0; i < 10000; i++ ) NOP(); //!!Testing how many time we still have...

   if( pFgRegs->cntrl_reg.bv.isRunning )
   {
      if( pFgRegs->cntrl_reg.bv.dataRequest )
         makeStart( channel );
      sendRefillSignalIfThreshold( channel );

   #ifdef CONFIG_USE_SENT_COUNTER
      if( feedAdacFg( pFgRegs ) )
         g_aFgChannels[channel].param_sent++;
   #else
      feedAdacFg( pFgRegs );
   #endif
   }
   else
   {
      makeStop( channel );
   }
  // timeMeasure( &g_irqTimeMeasurement );
}

/*================================== EOF ====================================*/
