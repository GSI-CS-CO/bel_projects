/*!
 *  @file lm32_hexdump.c
 *  @see  lm32_hexdump.h
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
#include <lm32_hexdump.h>

#ifndef __lm32__
   #include <stdio.h>
   #define mprintf printf
#else
   #include <mprintf.h>
#endif

#define BYTES_PER_LINE 16

#ifndef min
 #define min( a, b ) (((a) < (b)) ? (a) : (b))
#endif

/*! ---------------------------------------------------------------------------
 * @brief Prints a single hexdump line.
 */
static inline void _hexdumpLine( const uint8_t* pData, const size_t dataLen )
{
   int i;

#ifdef __lm32__
   mprintf( "%08X: ", pData );
#else
   mprintf( "%08lX: ", (unsigned long)pData );
#endif

   for( i = 0; i < BYTES_PER_LINE; i++ )
   {
      if( (i > 0) && ((i % (BYTES_PER_LINE / 2)) == 0) )
         mprintf( "- " );

      if( i < dataLen )
         mprintf( "%02X ", pData[i] );
      else
         mprintf(  "   " );
   }

   mprintf( "|" );
   for( i = 0; i < BYTES_PER_LINE; i++ )
   {
      if( i < dataLen )
      {
         if( (pData[i] < 0x20) || (pData[i] > 0x7E) )
            mprintf( "." ); /* not printable byte */
         else
            mprintf( "%c", pData[i] );
      }
      else
         mprintf( " " );
   }
   mprintf( "|\n" );
}

#ifdef __lm32__
   #define SPACE "    "
#else
   #define SPACE "        "
#endif

/*! ---------------------------------------------------------------------------
 * @see lm32_hexdump.h
 */
int hexdump( const void* pData, int len )
{
   int lineLen;
   uint8_t* addr;
   int printedLines;

   if( len <= 0 )
      return 0;

   if( pData == NULL )
      return 0;

   mprintf( ESC_BOLD ESC_FG_YELLOW
            "Addr:"SPACE"  0  1  2  3  4  5  6  7 -"
            "  8  9 10 11 12 13 14 15 "
            "|0123456789ABCDEF|\n" ESC_NORMAL );

   printedLines = 1;
   addr = (uint8_t*)pData;
   while( true )
   {
      lineLen = min( len, BYTES_PER_LINE );
      _hexdumpLine( addr, lineLen );
      printedLines++;
      len -= lineLen;
      if( len <= 0 )
         break;
      addr += lineLen;
   }

   return printedLines;
}

/*================================== EOF ====================================*/
