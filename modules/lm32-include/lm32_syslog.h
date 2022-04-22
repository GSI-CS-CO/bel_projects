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
#include <scu_mmu_lm32.h>
#include <lm32_syslog_common.h>

#ifdef __cplusplus
extern "C" {
#endif


/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 */
void syslog( uint32_t priority, const char* format, ... );

/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 */
void vsyslog( uint32_t priority, const char* format, va_list ap );

#ifdef __cplusplus
}
#endif
#endif /* ifndef _LM32_SYSLOG_H */
/*================================== EOF ====================================*/
