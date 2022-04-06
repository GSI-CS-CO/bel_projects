/*!
 * @file scu_mmu.h
 * @brief Memory Management Unit of SCU
 * 
 * Administration of the shared memory (for SCU3 using DDR3) between 
 * Linux host and LM32 application.
 * 
 * @note This source code is suitable for LM32 and Linux.
 * 
 * @see       scu_mmu.c
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
#ifndef _SCU_MMU_H
#define _SCU_MMU_H

#include <stdint.h>
#include <helper_macros.h>

#ifdef CONFIG_SCU_USE_DDR3
  #include <scu_ddr3.h>
#endif

#ifdef __cplusplus
extern "C" {
namespace Scu
{
namespace mmu
{
#endif

/*!
 * @brief Datatype for memory block identification.
 */
typedef uint16_t       MMU_TAG_T;

/*!
 * @brief datatype for memory offset respectively index for the smallest
 *        addressable memory unit.
 */
typedef unsigned int   MMU_ADDR_T;

#ifdef CONFIG_SCU_USE_DDR3
 /*!
  * @brief Datatype of the smallest addressable unit of the using memory.
  */
 typedef DDR3_PAYLOAD_T RAM_PAYLOAD_T;
#else
 #error Memory type is unknown!
#endif

/*!
 * @brief Return values of the memory management unit. 
 */
typedef enum
{
   /*!
    * @brief Action was successful.
    */
   OK              =  0,

   /*!
    * @brief Wishbone device of RAM not found. 
    */
   MEM_NOT_PRESENT = -1,

   LIST_NOT_FOUND  = -2,

   /*!
    * @brief Memory block not found.
    */
   TAG_NOT_FOUND   = -3,

   /*!
    * @brief Requested memory block already present.
    */
   ALREADY_PRESENT = -4,

   /*!
    * @brief Requested memory block doesn't fit in physical memory.
    */
   OUT_OF_MEM      = -5
} MMU_STATUS_T;

/*!
 * @brief Type of list item of memory partition list
 */
typedef struct PACKED_SIZE
{
   /*!
    * @brief Tag respectively identification (ID) of memory block.
    */
   MMU_TAG_T  tag;

   /*!
    * @brief Access flags of memory block. (rfu).
    */
   uint16_t  flags;

   /*!
    * @brief Index of next item.
    * @note In the case of the last item then it has to be zero.
    */
   uint32_t  iNext;

   /*!
    * @brief Start index of memory block.
    */
   uint32_t  iStart;

   /*!
    * @brief Data size in RAM_PAYLOAD_T units of memory block.
    */
   uint32_t  length;
} MMU_ITEM_T;

STATIC_ASSERT( sizeof( MMU_ITEM_T ) == 2 * sizeof( RAM_PAYLOAD_T ) );

/*!
 * @brief Access adapter for MMU_ITEM_T.
 */
typedef union
{
   MMU_ITEM_T     mmu;
   RAM_PAYLOAD_T  item[sizeof(MMU_ITEM_T)/sizeof(RAM_PAYLOAD_T)];
} MMU_ACCESS_T;

STATIC_ASSERT( sizeof( MMU_ACCESS_T ) == sizeof( MMU_ITEM_T ) );


const char* mmuStatus2String( const MMU_STATUS_T status );

/*! ---------------------------------------------------------------------------
 * @brief Returns "true" when the partition table is present. 
 */
bool mmuIsPresent( void );

/*! ---------------------------------------------------------------------------
 * @brief Returns the number of items of the memory partition table.
 */
unsigned int mmuGetNumberOfBlocks( void );

/*! ---------------------------------------------------------------------------
 * @brief Deletes a possible existing partition table.
 */
void mmuDelete( void );

/*! ---------------------------------------------------------------------------
 * @brief Allocates a memory area in the shared memory.
 * @param tag Unique tag respectively identifier for this memory area which
 *            has to be reserved.
 * @param pStartAddr Points on the value of the start address respectively
 *                   start index in smallest addressable memory items.
 * @param pLen Requested number of items to allocate in in smallest addressable
 *            memory items. 
 * @param create If true then a new memory block will created,
 *               else a existing block will found only.
 * @return @see MMU_STATUS_T
 */
MMU_STATUS_T mmuAlloc( const MMU_TAG_T tag, MMU_ADDR_T* pStartAddr,
                       size_t* pLen, const bool create );


/*! ---------------------------------------------------------------------------
 * @brief Writes the smallest addressable unit of the using memory. 
 *        In the case of SCU3 it's a 64-bit value.
 * @note This function depends on the platform (Linux or LM32), therefore it's
 *       NOT implemented in module scu_mmu.c! They has to be implemented
 *       separately for Linux or LM32.
 * @param index Start offset in addressable memory units.
 * @param pItem Pointer to memory area which has to be write.
 * @param len Number of items to write.
 */
void mmuWrite( MMU_ADDR_T index, const RAM_PAYLOAD_T* pItem, size_t len );

/*! ---------------------------------------------------------------------------
 * @brief Reads the smallest addressable unit of the using memory. 
 *        In the case of SCU3 it's a 64-bit value.
 * @note This function depends on the platform (Linux or LM32), therefore it's
 *       NOT implemented in module scu_mmu.c! They has to be implemented
 *       separately for Linux or LM32.
 * @param index Start offset in addressable memory units.
 * @param pItem Target pointer for the items to read.
 * @param len Number of items to read.
 */
void mmuRead( MMU_ADDR_T index, RAM_PAYLOAD_T* pItem, size_t len  );

#ifdef __cplusplus
} /* namespace mmu */
} /* namespace Scu */
} /* extern "C"    */
#endif

#endif /* ifndef _SCU_MMU_H */
/*================================== EOF ====================================*/
