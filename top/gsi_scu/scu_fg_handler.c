/*!
 * @file scu_fg_handler.c
 * @brief  Module for handling all SCU-BUS function generators
 *         (non MIL function generators)
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
 * @todo Finding the cause why this function works only when macro "__I" is
 *       defined and remove the access by index variant as soon as possible!
 *       This really dirty!!!! I know.... :-(
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
   controlReg.i16 = pThis->cntrl_reg.i16 & ~(0xfc07); // clear freq, step select, fg_running and fg_enabled
   controlReg.i16 |= ((pset.control & 0x38) << 10) | ((pset.control & 0x7) << 10);

#define __I

#ifdef __I
   ((uint16_t volatile *) pThis)[FG_CNTRL]
#else
   pThis->cntrl_reg.i16
#endif
      = controlReg.i16;

#ifdef __I
   ((uint16_t volatile *) pThis)[FG_A]
#else
   pThis->coeff_a_reg
#endif
      = pset.coeff_a;

#ifdef __I
   ((uint16_t volatile *) pThis)[FG_B]
#else
   pThis->coeff_b_reg
#endif
      = pset.coeff_b;

#ifdef __I
   ((uint16_t volatile *) pThis)[FG_SHIFT]
#else
   pThis->shift_reg
#endif
      = (pset.control & 0x3ffc0) >> 6;

#ifdef __I
   ((uint16_t volatile *) pThis)[FG_STARTL]
#else
   pThis->start_l
#endif
      = pset.coeff_c & 0xffff;

#ifdef __I
   ((uint16_t volatile *) pThis)[FG_STARTH]
#else
   pThis->start_h
#endif
      = pset.coeff_c >> BIT_SIZEOF(int16_t);

}
//#pragma GCC pop_options

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Handles a ADAC-FG
 * @param slot SCU-bus slot number respectively slave number.
 * @param fgAddrOffset Relative address offset of the concerning FG-macro
 *                     till now FG1_BASE or FG2_BASE.
 */
STATIC void handleAdacFg( const unsigned int slot,
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

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief task definition of scu_bus_handler
 * called by the scheduler in the main loop
 * decides which action for a scu bus interrupt is suitable
 * @param pThis pointer to the current task object (not used)
 * @see schedule
 */
void scu_bus_handler( register TASK_T* pThis FG_UNUSED )
{
   FG_ASSERT( pThis->pTaskData == NULL );

   if( !has_msg( &g_aMsg_buf[0], SCUBUS ) )
      return;

   const MSI_T m = remove_msg( &g_aMsg_buf[0], SCUBUS );
   if( m.adr != 0x0 )
      return;

   const uint32_t slave_nr = m.msg + 1;
   if( slave_nr > MAX_SCU_SLAVES )
   {
      mprintf( ESC_ERROR"Slave nr %d unknown!\n"ESC_NORMAL, slave_nr );
      return;
   }

   uint16_t* volatile  pIntActive =
     scuBusGetInterruptActiveFlagRegPtr( (const void*)g_pScub_base, slave_nr );
   uint16_t  flagsToReset = 0;

   if( (*pIntActive & POWER_UP_IRQ) != 0 )
   {
      flagsToReset |= POWER_UP_IRQ;
   }

   //int dummy;
   if( (*pIntActive & FG1_IRQ) != 0 )
   {
      handleAdacFg( slave_nr, FG1_BASE );
      flagsToReset |= FG1_IRQ;
   }

   if( (*pIntActive & FG2_IRQ) != 0 )
   {
      handleAdacFg( slave_nr, FG2_BASE );
      flagsToReset |= FG2_IRQ;
   }

#ifdef CONFIG_MIL_FG
   if( (*pIntActive & DREQ) != 0 )
   {
      add_msg( &g_aMsg_buf[0], DEVSIO, m );
      flagsToReset |= DREQ;
   }
#endif
   *pIntActive = flagsToReset;
}


/*================================== EOF ====================================*/
