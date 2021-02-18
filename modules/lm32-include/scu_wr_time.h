/*!
 * @file scu_wr_time.h
 * @brief Wishbone access to the White Rabbit timer
 * @note Header only!
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      05.03.2020
 */
#ifndef _SCU_WR_TIME_H
#define _SCU_WR_TIME_H

#include <mini_sdb.h>
#include <scu_lm32_macros.h>
#include <lm32Interrupts.h>

#ifdef __cplusplus
extern "C" {
#endif


/*! ---------------------------------------------------------------------------
 * @brief Returns the current white rabbit time.
 */
STATIC inline uint64_t getWrSysTime( void )
{
#if (__BYTE_ORDER__ != __ORDER_BIG_ENDIAN__)
   #error Byteorder big-endian is requested for this function!
#endif

#ifdef CONFIG_WR_TIME_NO_CARRY
   return (((uint64_t)pCpuSysTime[0]) << BIT_SIZEOF(uint32_t)) | pCpuSysTime[1];
#else
   uint64_t time;
   do
   {
      time = (((uint64_t)pCpuSysTime[0]) << BIT_SIZEOF(uint32_t)) |
              pCpuSysTime[1];
   }
   while( ((volatile uint32_t*)pCpuSysTime)[0] != ((uint32_t*)&time)[0] );
   return time;
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the current white rabbit time within a atomic section.
 */
STATIC inline uint64_t getWrSysTimeSafe( void )
{
   criticalSectionEnter();
   const uint64_t time = getWrSysTime();
   criticalSectionExit();
   return time;
}

#ifdef __cplusplus
}
#endif
#endif /* ifndef _SCU_WR_TIME_H */
/*================================== EOF ====================================*/
