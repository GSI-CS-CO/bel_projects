/*!
 * @file ecaMsiTest.c
 * @brief Test program for ECA -Interrupts
 *
 *        Prepared test for FreeRTOS outsourced from ecaMsiExample.c (D.Beck)
 *
 * @see https://www-acc.gsi.de/wiki/Timing/TimingSystemHowSoftCPUHandleECAMSIs
 * @copyright 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @date 07.04.2020
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 *
 *  This example demonstrates handling of message-signaled interrupts (MSI)
 *  caused by ECA channel.
 *  ECA is capable to send MSIs on certain conditions such as producing actions
 *  on reception of timing messages.
 *
 *  build: make clean && make TARGET=ecaMsiExample
 *  deploy: scp ecaMsiExample.bin root@scuxl0304.acc:.
 *  load: eb-fwload dev/wbm0 u 0x0 ecaMsiExample.bin
 *  run: eb-reset dev/wbm0 cpureset 0 (assume only one LM32 is instantiated)
 *  debug: eb-console dev/wbm0
 *
 *  To run example:
 *  - set ECA rules for eCPU action channel
 *  saft-ecpu-ctl tr0 -d -c 0x1122334455667788 0xFFFFFFFFFFFFFFFF 0 0x42
 *  - debug firmware output
 *  eb-console dev/wbm0
 *  - inject timing message (invoke on the second terminal)
 *  saft-ctl -p tr0 inject 0x1122334455667788 0x8877887766556642 0
 *
 ******************************************************************************/

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

/* includes specific for bel_projects */
#include "eb_console_helper.h"
#include "scu_msi.h"
#include "eca_queue_type.h"

/* ECA relevant definitions */
#define ECAQMAX           4   // max number of ECA channels in the system

#define MY_ACT_TAG        0x42       // ECA actions tagged for this eCPU

// global variables
volatile uint32_t *pEcaCtl;         // WB address of ECA control
//volatile uint32_t *pEca;            // WB address of ECA event input (discoverPeriphery())
ECA_QUEUE_ITEM_T* pECAQ;           // WB address of ECA queue

/*******************************************************************************
 *
 * Clear pending valid actions
 *
 ******************************************************************************/
void clearActions( void )
{
  uint32_t valCnt;

  *(pEcaCtl + (ECA_CHANNEL_SELECT_RW >> 2)) = ECA_SELECT_LM32_CHANNEL;
  *(pEcaCtl + (ECA_CHANNEL_NUM_SELECT_RW >> 2)) = 0x00;      // set the sub channel index
  valCnt = *(pEcaCtl + (ECA_CHANNEL_VALID_COUNT_GET >> 2));  // get/clear valid count

  if( valCnt != 0 )
  {
    mprintf("pending actions: %d\n", valCnt);
    valCnt = ecaClearQueue( pECAQ, valCnt );                          // pop pending actions
    mprintf("cleared actions: %d\n", valCnt);
  }
}

/*******************************************************************************
 *
 * Configure ECA to send MSI to embedded soft-core LM32:
 * - ECA action channel for LM32 is selected and MSI target address of LM32 is
 *   set in the ECA MSI target register
 *
 * @param[in] enable  Enable or disable ECA MSI
 * @param[in] channel The index of the selected ECA action channel
 *
 ******************************************************************************/
void configureEcaMsi( const bool enable, const uint32_t channel)
{
  if( channel > ECAQMAX )
  {
     mprintf("Bad channel argument: %d\n", channel);
     return;
  }

  clearActions();   // clean ECA queue and channel from pending actions

  *(pEcaCtl + (ECA_CHANNEL_SELECT_RW >> 2)) = channel;            // select channel
  *(pEcaCtl + (ECA_CHANNEL_NUM_SELECT_RW >> 2)) = 0x00;           // set the sub channel index
  *(pEcaCtl + (ECA_CHANNEL_MSI_SET_ENABLE_OWR >> 2)) = 0;         // disable ECA MSI (required to set a target address)
  *(pEcaCtl + (ECA_CHANNEL_MSI_SET_TARGET_OWR >> 2)) = (uint32_t)pMyMsi;  // set MSI destination address as a target address
  *(pEcaCtl + (ECA_CHANNEL_MSI_SET_ENABLE_OWR >> 2)) = (uint32_t)enable;    // enable ECA MSI

  mprintf("MSI path (ECA -> LM32)           : %s\n\tECA channel = %d\n\tdestination = 0x%08x\n",
      enable? "enabled" : "disabled", channel, (uint32_t)pMyMsi);
}

/*! ---------------------------------------------------------------------------
 *
 * @brief Pop pending embedded CPU actions from an ECA queue and handle them
 *
 * @param cnt The number of pending valid actions
 *
 */
STATIC void ecaHandler( const unsigned int cnt )
{
   for( unsigned int i = 0; i < cnt; i++ )
   {
      if( !ecaIsValid( pECAQ ) )
         continue;

      ECA_QUEUE_ITEM_T ecaItem = *pECAQ;

      ecaPop( pECAQ );

      /*
       * here: do something according to action
       */
      if( ecaItem.tag == MY_ACT_TAG )
      {
         mprintf( "ecaHandler: id: 0x%08x%08x\n"
                  "deadline:       0x%08x%08x\n"
                  "param:          0x%08x%08x\n"
                  "flag:           0x%08x\n",
                  ecaItem.eventIdH,  ecaItem.eventIdL,
                  ecaItem.deadlineH, ecaItem.deadlineL,
                  ecaItem.paramH,    ecaItem.paramL,
                  ecaItem.flags
                );
      }
      else
      {
         mprintf( "ecaHandler: unknown tag: %d\n", ecaItem.tag );
      }
   }
} // ecaHandler

/*******************************************************************************
 *
 * Handle pending valid actions
 *
 ******************************************************************************/
STATIC void handleValidActions( void )
{
  uint32_t valCnt;
  *(pEcaCtl + (ECA_CHANNEL_SELECT_RW >> 2)) = ECA_SELECT_LM32_CHANNEL;
  *(pEcaCtl + (ECA_CHANNEL_NUM_SELECT_RW >> 2)) = 0x00;      // set the sub channel index
  valCnt = *(pEcaCtl + (ECA_CHANNEL_VALID_COUNT_GET >> 2));  // read and clear valid counter
  mprintf( "valid:\t%d\n", valCnt );

  if( valCnt != 0 )
    ecaHandler( valCnt );                             // pop pending valid actions
}

/*! ---------------------------------------------------------------------------
 * @brief Handle MSIs sent by ECA
 *
 * If interrupt was caused by a valid action, then MSI has value of (4<<16|num).\n
 * Both ECA action channel and ECA queue connected to that channel must be handled:\n
 * - read and clear the valid counter value of ECA action channel for LM32 and,\n
 * - pop pending actions from ECA queue connected to this action channel
 */
STATIC void onEcaEvent( const unsigned int intNum,
                        const void* pContext UNUSED )
{
   MSI_ITEM_T m;

   irqMsiCopyObjectAndRemove( &m, intNum );

   mprintf("\nMSI:\t0x%08x\n"
             "Adr:\t0x%08x\n"
             "Sel:\t0x%01x\n",
             m.msg,
             m.adr,
             m.sel );

   if( (m.msg & ECA_VALID_ACTION) != 0 ) // valid actions are pending
      handleValidActions();              // ECA MSI handling

}

/*! ---------------------------------------------------------------------------
 */
STATIC void initIrqTable( void )
{
   irqRegisterISR( 0, NULL, onEcaEvent );
   irqEnable();
   mprintf("Init IRQ table is done.\n");
}

/*! ---------------------------------------------------------------------------
 *
 * @brief Initialization
 *
 * - discover WB devices
 * - init UART
 * - detect ECA control unit
 * - detect ECA queues
 */
STATIC void init( void )
{
   discoverPeriphery();    // mini-sdb: get info on important Wishbone infrastructure, such as (this) CPU, flash, ...

   uart_init_hw();         // init UART, required for printf... . To view print message, you may use 'eb-console' from the host

   mprintf( ESC_CLR_SCR ESC_XY( "1", "1" ) "--- Demo for ECA MSI handling ---\n");

   if( pEca != NULL )
     mprintf("ECA event input                  @ 0x%08x\n", (uint32_t) pEca);
   else
   {
     mprintf(ESC_ERROR"Could not find the ECA event input. Exit!\n");
     return;
   }

   mprintf("MSI destination addr for LM32    : 0x%08x\n", (uint32_t)pMyMsi);

   pEcaCtl = find_device_adr(ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID);

   if( pEcaCtl != NULL )
      mprintf("ECA channel control              @ 0x%08x\n", (uint32_t) pEcaCtl);
   else
   {
      mprintf(ESC_ERROR "Could not find the ECA channel control. Exit!\n");
      return;
   }

   pECAQ = ecaGetLM32Queue();
   if( pECAQ != NULL )
   {
       mprintf("ECA queue to LM32 action channel @ 0x%08x\n", (uint32_t) pECAQ);
   }
   else
   {
     mprintf(ESC_ERROR "Could not find the ECA queue connected to eCPU action channel. Exit!\n");
     return;
   }
}

void main( void )
{
  init(); // get own MSI target addr, ECA event input and ECA queue for LM32 channel

  configureEcaMsi( true, ECA_SELECT_LM32_CHANNEL);
  initIrqTable();                 // set up MSI handler

  mprintf("waiting for MSI ...\n");

  /* main loop */
  while( true );

}

/* ================================= EOF ====================================*/
