/*!
 *
 * @brief     Definition of the macro LM32_ASSERT similar to ANSI-macro "assert"
 *
 * @file      lm32_assert.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      05.11.2018
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
#ifndef _LM32_ASSERT_H
#define _LM32_ASSERT_H

#ifndef NDEBUG
 #include "mprintf.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined( NDEBUG ) || defined( CONFIG_NO_LM32_ASSERT )
  /* required by ANSI standard */
# define LM32_ASSERT(__e) ((void)0)
#else
# define LM32_ASSERT(__e) ((__e) ? (void)0 : __lm32_assert_func (__FILE__, __LINE__, \
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

#define __ESC_BOLD   "\e[1m"
#define __ESC_RED    "\e[31m"
#define __ESC_NORMAL "\e[0m"

#define __STATIC_LOCAL __attribute__((section(".BSS"))) static

static inline void __lm32_assert_func( const char* fileName,
                                       int lineNumber,
                                       const char* functionName,
                                       const char* conditionStr )
{
   __STATIC_LOCAL char* assertMsg =
            __ESC_BOLD __ESC_RED
            "Assertion failed in file: \"%s\" line: %d function: \"%s\" condition: \"%s\"\n"
            "System stopped!\n" __ESC_NORMAL;
   mprintf( assertMsg, fileName, lineNumber, functionName, conditionStr );
   while( 1 );
}

#endif /* !NDEBUG */

#ifdef __cplusplus
}
#endif

#endif // _LM32_ASSERT_H
/*================================== EOF ====================================*/

