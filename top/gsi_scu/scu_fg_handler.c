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

   const uint16_t slv_int_act_reg = g_pScub_base[OFFS(slave_nr) + SLAVE_INT_ACT];
   uint16_t       slave_acks      = 0;

   if( (slv_int_act_reg & 0x1) != 0 )
   {// powerup interrupt
      slave_acks |= 0x1;
   }

   int dummy;
   if( (slv_int_act_reg & FG1_IRQ) != 0 )
   { //FG irq?
      handleMacros( slave_nr, FG1_BASE, 0, &dummy );
      slave_acks |= FG1_IRQ;
   }

   if( (slv_int_act_reg & FG2_IRQ) != 0 )
   { //FG irq?
      handleMacros( slave_nr, FG2_BASE, 0, &dummy );
      slave_acks |= FG2_IRQ;
   }

   if( (slv_int_act_reg & DREQ) != 0 )
   { //DRQ irq?
      add_msg( &g_aMsg_buf[0], DEVSIO, m );
      slave_acks |= DREQ;
   }

   g_pScub_base[OFFS(slave_nr) + SLAVE_INT_ACT] = slave_acks; // ack all pending irqs
}


/*================================== EOF ====================================*/
