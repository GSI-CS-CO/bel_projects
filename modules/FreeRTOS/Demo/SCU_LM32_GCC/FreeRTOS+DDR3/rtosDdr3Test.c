/*!
 * @file rtosDdr3Test.c
 * @brief FreeRtos testprogram on SCU using DDR3 RAM
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      14.04.2020
 */
#include "eb_console_helper.h"
#include "mini_sdb.h"
#include "generated/shared_mmap.h"
#include "scu_lm32_macros.h"
#include "shared_memory_helper.h"
#include "FreeRTOS.h"
#include "task.h"

#ifndef CONFIG_RTOS
   #error "This project provides FreeRTOS"
#endif

#define TEST_TASK_PRIORITY    ( tskIDLE_PRIORITY + 1 )

/*! ---------------------------------------------------------------------------
 */
STATIC inline void init( void )
{
   discoverPeriphery(); // mini-sdb: get info on important Wishbone infrastructure
   uart_init_hw();      // init UART, required for printf...
}



STATIC void vTaskMain( void* pvParameters UNUSED )
{
}


STATIC inline BaseType_t initAndStartRTOS( void )
{
   return pdPASS;
}

void main( void )
{
   init();
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR "FreeRTOS DDR3 test\n"
            "Compiler: " COMPILER_VERSION_STRING "\n" );

   const BaseType_t status = initAndStartRTOS();
   mprintf( ESC_ERROR "Error: This point shall never be reached!\n"
                      "Status: %d\n" ESC_NORMAL, status );
   while( true );
}

/*================================== EOF ====================================*/
