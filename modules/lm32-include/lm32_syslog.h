/*!
 * @file lm32_syslog.h
 * @brief LM32 version of syslog.
 *
 * @see       lm32_syslog.c
 * @see       lm32_syslog_common.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      22.04.2022
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#ifndef _LM32_SYSLOG_H
#define _LM32_SYSLOG_H

#ifndef __lm32__
 #error This module is for LM32 only!
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <scu_mmu_lm32.h>
#include <lm32_syslog_common.h>

/*!
 * @defgroup LM32_LOG
 * @brief Logging system for LM32 applications which can be monitored by
 *        the linux application lm32-logd.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 * @brief Initializes the LM32 logging system.
 * @see SCU_MMU
 * @param numOfItems Number of log items which shall be reserved in DDR3-RAM.
 * @retval OK Success
 * @retval ALREADY_PRESENT Memory for log items has already be reserved,
 *                         therefore the value of argument numOfItems remains
 *                         without effect. 
 */
MMU_STATUS_T lm32LogInit( unsigned int numOfItems );

/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 * @brief Sends a log message which can be received by the linux application
 *        "lm32-logd".
 *
 * The parameter is similar like like the printf function, except the optional
 * parameters are limited by 4, all further parameters becomes ignored. \n
 * Following parameter types are possible:\n
 * %%s, %%S String. \n
 * %%c Single character. \n
 * %%X Hexadecimal upper case. \n
 * %%x Hexadecimal lower case. \n
 * %%p Pointer (32-bit hexadecimal number with leading zeros).\n
 * %%d, %%i Signed decimal number. \n
 * %%u Unsigned decimal number. \n
 * %%o Octal number. \n
 * %%b Binary number (not ANSI-conform). \n
 *
 * @note CAUTION: The function lm32LogInit has to be invoked successfully
 *                before!
 * @see lm32LogInit
 *
 * @param filter Filter value in the range form 0 to 31 the filter
 *               becomes determined by the option "-f<filter>" of the
 *               linux application "lm32-logd".
 *               @see LOG_FILTER_T
 * @param format Format-string similar like printf().
 */
void lm32Log( const unsigned int filter, const char* format, ... );

/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 * @brief Function becomes invoked by lm32log().
 * @see lm32Log
 */
void vLm32log( const unsigned int filter, const char* format, va_list ap );

#ifdef __cplusplus
}
#endif
#endif /* ifndef _LM32_SYSLOG_H */
/*================================== EOF ====================================*/
