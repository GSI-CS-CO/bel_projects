/*!
 * @file scu_mmu.c
 * @brief Memory Management Unit of SCU
 * 
 * Administration of the shared memory (for SCU3 using DDR3) between 
 * Linux host and LM32 application.
 * 
 * @note This source code is suitable for LM32 and Linux.
 * 
 * @see       scu_mmu.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      30.03.2022
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
#include <scu_mmu.h>

#include <lm32_hexdump.h> //!!

/*!
 * @brief Identifier for the bigin of the patition list.
 */
const uint32_t MMU_MAGIC = 0xAAFF1155;

/*!
 * @brief Start index of the first partition list item.
 */
const MMU_ADDR_T MMU_LIST_START = 0;

#ifdef CONFIG_SCU_USE_DDR3
const MMU_ADDR_T MMU_MAX_INDEX = DDR3_MAX_INDEX64;
#endif

void mmuPrintItem( const MMU_ITEM_T* pItem )
{
   mprintf( "tag:    0x%04X\n"
            "flags:  0x%04X\n"
            "iNext:  %u\n"
            "iStart: %u\n"
            "length: %u\n",
            pItem->tag,
            pItem->flags,
            pItem->iNext,
            pItem->iStart,
            pItem->length );
}


/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
bool mmuIsPresent( void )
{
   RAM_PAYLOAD_T probe;

   mmuRead( MMU_LIST_START, &probe, 1 );
//hexdump( &probe.ad32[0], sizeof( probe.ad32[0] ) );
   return ( probe.ad32[0] == MMU_MAGIC );
}

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
void mmuDelete( void )
{
   const RAM_PAYLOAD_T probe = { .ad32[0] = ~MMU_MAGIC, .ad32[1] = 0 };
   mmuWrite( MMU_LIST_START, &probe, 1 );
}

/*! ---------------------------------------------------------------------------
 */
void mmuReadItem( const MMU_ADDR_T index, MMU_ITEM_T* pItem )
{
   mmuRead( MMU_LIST_START + index,
            ((MMU_ACCESS_T*)pItem)->item, 1 );
            //sizeof( MMU_ITEM_T ) / sizeof( RAM_PAYLOAD_T ) );
}

/*! ---------------------------------------------------------------------------
 */
void mmuWriteItem( const MMU_ADDR_T index, const MMU_ITEM_T* pItem )
{
   mmuWrite( MMU_LIST_START + index,
            ((MMU_ACCESS_T*)pItem)->item, 1 );
          //  sizeof( MMU_ITEM_T ) / sizeof( RAM_PAYLOAD_T ) );
}

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
unsigned int mmuGetNumberOfBlocks( void )
{
   if( !mmuIsPresent() )
      return 0;

   MMU_ITEM_T listItem = { .iNext = 0 };
   unsigned int count = 0;
   while( true )
   {
      mmuReadItem( listItem.iNext, &listItem );
      if( listItem.iNext == 0 )
         break;
      count++;
   }

   return count;
}

#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
  #define MAGIC_LOW   GET_UPPER_HALF( MMU_MAGIC )
  #define MAGIC_HIGH  GET_LOWER_HALF( MMU_MAGIC )
#else
  #define MAGIC_LOW   GET_LOWER_HALF( MMU_MAGIC )
  #define MAGIC_HIGH  GET_UPPER_HALF( MMU_MAGIC )
#endif

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
MMU_STATUS_T mmuAlloc( MMU_TAG_T tag, MMU_ADDR_T* pStartAddr, size_t len )
{
   if( !mmuIsPresent() )
   {
      const MMU_ITEM_T listItem =
      {
         .tag    = MAGIC_LOW,
         .flags  = MAGIC_HIGH,
         .iNext  = 0,
         .iStart = 0,
         .length = 0
      };
      mprintf( "*\n" );
      mmuWriteItem( 0, &listItem );
      mmuPrintItem( &listItem );

   }
   MMU_ITEM_T li;
mmuReadItem( 0, &li );
mmuPrintItem( &li );
   return OK; //TODO
}

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
MMU_STATUS_T mmuGet( MMU_TAG_T tag, MMU_ADDR_T* pStartAddr, size_t* pLen )
{
   return OK; //TODO
}


/*================================== EOF ====================================*/
