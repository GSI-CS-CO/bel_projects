/*!
 *
 * @brief Testprogram for module wr_time.c
 *
 *
 * @file      wr-test.c
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      08.01.2019
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
#include "mini_sdb.h"
#include "eb_console_helper.h"
#include "wr_time.h"

void main( void )
{
   discoverPeriphery();
   uart_init_hw();

   gotoxy( 0, 0 );
   clrscr();
   mprintf( "White Rabbit time\n");

   WR_PPS_T* pWrt = wrGetPtr();
   if( pWrt == NULL )
   {
      mprintf( ESC_FG_RED "Error could not initialize WR-pointer!\n" ESC_NORMAL );
      return;
   }

#if 1
   TM_T tm;
   typeof(pWrt->tsv.tv_secLo) lastSecs = (typeof(pWrt->tsv.tv_secLo))~0L;
   do
   {
      if( lastSecs != pWrt->tsv.tv_secLo )
      {
         lastSecs = pWrt->tsv.tv_secLo;
         wrTime2tm( pWrt, 0, &tm );
         gotoxy( 1, 2 );
         mprintf( "%d.%d.%d  %02d:%02d:%02d and %d ns",
               tm.tm_mday,
               tm.tm_mon + 1,
               tm.tm_year + 1900,
               tm.tm_hour,
               tm.tm_min,
               tm.tm_sec,
               pWrt->tsv.tv_nsec );
      }
   }
   while( 1 );
#endif
}

