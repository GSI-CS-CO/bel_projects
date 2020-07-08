/*!
 *  @file scu_bus_defines.h
 *  @brief Some definitions and inline functions of the SCU-Bus for
 *         LM32 and Linux.
 *
 * Outsourced from scu_bus.h
 *
 *  @see scu_bus.h
 *  @see scu_bus.c
 *  @date 04.03.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
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
#ifndef _SCU_BUS_DEFINES_H
#define _SCU_BUS_DEFINES_H

#include <stdint.h>
#include <stdbool.h>
#include <helper_macros.h>

#ifdef CONFIG_SCU_BUS_PEDANTIC_CHECK
   /* Paranoia mode is enabled... ;-) */
   #include <scu_assert.h>
   #define SCUBUS_ASSERT SCU_ASSERT
#else
   #define SCUBUS_ASSERT(__e) ((void)0)
#endif

#ifdef __cplusplus
extern "C" {
namespace Scu
{
#endif

/*!
 * @ingroup SCU_BUS
 * @brief Physical maximum number of SCU-Bus slots
 */
#ifndef MAX_SCU_SLAVES
  #define MAX_SCU_SLAVES    12
#endif

#if (MAX_SCU_SLAVES > 12)
  #error Maximum value of macro MAX_SCU_SLAVES has to be 12 !
#endif
#if (MAX_SCU_SLAVES < 1)
  #error Minimum value of macro MAX_SCU_SLAVES has to be at least 1 !
#endif

/*!
 * @ingroup SCU_BUS
 * @brief First slot of SCU-bus
 */
#define SCUBUS_START_SLOT  1

/*!
 * @ingroup SCU_BUS
 * @brief Flag field for slaves connected in the SCU bus.
 *
 * Each bit reflects a slot in the SCU bus.
 * If a bit equal one so a SCU device is connected at this place. \n
 * E.g.: \n
 * 000000010101 means: Slot 1, 3 and 5 are used.
 * @see MAX_SCU_SLAVES
 */
typedef uint16_t SCUBUS_SLAVE_FLAGS_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( BIT_SIZEOF( SCUBUS_SLAVE_FLAGS_T ) >= MAX_SCU_SLAVES );
#endif


/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Extract a single slave-present-flag from the SCU-slave-flag-present
 *        field
 * @see scuBusFindSpecificSlaves
 * @see scuBusFindAllSlaves
 * @param flags packed slave present flags of all SCU bus slots
 * @param slot Slot number, valid range 1 .. MAX_SCU_SLAVES (12)
 * @retval true slave present
 * @retval false slave not present
 */
STATIC inline
bool scuBusIsSlavePresent( const SCUBUS_SLAVE_FLAGS_T flags,
                                                     const unsigned int slot )
{
   SCUBUS_ASSERT( slot >= SCUBUS_START_SLOT );
   SCUBUS_ASSERT( slot <= MAX_SCU_SLAVES );

   return ((flags & (1 << (slot-SCUBUS_START_SLOT))) != 0);
}

#ifdef __cplusplus
} /* namespace Scu */
} /* extern "C"    */
#endif

#endif /* ifndef _SCU_BUS_DEFINES_H */
/*================================= EOF =====================================*/
