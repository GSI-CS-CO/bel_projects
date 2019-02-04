/*!
 *  @file scu_ddr3.h
 *  @brief Interface routines for DDR3 RAM in SCU3
 *
 *  @see scu_ddr3.c
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
#ifndef _SCU_DDR3_H
#define _SCU_DDR3_H
#include <stdint.h>
#include <helper_macros.h>

#ifdef __cplusplus
extern "C" {
#endif
/*!
 * @defgroup SCU_DDR3
 * @brief DDR3 RAM of SCU3
 * @{
 */

#ifdef CONFIG_DDR_PEDANTIC_CHECK
   #include <scu_assert.h>
   #define DDR_ASSERT SCU_ASSERT
#else
   #define DDR_ASSERT(__e) ((void)0)
#endif

/*!
 * @brief Elementary data type of DDR3 within SCU3
 */
//typedef uint64_t ;


typedef uint32_t DDR3_ALIGN_T;

/*!
 * @brief Maximum size of DDR3 RAM in bytes (1GiBit = GB/8) (134 MB)
 *        (134217728 B)
 */
#define DDR3_MAX_SIZE 0x8000000UL

/*!
 * @brief Maximum usable DDR3 address.
 */
#define DDR3_MAX_ADDR 0x7FFFFEC

#define DDR3_MAX_INDEX64 (DDR3_MAX_ADDR / sizeof(DDR3_PAYLOAD_T))

/*!
 * @brief Offset address of burst mode start-address register
 */
#define DDR3_BURST_START_ADDR_REG_OFFSET 0x7FFFFF4

/*!
 * @brief Elementary data type of DDR3 within SCU3
 */
typedef uint64_t DDR3_BASE_T;


typedef union
{
   uint64_t  d64;
   uint32_t  ad32[sizeof(uint64_t)/sizeof(uint32_t)];
   uint16_t  ad16[sizeof(uint64_t)/sizeof(uint16_t)];
   uint8_t   ad8[sizeof(uint64_t)/sizeof(uint8_t)];
} DDR3_PAYLOAD_T;
STATIC_ASSERT( sizeof(DDR3_PAYLOAD_T) == sizeof(uint64_t) );

/*!
 * @brief Object type of SCU internal DDR3 RAM.
 */
typedef struct
{
   /*!
    * @brief WB Base-address of transparent mode
    */
   DDR3_ALIGN_T* volatile pTrModeBase;
   /*!
    * @brief WB Base-address of burst mode
    */
   DDR3_ALIGN_T* volatile pBurstModeBase;
} DDR3_T;

/*! ---------------------------------------------------------------------------
 * @brief Initializing of DDR3 RAM
 * @param pThis Pointer to the DDR3 object
 * @retval 0 Initializing was successful
 * @retval <0 Error
 */
int ddr3init( register DDR3_T* pThis );

/*! ---------------------------------------------------------------------------
 * @brief Writes a 64-bit value in the DDR3 RAM
 * @see DDR3_PAYLOAD_T
 * @param pThis Pointer to the DDR3 object
 * @param index64 64 bit aligned index
 * @param pData Pointer to the 64 bit data to write.
 */
static inline
void ddr3write64( register const  DDR3_T* pThis, const unsigned int index64,
                  const DDR3_PAYLOAD_T* pData )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pTrModeBase != NULL );
   DDR_ASSERT( index64 <= DDR3_MAX_INDEX64 );
   register const unsigned int index32 =
                  index64 * (sizeof(uint64_t)/sizeof(uint32_t));
   /*
    * CAUTION: Don't change the order of the following both
    * code lines!
    */
   pThis->pTrModeBase[index32+1] = pData->ad32[1]; // DDR3 high word
   pThis->pTrModeBase[index32+0] = pData->ad32[0]; // DDR3 low word
}

/*! ---------------------------------------------------------------------------
 * @brief Reads a 64-bit value
 * @see DDR3_PAYLOAD_T
 * @param pThis Pointer to the DDR3 object
 * @param index64 64 bit aligned index
 * @param pData Pointer to the 64-bit-target where the function should copy the data.
 */
static inline
void ddr3read64( register const DDR3_T* pThis, DDR3_PAYLOAD_T* pData,
                 const unsigned int index64 )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pTrModeBase != NULL );
   DDR_ASSERT( index64 <= DDR3_MAX_INDEX64 );
   register const unsigned int index32 =
                  index64 * (sizeof(uint64_t)/sizeof(uint32_t));
   /*
    * CAUTION: Don't change the order of the following both
    * code lines!
    */
   pData->ad32[0] = pThis->pTrModeBase[index32+0]; // DDR3 low word
   pData->ad32[1] = pThis->pTrModeBase[index32+1]; // DDR3 high word
}

/*! @} */ //End of group  SCU_DDR3
#ifdef __cplusplus
}
#endif

#endif /* ifndef _SCU_DDR3_H */
/*================================== EOF ====================================*/
