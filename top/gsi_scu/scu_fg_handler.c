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

STATIC void feedAdacFg( FG_REGISTER_T* pFgRegs )
{
   FG_PARAM_SET_T pset;

   if( !cbRead( &g_shared.fg_buffer[0], &g_shared.fg_regs[0], pFgRegs->cntrl_reg.bv.number, &pset ) )
   {
      hist_addx(HISTORY_XYZ_MODULE, "buffer empty, no parameter sent", pFgRegs->cntrl_reg.bv.number);
      return;
   }
   //TODO.....


   
}

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
#if 1
   int dummy;
   handleMacros( slot, fgAddrOffset, 0, &dummy );
#else
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
      int dummy;
      send_fg_param( slot, fgAddrOffset, pFgRegs->cntrl_reg.i16, &dummy );
   }
   else
   {
      makeStop( channel );
   }
#endif
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
