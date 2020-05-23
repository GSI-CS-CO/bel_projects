/*!
 * @file   lm32signal.h
 * @brief  Very small variant of signal.h for LM32.
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      19.05.2020
 */
#ifndef _LM32SIGNAL_H
#define _LM32SIGNAL_H

#ifndef SIGINT
   #define SIGINT  2   /*!<@brief interrupt */
#endif

#ifndef SIGTRAP
   #define SIGTRAP 5   /*!<@brief trace trap */
#endif

#ifndef SIGFPE
   #define SIGFPE  8   /*!<@brief arithmetic exception */
#endif

#ifndef SIGSEGV
   #define SIGSEGV 11  /*!<@brief segmentation violation */
#endif

#endif /* ifndef _LM32SIGNAL_H */
/*================================== EOF ====================================*/
