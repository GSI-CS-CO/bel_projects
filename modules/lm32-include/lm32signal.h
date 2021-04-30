/*!
 * @file   lm32signal.h
 * @brief  Very small variant of signal.h for LM32.
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>m
 * @date      19.05.2020
 * @see       crt0ScuLm32.S
 */
#ifndef _LM32SIGNAL_H
#define _LM32SIGNAL_H

#ifndef SIGINT
   #define SIGINT  2   /*!<@brief Interrupt. */
#endif

#ifndef SIGTRAP
   #define SIGTRAP 5   /*!<@brief Trace trap. */
#endif

#ifndef SIGFPE
   #define SIGFPE  8   /*!<@brief Arithmetic exception eg. division by zero. */
#endif

#ifndef SIGSEGV
   #define SIGSEGV 11  /*!<@brief Segmentation violation. */
#endif

#ifndef STACK_MAGIC
   /*!
    * @brief Magic number for self made stack overflow checking.
    * 
    * It is the value for the global system variable _endram witch becomes
    * initialized in the startup module crt0ScuLm32.S
    * @see crt0ScuLm32.S
    * @see _endram
    */ 
   #define STACK_MAGIC 0xAAAAAAAA
#endif

#ifndef __ASSEMBLER__
   #include <stdint.h>

   /*!
    * @brief Global stack overflow indicator variable, it becomes initialized
    *        by the value STACK_MAGIC in the startup module crt0ScuLm32.S.
    * @see   crt0ScuLm32.S
    * @see   makefile.scu
    * @see   STACK_MAGIC
    * Example:
    * @code
    * #include <lm32signal.h>
    * 
    * ...
    * 
    * // main-loop:
    * while( true )
    * {
    *    if( _endram != STACK_MAGIC )
    *    {
    *      // Stack overflow handling...
    *    }
    *    // Main-loop routines...
    * } 
    * @endcode
    */
   extern volatile uint32_t _endram;
#endif

#endif /* ifndef _LM32SIGNAL_H */
/*================================== EOF ====================================*/
