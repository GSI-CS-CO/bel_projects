/*!
 *  @file scu_ddr3.c
 *  @brief Interface routines for Double Data Rate (DDR3) RAM in SCU3
 *
 *  @see scu_ddr3.h
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/MacroF%C3%BCr1GbitDDR3MT41J64M16LADesSCUCarrierboards">
 *     DDR3 VHDL Macro der SCU3 </a>
 *  @date 01.02.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
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
#include <scu_ddr3.h>
#ifdef __lm32__
#include <mini_sdb.h>
#endif
#include <dbg.h>

/*! ---------------------------------------------------------------------------
 * @see scu_ddr3.h
 */
int ddr3init( register DDR3_T* pThis )
{
   DDR_ASSERT( pThis != NULL );
#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
   pThis->pBurstModeBase = NULL;
#endif
#ifdef __lm32__
   pThis->pTrModeBase = find_device_adr( GSI, WB_DDR3_if1 );
   if( pThis->pTrModeBase == (uint32_t*)ERROR_NOT_FOUND )
   {
      pThis->pTrModeBase = NULL;
      DBPRINT1( "DBG: ERROR: DDR3: Can't find address of WB_DDR3_if1 !\n" );
      return -1;
   }
#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
   pThis->pBurstModeBase = find_device_adr( GSI, WB_DDR3_if2 );
   if( pThis->pBurstModeBase == (uint32_t*)ERROR_NOT_FOUND )
   {
      pThis->pBurstModeBase = NULL;
      pThis->pTrModeBase    = NULL;
      DBPRINT1( "DBG: ERROR: DDR3: Can't find address of WB_DDR3_if2 !\n" );
      return -1;
   }
#endif
#else /* __lm32__ */
#error Nothing for Linux implemented yet!
#endif /* /__lm32__ */
   return 0;
}

#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
/*! ---------------------------------------------------------------------------
 * @see scu_ddr3.h
 */
int ddr3FlushFiFo( register const DDR3_T* pThis, unsigned int start,
                   unsigned int word64len, DDR3_PAYLOAD_T* pTarget,
                   DDR3_POLL_FT poll )
{
   int pollRet = 0;
   DDR_ASSERT( pTarget != NULL );
   DDR_ASSERT( (word64len + start) <= DDR3_MAX_INDEX64 );

   while( word64len > 0 )
   {
      unsigned int blkLen = min( word64len, DDR3_XFER_FIFO_SIZE );
      DBPRINT2( "DBG: blkLen: %d\n", blkLen );
      ddr3StartBurstTransfer( pThis, start, blkLen );
      for( unsigned int i = 0; i < blkLen; i++ )
      {
         unsigned int pollCount = 0;
         while( (ddr3GetFifoStatus( pThis ) & DDR3_FIFO_STATUS_MASK_EMPTY) != 0 )
         {
            if( poll == NULL )
               continue;
            pollRet = poll( pThis, pollCount );
            if( pollRet < 0 )
               return pollRet;
            if( pollRet > 0 )
               break;
            pollCount++;
         }
         ddr3PopFifo( pThis, &pTarget[start] );
         start++;
      }
      word64len -= blkLen;
   }
   DBPRINT2( "DBG: FiFo-status final: 0x%08x\n", ddr3GetFifoStatus( pThis ) );
   return pollRet;
}

#endif /* ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS */

/*================================== EOF ====================================*/
