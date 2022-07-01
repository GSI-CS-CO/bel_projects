/*!
 *  @file scu_ddr3.h
 *  @brief Interface routines for Double Data Rate (DDR3) RAM in SCU3
 *
 *  @note This module is suitable for Linux and LM32 at the moment.
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
#include <stdbool.h>
#include <helper_macros.h>

#ifdef CONFIG_RTOS
  /*
   * Obtaining the functions ddr3Lock() and ddr3Unlock() in
   * the project configuration file of FreeRTOS.
   */
  #include <FreeRTOS.h>
#endif

#ifdef __cplusplus
extern "C" {
namespace Scu
{
#endif
/*!
 * @defgroup SCU_DDR3
 * @ingroup SCU_RAM_BUFFER
 * @brief DDR3 RAM of SCU3
 * @{
 */

#ifdef CONFIG_DDR_PEDANTIC_CHECK
   /* CAUTION:
    * Assert-macros could be expensive in memory consuming and the
    * latency time can increase as well!
    * Especially in embedded systems with small resources.
    * Therefore use them for bug-fixing or developing purposes only!
    */
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
#define DDR3_MAX_ADDR 0x7FFFFECUL

/*!
 * @brief Maximum of 64 bit oriented access-index.
 * @see ddr3write64 ddr3read64
 */
#define DDR3_MAX_INDEX64 (DDR3_MAX_ADDR / sizeof(DDR3_PAYLOAD_T))

/*!
 * @brief 32 bit oriented offset address of burst mode start-address register
 * @see ddr3StartBurstTransfer
 */
#define DDR3_BURST_START_ADDR_REG_OFFSET 0x1FFFFFD

/*!
 * @brief 32 bit oriented offset address of Xfer_Cnt register.
 * Maximum value is 512 64-bit words.
 * @see ddr3StartBurstTransfer
 */
#define DDR3_BURST_XFER_CNT_REG_OFFSET   0x1FFFFFE

/*!
 * @brief Maximum size of DDR3 Xfer Fifo in 64-bit words
 */
#define DDR3_XFER_FIFO_SIZE  512

/*!
 * @brief 32 bit oriented offset address of fifo status
 */
#define DDR3_FIFO_STATUS_OFFSET_ADDR 0x0E

/*!
 * @see ddr3GetFifoStatus
 */
#define DDR3_FIFO_STATUS_MASK_EMPTY       (1 << 31)

/*!
 * @see ddr3GetFifoStatus
 */
#define DDR3_FIFO_STATUS_MASK_INIT_DONE   (1 << 30)

/*!
 * @see ddr3GetFifoStatus
 */
#define DDR3_FIFO_STATUS_MASK_USED_WORDS  0x1FF

/*!
 * @brief 32 bit oriented offset address of the low data fifo-register
 * @see ddr3PopFifo
 */
#define DDR3_FIFO_LOW_WORD_OFFSET_ADDR  0x0C

/*!
 * @brief 32 bit oriented offset address of the high data fifo-register.
 * @see ddr3PopFifo
 */
#define DDR3_FIFO_HIGH_WORD_OFFSET_ADDR 0x0D

#ifdef __lm32__
   typedef uint32_t* volatile DDR3_ADDR_T;
   typedef void               DDR3_RETURN_T;
   #define DDR3_INVALID       NULL
   #define LM32_VOLATILE      volatile
#else
   typedef uint32_t           DDR3_ADDR_T;
   typedef void        DDR3_RETURN_T;
   #define DDR3_INVALID       0
   #define LM32_VOLATILE
#endif

/*!
 * @brief 64-bit payload base type of the smallest storing unit
 *        of the SCU- DDR3- RAM.
 */
typedef LM32_VOLATILE union
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
   /*! @brief WB Base-address of transparent mode */
   DDR3_ADDR_T pTrModeBase;
#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
   /*! @brief WB Base-address of burst mode */
   DDR3_ADDR_T pBurstModeBase;
#endif
} DDR3_T;

#ifdef CONFIG_RTOS
  #include <lm32Interrupts.h>
  #define ddr3Lock()   criticalSectionEnter()
  #define ddr3Unlock() criticalSectionExit()
#else
 /*
  * Dummy functions when FreeRTOS will not used.
  */
 #define ddr3Lock()
 #define ddr3Unlock()
#endif

/*! --------------------------------------------------------------------------
 */
STATIC inline
unsigned int __ddr3GetEndianIndex16( const unsigned int i )
{
   return (i%2)? (i-1) : (i+1);
}

/*! --------------------------------------------------------------------------
 */
STATIC inline
void ddr3SetPayload16( DDR3_PAYLOAD_T* pPl, const uint16_t d,
                       const unsigned int i )
{
   DDR_ASSERT( i <= ARRAY_SIZE( pPl->ad16 ) );
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
   pPl->ad16[__ddr3GetEndianIndex16(i)] = d;
#else
   pPl->ad16[i] = d;
#endif
}

/*! --------------------------------------------------------------------------
 */
STATIC inline
uint16_t ddr3GetPayload16( DDR3_PAYLOAD_T* pPl, const unsigned int i )
{
   DDR_ASSERT( i <= ARRAY_SIZE( pPl->ad16 ) );
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
   return pPl->ad16[__ddr3GetEndianIndex16(i)];
#else
   return pPl->ad16[i];
#endif
}

#if defined(__lm32__) || defined(__DOXYGEN__)
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
STATIC inline
void ddr3write64( register const  DDR3_T* pThis,
                           const unsigned int index64,
                           const DDR3_PAYLOAD_T* pData )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pTrModeBase != DDR3_INVALID );
   DDR_ASSERT( index64 <= DDR3_MAX_INDEX64 );

   register const unsigned int index32 =
                  index64 * (sizeof(DDR3_PAYLOAD_T)/sizeof(uint32_t));
   ddr3Lock();
   /*
    * CAUTION: Don't change the order of the following both
    * code lines!
    */
   pThis->pTrModeBase[index32+1] = pData->ad32[1]; // DDR3 high word
   pThis->pTrModeBase[index32+0] = pData->ad32[0]; // DDR3 low word

   ddr3Unlock();
}

/*! ---------------------------------------------------------------------------
 * @brief Reads a 64-bit value
 * @see DDR3_PAYLOAD_T
 * @param pThis Pointer to the DDR3 object
 * @param index64 64 bit aligned index
 * @param pData Pointer to the 64-bit-target where the function should
 *              copy the data.
 */
STATIC inline
void ddr3read64( register const DDR3_T* pThis, DDR3_PAYLOAD_T* pData,
                          const unsigned int index64 )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pTrModeBase != DDR3_INVALID );
   DDR_ASSERT( index64 <= DDR3_MAX_INDEX64 );

   register const unsigned int index32 =
                  index64 * (sizeof(DDR3_PAYLOAD_T)/sizeof(uint32_t));
   ddr3Lock();
   /*
    * CAUTION: Don't change the order of the following both
    * code lines!
    */
   pData->ad32[0] = pThis->pTrModeBase[index32+0]; // DDR3 low word
   pData->ad32[1] = pThis->pTrModeBase[index32+1]; // DDR3 high word

   ddr3Unlock();
}
#endif /* ifdef __lm32__ */

#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
#ifdef __lm32__
/*! ---------------------------------------------------------------------------
 * @brief Returns the DDR3-fofo -status;
 * @see DDR3_FIFO_STATUS_MASK_EMPTY
 * @see DDR3_FIFO_STATUS_MASK_INIT_DONE
 * @see DDR3_FIFO_STATUS_MASK_USED_WORDS
 * @param pThis Pointer to the DDR3 object
 * @return Currently fifo status;
 */
STATIC inline volatile
uint32_t ddr3GetFifoStatus( register const DDR3_T* pThis )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pBurstModeBase != DDR3_INVALID );

   ddr3Lock();
   const uint32_t ret = pThis->pBurstModeBase[DDR3_FIFO_STATUS_OFFSET_ADDR];
   ddr3Unlock();

   return ret;
}

/*! ---------------------------------------------------------------------------
 * @brief Rremoves a 64-bit data word from the button of the fifo..
 * @see DDR3_FIFO_LOW_WORD_OFFSET_ADDR
 * @see DDR3_FIFO_HIGH_WORD_OFFSET_ADDR
 * @param pThis Pointer to the DDR3 object
 */
STATIC inline
void ddr3PopFifo( register const DDR3_T* pThis,
                           DDR3_PAYLOAD_T* pData )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pBurstModeBase != DDR3_INVALID );

   ddr3Lock();
   /*
    * CAUTION: Don't change the order of the following both
    * code lines!
    */
   pData->ad32[0] = pThis->pBurstModeBase[DDR3_FIFO_LOW_WORD_OFFSET_ADDR];
   pData->ad32[1] = pThis->pBurstModeBase[DDR3_FIFO_HIGH_WORD_OFFSET_ADDR];

   ddr3Unlock();
}

/*! ---------------------------------------------------------------------------
 * @brief Starts the burst transfer.
 * @param pThis Pointer to the DDR3 object
 * @param burstStartAddr 64 bit oriented start address in fifo
 * @param burstLen 64 bit oriented data-length the value
 *                 has to be between [1..DDR3_XFER_FIFO_SIZE]
 * @see DDR3_XFER_FIFO_SIZE
 */
STATIC inline
void ddr3StartBurstTransfer( register const DDR3_T* pThis,
                                      const unsigned int burstStartAddr,
                                      const unsigned int burstLen )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pTrModeBase != DDR3_INVALID );
   DDR_ASSERT( burstLen <= DDR3_XFER_FIFO_SIZE );

   ddr3Lock();
   /*
    * CAUTION: Don't change the order of the following both
    * code lines!
    */
   pThis->pTrModeBase[DDR3_BURST_START_ADDR_REG_OFFSET] = burstStartAddr;
   pThis->pTrModeBase[DDR3_BURST_XFER_CNT_REG_OFFSET]   = burstLen;

   ddr3Unlock();
}
#endif /* ifdef __lm32__ */
/*! ---------------------------------------------------------------------------
 * @brief Pointer type of the optional polling-function for
 *        the argument "poll" of the function ddr3FlushFiFo.
 *
 * This callback function can be used e.g.: to implement a timeout function
 * or in the case when using a OS to invoke a scheduling function.
 * @see ddr3FlushFiFo
 * @param pThis Pointer to the DDR3 object
 * @param count Number of subsequent calls of this function. E.g.: The
 *              condition (count == 0) can be used to initialize
 *              a timer.
 * @retval >0   Polling loop will terminated despite of the wrong FiFo-status.
 * @retval ==0  Polling will continue till the FiFo contains at least one
 *              data-word.
 * @retval <0   Function ddr3FlushFiFo will terminated immediately with the
 *              return-value of this function.
 */
typedef int (*DDR3_POLL_FT)( register const DDR3_T* pThis UNUSED,
                             unsigned int count UNUSED );

/*! ---------------------------------------------------------------------------
 * @brief Flushes the DDR3 Fifo and writes it's content in argument pTarget
 * @param pThis Pointer to the DDR3 object
 * @param start Start-index (64-byte oriented) in fifo.
 * @param word64len Number of 64 bit words to read.
 * @param pTarget Target address. The memory-size where this address points
 *                has to be at least sizeof(uint64_t) resp. 8 bytes.
 * @param poll Optional pointer to a polling function. If not used then
 *             this parameter has to be set to NULL.
 *             @see DDR3_POLL_FT
 * @return Return status of poll-function if used.
 */
int ddr3FlushFiFo( register const DDR3_T* pThis, unsigned int start,
                   unsigned int word64len, DDR3_PAYLOAD_T* pTarget,
                   DDR3_POLL_FT poll
                 );

#endif /* ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS */
/*! @} */ //End of group  SCU_DDR3
#ifdef __cplusplus
}
}
#endif

#endif /* ifndef _SCU_DDR3_H */
/*================================== EOF ====================================*/
