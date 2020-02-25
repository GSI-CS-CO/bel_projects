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

//#pragma GCC push_options
//#pragma GCC optimize("O0")
/*! ---------------------------------------------------------------------------
 * @brief Supplies an  ADAC- function generator with data.
 * @param pThis Pointer to the concerning FG-macro register set.
 * @todo Replace this hideous bit-masking and bit-shifting by bit fields
 *       as far as possible!
 */
STATIC inline void feedAdacFg( FG_REGISTER_T* pThis )
{
   FG_PARAM_SET_T pset;

   if( !cbRead( &g_shared.fg_buffer[0], &g_shared.fg_regs[0],
                pThis->cntrl_reg.bv.number, &pset ) )
   {
      hist_addx( HISTORY_XYZ_MODULE, "buffer empty, no parameter sent",
                 pThis->cntrl_reg.bv.number );
      return;
   }

   FG_CTRL_RG_T controlReg;
   /*
    * clear freq, step select, fg_running and fg_enabled
    */
   controlReg.i16 = pThis->cntrl_reg.i16 & ~(0xfc07);
   controlReg.i16 |= ((pset.control & 0x38) << 10) | ((pset.control & 0x7) << 10);

   ADAC_FG_ACCESS( pThis, cntrl_reg.i16 ) = controlReg.i16;
   ADAC_FG_ACCESS( pThis, coeff_a_reg )   = pset.coeff_a;
   ADAC_FG_ACCESS( pThis, coeff_b_reg )   = pset.coeff_b;
   ADAC_FG_ACCESS( pThis, shift_reg )     = (pset.control & 0x3ffc0) >> 6;
   ADAC_FG_ACCESS( pThis, start_l )       = pset.coeff_c & 0xffff;
   ADAC_FG_ACCESS( pThis, start_h )       = pset.coeff_c >> BIT_SIZEOF(int16_t);
}
//#pragma GCC pop_options

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

   if( pFgRegs->cntrl_reg.bv.isRunning )
   {
      if( pFgRegs->cntrl_reg.bv.dataRequest )
         makeStart( channel );
      sendRefillSignalIfThreshold( channel );
      feedAdacFg( pFgRegs );
      g_aFgChannels[channel].param_sent++;
   }
   else
   {
      makeStop( channel );
   }
}

/*================================== EOF ====================================*/
