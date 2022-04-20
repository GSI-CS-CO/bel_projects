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
#ifndef __lm32__
   #include <string.h>
#endif
/*!
 * @brief Identifier for the bigin of the patition list.
 */
const uint32_t MMU_MAGIC = 0xAAFF0055;

/*!
 * @brief Start index of the first partition list item.
 */
const MMU_ADDR_T MMU_LIST_START = 0;


#ifdef CONFIG_DEBUG_MMU
#ifndef __lm32__
   #include <stdio.h>
   #define mprintf printf
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
#else
#define mmuPrintItem( item ) ((void)0)
#endif


/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
const char* mmuStatus2String( const MMU_STATUS_T status )
{
   #define CASE_RETURN( c ) case c: return #c
   switch( status )
   {
      CASE_RETURN( OK );
      CASE_RETURN( MEM_NOT_PRESENT );
      CASE_RETURN( LIST_NOT_FOUND );
      CASE_RETURN( TAG_NOT_FOUND );
      CASE_RETURN( ALREADY_PRESENT );
      CASE_RETURN( OUT_OF_MEM );
   }
   return "unknown";
}

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
bool mmuIsPresent( void )
{
   RAM_PAYLOAD_T probe;

   mmuRead( MMU_LIST_START, &probe, 1 );
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
            ((MMU_ACCESS_T*)pItem)->item,
            MMU_ITEMSIZE );
}

#define _CONFIG_MMU_PATCH

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_MMU
 */
void mmuWriteItem( const MMU_ADDR_T index, const MMU_ITEM_T* pItem )
{
#if !defined( __lm32__ ) && defined( _CONFIG_MMU_PATCH )
   /*
    * TODO: This is a very bad patch! Remove this ASAP!!!!
    */
   const MMU_ITEM_T item =
   {
      .tag    = pItem->tag,
      .flags  = pItem->flags,
      .iNext  = pItem->length,
      .iStart = pItem->iStart,
      .length = pItem->iNext
   };

   MMU_ITEM_T resp;
   do
   {
      mmuWrite( MMU_LIST_START + index,
               ((MMU_ACCESS_T*)&item)->item,
               MMU_ITEMSIZE );

      mmuReadItem( index, &resp );
   }
   while( memcmp( pItem, &resp, sizeof( resp ) ) != 0 );
#else
   mmuWrite( MMU_LIST_START + index,
            ((MMU_ACCESS_T*)pItem)->item,
            MMU_ITEMSIZE );
#endif
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
      mmuReadNextItem( &listItem );
      if( listItem.iNext == 0 )
         break;
      count++;
   }

   return count;
}

/*!
 * @ingroup SCU_MMU
 * @brief Special block type for the beginning of the list only.
 */
typedef struct PACKED_SIZE
{
   uint32_t magicNumber;
   uint32_t iNext;
   uint64_t padding;
} START_BLOCK_T;

STATIC_ASSERT( sizeof( START_BLOCK_T ) == sizeof( MMU_ITEM_T ) );
STATIC_ASSERT( offsetof( START_BLOCK_T, iNext ) == offsetof( MMU_ITEM_T, iNext ) );

/*!
 * @ingroup SCU_MMU
 * @brief Adapter for START_BLOCK_T.
 */
typedef union
{
   START_BLOCK_T startBlock;
   MMU_ITEM_T    item;
} START_BLOCK_ACCESS_T;

STATIC_ASSERT( sizeof( START_BLOCK_ACCESS_T ) == sizeof( MMU_ITEM_T ) );

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
MMU_STATUS_T mmuAlloc( const MMU_TAG_T tag, MMU_ADDR_T* pStartAddr,
                       size_t* pLen, const bool create )
{
   if( !mmuIsPresent() )
   {
      if( !create )
         return LIST_NOT_FOUND;
     /*
      * List has not yet been created.
      * This will made here.
      */
      const START_BLOCK_ACCESS_T access =
      {
         .startBlock.magicNumber = MMU_MAGIC,
         .startBlock.iNext       = 0,
         .startBlock.padding     = 0L
      };
      mmuWriteItem( 0, &access.item );
   }

   /*
    * Climbing to the end of the already allocated area.
    */
   MMU_ITEM_T item = { .iNext = 0 };
   size_t level = 0;
   uint32_t lastNext;
   do
   {
      lastNext = item.iNext;
      mmuReadNextItem( &item );
      mmuPrintItem( &item );
      if( (level != 0) && (item.tag == tag) )
      { /*
         * Memory block was already allocated.
         */
         *pStartAddr = item.iStart;
         *pLen       = item.length;
         return create? ALREADY_PRESENT : OK;
      }
      level += MMU_ITEMSIZE + item.length;
   }
   while( item.iNext != 0 );

   if( !create )
      return TAG_NOT_FOUND;

   /*
    * Checking if enough free memory there.
    */
   if( level + *pLen + MMU_ITEMSIZE >= MMU_MAX_INDEX )
      return OUT_OF_MEM;

   /*
    * Actualizing the last found item.
    */
   item.iNext = level;
   mmuWriteItem( lastNext, &item );
   mmuPrintItem( &item );

   /*
    * Creating the new item.
    */
   item.tag    = tag;
   item.flags  = 0;
   item.iNext  = 0;
   item.iStart = level + MMU_ITEMSIZE;
   item.length = *pLen;
   mmuWriteItem( level, &item );
   mmuPrintItem( &item );
   *pStartAddr = item.iStart;

   return OK;
}


/*================================== EOF ====================================*/
