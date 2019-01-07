/*!
 *
 * @brief     Testprogram for using shared memory in LM32.
 *
 * @file      shared.c
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      26.11.2018
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
#include <string.h>
#include <stdbool.h>
#include "mini_sdb.h"
#include "aux.h"
#include "../../tools/wb_slaves.h"
#include "eb_console_helper.h"
#include "scu_lm32_macros.h"
#include "lm32_assert.h"
#include "generated/shared_mmap.h"

#include "wr_time.h"

typedef struct
{
   uint32_t a;
   uint32_t b;
   uint32_t c;
} IO_T;
STATIC_ASSERT( sizeof(IO_T) <= SHARED_SIZE );

//extern uint32_t*       _startshared[];
volatile IO_T SHARED io;


void getWishboneTAI()
{
  uint32_t *pPPSGen;   // WB address of PPS_GEN
  uint32_t taiSecs;    // TAI full seconds
  uint32_t taiNsecs;   // TAI nanoseconds part
  struct tm oTime;

  // find Wishbone address of WR PPS GEN
  pPPSGen   = find_device_adr(WR_PPS_GEN_VENDOR, WR_PPS_GEN_PRODUCT);

  taiSecs  = *((uint32_t*)(((uint8_t*)pPPSGen) + WR_PPS_GEN_CNTR_UTCLO));
  taiNsecs = *((uint32_t*)(((uint8_t*)pPPSGen) + WR_PPS_GEN_CNTR_NSEC));

  //print TAI to UART
  mprintf("TAI: %08u.%09u\n", taiSecs, taiNsecs);

  time64_to_tm( taiSecs, 0, &oTime );

  mprintf( "%d.%d.%d  %02d:%02d:%02d\n",
           oTime.tm_mday,
           oTime.tm_mon,
           oTime.tm_year + 1900,
           oTime.tm_hour,
           oTime.tm_min,
           oTime.tm_sec );

} // getWishboneTAI



#define LM32_INTERN_OFFSET 0x80000000

/*! ---------------------------------------------------------------------------
 * @brief Gets the pointer to shared memory for
 *        external perspective from host bridge.
 * @retval !=NULL Base pointer for external access via host
 * @retval ==NULL Error
 */
uint32_t* getEtherBoneBaseAddressToSharedMemory( const uint32_t sharedOffset )
{
   uint32_t idx;
   sdb_location aFoundSdb[10]; //! @todo Check array size!
   sdb_location foundClu;
   unsigned int cpuId = getCpuIdx();

   idx = 0;
   find_device_multi( &foundClu, &idx, 1, GSI, LM32_CB_CLUSTER );
   idx = 0;
   find_device_multi_in_subtree( &foundClu, aFoundSdb, &idx, ARRAY_SIZE(aFoundSdb), GSI, LM32_RAM_USER);
   if(idx < cpuId)
      return NULL;

   LM32_ASSERT( cpuId < ARRAY_SIZE(aFoundSdb) );
   return (uint32_t*)(getSdbAdr(&aFoundSdb[cpuId]) + sharedOffset - LM32_INTERN_OFFSET);
}

void main( void )
{
   discoverPeriphery();
   uart_init_hw();

   gotoxy( 0, 0 );
   clrscr();
   mprintf( "Shared memory test\n");

   getWishboneTAI();

   uint32_t* pCpuRamExternal = getEtherBoneBaseAddressToSharedMemory(SHARED_OFFS);
   if( pCpuRamExternal == NULL )
   {
      mprintf( ESC_FG_RED "ERROR: Could not find external WB address of my own RAM !\n" ESC_NORMAL);
      return;
   }

   mprintf( "intern: &io.a: 0x%x extern: 0x%x\n", &io.a, ((uint32_t)pCpuRamExternal) + offsetof( IO_T, a ) );
   mprintf( "intern: &io.b: 0x%x extern: 0x%x\n", &io.b, ((uint32_t)pCpuRamExternal) + offsetof( IO_T, b ) );
   mprintf( "intern: &io.c: 0x%x extern: 0x%x\n", &io.c, ((uint32_t)pCpuRamExternal) + offsetof( IO_T, c ) );

   while( true )
   {
      io.a = 0;
      while( io.a == 0 );
      mprintf( "io.a: 0x%x\n", io.a );
      mprintf( "io.b: 0x%x\n", io.b );
      mprintf( "io.c: 0x%x\n", io.c );
   }

}

/*================================== EOF ====================================*/
