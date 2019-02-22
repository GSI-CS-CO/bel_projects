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

typedef struct PACKED_SIZE
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
   uint16_t a:  4;
   uint16_t b:  8;
   uint16_t c:  4;
#else
   uint16_t c:  4;
   uint16_t b:  8;
   uint16_t a:  4;
#endif
} BF_T;
STATIC_ASSERT( sizeof( BF_T ) == sizeof(uint16_t) );

typedef struct PACKED_SIZE
{
   uint32_t a;
   uint32_t b;
} SUB_T;
STATIC_ASSERT( sizeof( SUB_T ) == 2 * sizeof(uint32_t) );

typedef struct PACKED_SIZE
{
   uint16_t a;
   uint16_t b;
   uint16_t c;
   BF_T     bf;
   SUB_T    sb;
} IO_T;
STATIC_ASSERT( sizeof(IO_T) == (3 * sizeof(uint16_t) + sizeof(BF_T) + sizeof(SUB_T)) );
STATIC_ASSERT( sizeof(IO_T) <= SHARED_SIZE );

#ifndef __lm32__
   #define mprintf printf
#endif

static inline
void printIO( volatile IO_T* pIo )
{
   mprintf( "{\n" );
   mprintf( "   .a = 0x%04x, %d\n", pIo->a, pIo->a );
   mprintf( "   .b = 0x%04x, %d\n", pIo->b, pIo->b );
   mprintf( "   .c = 0x%04x, %d\n", pIo->c, pIo->c );
   mprintf( "   .bf =\n" );
   mprintf( "   {\n" );
   mprintf( "      .a = 0x%02x, %d\n", pIo->bf.a, pIo->bf.a );
   mprintf( "      .b = 0x%02x, %d\n", pIo->bf.b, pIo->bf.b );
   mprintf( "      .c = 0x%02x, %d\n", pIo->bf.c, pIo->bf.c );
   mprintf( "   }\n" );
   mprintf( "   .sb =\n" );
   mprintf( "   {\n" );
   mprintf( "      .a = 0x%02x, %d\n", pIo->sb.a, pIo->sb.a );
   mprintf( "      .b = 0x%02x, %d\n", pIo->sb.b, pIo->sb.b );
   mprintf( "   }\n}\n" );
}



#endif
/*================================== EOF ====================================*/
