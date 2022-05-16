/*!
 * @file scu_syslog.h
 * @brief Dummy-functions for LM32 logging when CONFIG_USE_LM32LOG is not
 *        defined.
 *
 * @note      Header only!
 * 
 * @see       lm32_syslog.h
 * @see       lm32_syslog_common.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      16.05.2022
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
#ifndef _SCU_SYSLOG_H
#define _SCU_SYSLOG_H

#ifdef CONFIG_USE_LM32LOG
 #include <lm32_syslog.h>
#else
 #define lm32Log( filter, format, ... ) ((void)0)
#endif

#endif
/*================================== EOF ====================================*/
