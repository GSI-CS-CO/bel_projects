/*!
 *  @file lm32_hexdump.h
 *  @see  lm32_hexdump.c
 *  @brief Generates a hex-dump e.g. for debugging purposes.
 *  @date 16.11.2018
 *  @copyright (C) 2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT AqNY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */
#ifndef _LM32_HEXDUMP_H
#define _LM32_HEXDUMP_H

#include <stdbool.h>
#include <stdint.h>
#include "eb_console_helper.h"
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Generates a hexdump output via mptintf().
 *
 * Example:
 * @code
 * hexdump( 0x804a0000, 48 );
 * @endcode
 * Generates a console output as follows:
 * @code
 * Addr:      0  1  2  3  4  5  6  7 -  8  9  A  B  C  D  E  F |0123456789ABCDEF|
 * 804A0000: DE AD 02 00 00 07 00 00 - 00 37 00 45 05 02 00 00 |.........7.E....|
 * 804A0010: 00 00 30 D4 DE AD DE AD - DE AD DE AD DE AD DE AD |..0.............|
 * 804A0020: 00 00 00 03 DE AD DE AD - DE AD DE AD DE AD DE AD |................|
 * @endcode
 * @param pData Start address
 * @param Number ob bytes to hex-dump.
 * @return Number of printed lines inclusive the headline
 */
int hexdump( const void* pData, int len );

#ifdef __cplusplus
}
#endif

#endif // /_LM32_HEXDUMP_H
/*================================== EOF ====================================*/