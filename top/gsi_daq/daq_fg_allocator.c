/*!
 * @file daq_fg_allocator.c
 * @brief Allocation of set- and actual- DAQ-channel for a given
 *        function generator.
 *
 * @note This module is suitable for LM32 and Linux
 *
 * @date 21.10.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
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
#include <daq_fg_allocator.h>

//#include <scu_function_generator.h> //TODO

/*! ---------------------------------------------------------------------------
 */
unsigned int daqGetSetDaqNumberOfFg( const unsigned int fgNum, const DAQ_DEVICE_TYP_T type )
{ //TODO
   return fgNum + 2; //MAX_FG_PER_SLAVE;
}

/*! ---------------------------------------------------------------------------
 */
unsigned int daqGetActualDaqNumberOfFg( const unsigned int fgNum, const DAQ_DEVICE_TYP_T type )
{ //TODO
   return fgNum;
}

/*================================== EOF ====================================*/
