/*!
 *
 * @brief     Testprogram for using shared memory in LM32.
 *
 * @file      shared.h
 * @see       shared.c
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      19.02.2019
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
#ifndef _SHARED_H
#define _SHARED_H

#include <helper_macros.h>
#include "generated/shared_mmap.h"

typedef struct PACKED
{
   uint32_t a;
   uint32_t b;
   uint32_t c;
} IO_T;
STATIC_ASSERT( sizeof(IO_T) <= SHARED_SIZE );

#endif
/*================================== EOF ====================================*/
