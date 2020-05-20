/*!
 * @file scu_std_init.c
 * @brief Module makes some standard initializations before the function
 *        "main()" becomes called.
 * @note Provided the startup module crt0ScuLm32.S will used!
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      20.05.2020
 */
#include "mini_sdb.h"
#include "uart.h"

/*! ---------------------------------------------------------------------------
 * @brief Function becomes invoked immediately before the function main() by
 *        the startup module crt0ScuLm32.S.
 * @see crt0ScuLm32.S
 */
void __init( void )
{
   /*
    * Get info on important Wishbone infrastructure by module mini_sdb.
    * Initialization of some global pointers.
    */
   discoverPeriphery();

   /*
    * Initialization of the UART, required for mprintf...
    */
   uart_init_hw();
}

/*================================== EOF ====================================*/
