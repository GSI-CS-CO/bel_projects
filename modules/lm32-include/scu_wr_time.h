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

#ifdef __cplusplus
extern "C" {
#endif

/*! ---------------------------------------------------------------------------
 * @brief Returns the current white rabbit time.
 */
STATIC inline uint64_t getWrSysTime( void )
{
   return (((uint64_t)pCpuSysTime[0]) << BIT_SIZEOF(uint32_t)) |
          (pCpuSysTime[1] & 0xFFFFFFFF);
}

#ifdef __cplusplus
}
#endif
#endif /* ifndef _SCU_WR_TIME_H */
/*================================== EOF ====================================*/
