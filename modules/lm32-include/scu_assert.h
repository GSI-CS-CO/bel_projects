/*!
 * @brief  Definition of the macro SCU_ASSERT similar to ANSI-macro "assert"
 *
 * This macro is suitable for LatticeMico32 (LM32) applications within
 * the SCU environment and for Linux applications.
 *
 * @file      scu_assert.h
 * @note Header only!
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      05.11.2018
 * @note CAUTION:
 *       Assert-macros could be expensive in memory consuming and the
 *       latency time can increase as well! \n
 *       Especially in embedded systems with small resources. \n
 *       Therefore use them for bug-fixing and/or developing purposes only! \n
 *       You can disable this macros by defining CONFIG_NO_SCU_ASSERT
 *       independently of the compiler-switch NDEBUG. \n
 *       Understand the macros as scaffolding and in a way as a kind of
 *       "active commenting".
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
*/
#ifndef _SCU_ASSERT_H
#define _SCU_ASSERT_H

#if !defined( NDEBUG ) && !defined( CONFIG_NO_SCU_ASSERT )
  #if defined(__lm32__)
    #include <mprintf.h>
    #define __stderr
    #define assertMprintf mprintf
  #elif !defined( assertMprintf )
    #include <stdio.h>
   #ifndef CONFIG_SCU_ASSERT_CONTINUE
     #include <stdlib.h>
   #endif
    #ifndef SCU_ASSERT_OSTREAM
      #define SCU_ASSERT_OSTREAM stderr
    #endif
    #define __stderr SCU_ASSERT_OSTREAM,
    #define assertMprintf fprintf
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined( NDEBUG ) || defined( CONFIG_NO_SCU_ASSERT )
  /* required by ANSI standard */
# define SCU_ASSERT(__e) ((void)0)
#else
# define SCU_ASSERT(__e) ((__e) ? (void)0 : __scu_assert_func(__FILE__, __LINE__, \
                                                              __ASSERT_FUNC, #__e))

# ifndef __ASSERT_FUNC
  /* Use g++'s demangled names in C++.  */
#  if defined __cplusplus && defined __GNUC__
#   define __ASSERT_FUNC __PRETTY_FUNCTION__

  /* C99 requires the use of __func__.  */
#  elif __STDC_VERSION__ >= 199901L
#   define __ASSERT_FUNC __func__

  /* Older versions of gcc don't have __func__ but can use __FUNCTION__.  */
#  elif __GNUC__ >= 2
#   define __ASSERT_FUNC __FUNCTION__

  /* failed to detect __func__ support.  */
#  else
#   define __ASSERT_FUNC ((char *) 0)
#  endif
# endif /* !__ASSERT_FUNC */

#ifndef CONFIG_SCU_ASSERT_NO_COLORED
  #define __ESC_BOLD   "\e[1m"
  #define __ESC_RED    "\e[31m"
  #define __ESC_NORMAL "\e[0m"
#else
  #define __ESC_BOLD
  #define __ESC_RED
  #define __ESC_NORMAL
#endif

static inline void __scu_assert_func( const char* fileName,
                                      int lineNumber,
                                      const char* functionName,
                                      const char* conditionStr )
{
#ifdef __lm32__
   asm volatile ( "wcsr ie, r0" );
#endif
   assertMprintf( __stderr __ESC_BOLD __ESC_RED "Assertion failed in file: \"%s\""
            " line: %d function: \"%s\" condition: \"%s\"\n"
#ifndef CONFIG_SCU_ASSERT_CONTINUE
            "System stopped!\n"
#endif
            __ESC_NORMAL,
            fileName, lineNumber, functionName, conditionStr );
#ifndef CONFIG_SCU_ASSERT_CONTINUE
 #ifdef __lm32__
   while( 1 );
 #else
   abort();
 #endif
#else
 #ifdef __lm32__
   const uint32_t ie = 0x00000001;
   asm volatile ( "wcsr ie, %0"::"r"(ie) );
 #endif
#endif
}

#endif /* !NDEBUG */

#ifdef __cplusplus
}
#endif

#endif // _SCU_ASSERT_H
/*================================== EOF ====================================*/

