/*!
 * @file  eb_lm32_helper.h Some helper macros to simplifying the data transfer
 *        between LM32 and Linux host of heterogeneous flat structures via
 *        shared memory.
 *
 * @note Flat objects means: the object doesn't contain members of type
 *       pointer or reverence, but the use of nested sub-structures can be
 *       arbitrary deep.
 *
 * Based on etherbone.h
 *
 * Required library: etherbone
 *
 * @see       etherbone.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      24.06.2019
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
#ifndef _EB_LM32_HELPER_H
#define _EB_LM32_HELPER_H

#ifdef __lm32__
  #error These are helper macros for the communication with LM32 but this \
  is not a part for a LM32 module!
#endif
#ifndef __linux__
 #error These macros are only for Linux targets!
#endif
#include <etherbone.h>
#include <helper_macros.h>


/*!
 * @defgroup EB_HELPER
 * @brief Some helper macros simplifying the
 *        data transfer of flat objects of types struct, union or class
 *        via wishbone/etherbone bus.
 * @{
 */
/*!
 * @brief Global variable contains the base address of the LM32 user RAM.
 */
extern uint32_t g_lm32Base;

/*! ---------------------------------------------------------------------------
 */
#ifndef EB_LM32_BASE
   //#define EB_LM32_BASE 0x100A0000
   #define EB_LM32_BASE g_lm32Base
#endif

/*! ---------------------------------------------------------------------------
 * @brief Base address of the Linux perspective of the shared memory for
 *        the communication between LM32 and Linux.
 * @note The macro SHARED_OFFS are project dependent and
 *       will be defined in the automatically generated header file
 *       "generated/shared_mmap.h". Therefore this file has to be also
 *       registered in the header file include path of the associated Linux
 *       project. For this reason the Linux module depends on the
 *       LM32 module, whereby the LM32-module has to be compiled first.
 */
#define EB_LM32_SHARED_BASE_ADDRESS (EB_LM32_BASE + SHARED_OFFS)

/*! ---------------------------------------------------------------------------
 * @brief Macro calculates the eb/wb address of a member variable of a
 *        transfer object for the communication between LM32 and Linux.
 * @param type Name of the data type including the concerning member.
 * @param member Name of the member variable.
 */
#define EB_LM32_GET_ADDR_OF_MEMBER( type, member )                            \
   (EB_LM32_SHARED_BASE_ADDRESS + offsetof( type, member ))

/*! ---------------------------------------------------------------------------
 * @brief Macro builds a parameter list for the functions eb_cycle_read and
 *        eb_cycle_write respectively etherbone::Cycle::read and
 *        etherbone::Cycle::write.
 * @param type Name of the data type including the concerning member.
 * @param member Name of the member variable.
 */
#define EB_LM32_FOR_MEMBER( type, member )                                    \
   EB_LM32_GET_ADDR_OF_MEMBER( type, member ),                                \
   GET_SIZE_OF_MEMBER( type, member ) | EB_BIG_ENDIAN


/*!@}*/
#endif /* ifndef _EB_LM32_HELPER_H */
/*================================== EOF ====================================*/
