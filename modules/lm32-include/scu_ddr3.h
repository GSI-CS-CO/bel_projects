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
 * @brief Maximum size of DDR3 RAM in bytes (1GiBit = GB/8) (134 MB)
 *        (134217728 B)
 */
#define DDR3_MAX_SIZE 0x8000000UL

/*!
 * @brief Maximum usable DDR3 address.
 */
#define DDR3_MAX_ADDR 0x7FFFFEC

/*!
 * @brief Elementary data type of DDR3 within SCU3
 */
typedef uint64_t DDR3_PAYLOAD_T;

/*!
 * @brief Object type of SCU internal DDR3 RAM.
 */
typedef struct
{
   /*!
    * @brief WB Base-address of transparent mode
    */
   uint32_t* volatile pTrModeBase;
   /*!
    * @brief WB Base-address of burst mode
    */
   uint32_t* volatile pBurstModeBase;
} DDR3_T;

/*! ---------------------------------------------------------------------------
 * @brief Initializing of DDR3 RAM
 * @param pThis Pointer to the DDR3 object
 * @retval 0 Initializing was successful
 * @retval <0 Error
 */
int ddr3init( register DDR3_T* pThis );

/*! ---------------------------------------------------------------------------
 * @brief Writes a 32 bit value by index addressed DDR-RAM position.
 * @note The index must be dividable by sizeof(uint32_t) that means
 *       dividable by 4.
 * @param pThis Pointer to the DDR3 object
 * @param index Offset address in DDR-RAM (must be dividable by 4)
 * @param value Value to write
 */
static inline
void ddr3write32( register DDR3_T* pThis, unsigned int index, uint32_t value )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pTrModeBase != NULL );
   DDR_ASSERT( (index % sizeof(uint32_t)) == 0 );
   DDR_ASSERT( index <= DDR3_MAX_ADDR );
   pThis->pTrModeBase[index] = value;
}

/*! ---------------------------------------------------------------------------
 * @brief Reads a 32 bit value from the by indexed position of the
 *        DDR RAM.
 * @note The index must be dividable by sizeof(uint32_t) that means
 *       dividable by 4.
 * @param pThis Pointer to the DDR3 object
 * @param index Offset address in DDR-RAM (must be dividable by 4)
 * @return Value from DDR-RAM
 */
static inline
uint32_t ddr3read32( register DDR3_T* pThis, unsigned int index )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pTrModeBase != NULL );
   DDR_ASSERT( (index % sizeof(uint32_t)) == 0 );
   DDR_ASSERT( index <= DDR3_MAX_ADDR );
   return pThis->pTrModeBase[index];
}


/*! @} */ //End of group  SCU_DDR3
#ifdef __cplusplus
}
#endif

#endif /* ifndef _SCU_DDR3_H */
/*================================== EOF ====================================*/
