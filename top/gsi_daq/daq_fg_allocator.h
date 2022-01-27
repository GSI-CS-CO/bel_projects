/*!
 * @file daq_fg_allocator.h
 * @brief Allocation of set- and actual- DAQ-channel for a given
 *        function generator.
 *
 * @note This module is suitable for LM32 and Linux
 *
 * <a href="https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/DataAquisitionMacrof%C3%BCrSCUSlaveBaugruppen">
 * Data Aquisition Macro fuer SCU Slave Baugruppen</a>
 *
 * @date 21.10.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @todo Allocation of DIOB devices
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
#ifndef _DAQ_FG_ALLOCATOR_H
#define _DAQ_FG_ALLOCATOR_H

#include <scu_control_config.h>

#ifdef __cplusplus
extern "C" {
namespace Scu
{
namespace daq
{
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_DEVICE
 * @brief Info-type holding the DAQ device type ADDAC, ACU, DOIB or MIL.
 */
typedef enum
{  /*!
    * @brief DAQ- slave device is unknown (yet).
    */
   UNKNOWN = 0,

   /*!
    * @brief DAQ- slave device is a ADDAC-DAQ
    */
   ADDAC = 1,

   /*!
    * @brief DAQ- slave device is a ACU-DAQ
    */
   ACU = 2,

   /*!
    * @brief DAQ- slave device is a DIOB-DAQ
    */
   DIOB = 3

#ifdef CONFIG_MIL_FG
   /*!
    * @brief DAQ- slave device is a MIL-DAQ
    */
   ,MIL = 4
#endif
} DAQ_DEVICE_TYP_T;


/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @brief Function converts the binary enum type of DAQ device-type in a
 *        human readable string.
 * @param type Binary coded device type.
 * @return Human readable DAQ device type.
 */
const char* daqDeviceTypeToString( const DAQ_DEVICE_TYP_T type );


/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @brief Returns the channel number for the set-value of the given function-
 *        generator number.
 * @todo Allocation of DIOB devices
 * @param fgNum ADDAC function generator number 0 or 1
 * @param type Type of SCU-bus slave.
 * @return Feedback DAQ-channel number for set value.
 */
unsigned int daqGetSetDaqNumberOfFg( const unsigned int fgNum,
                                     const DAQ_DEVICE_TYP_T type );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @brief Returns the channel number for the actual-value of the given function-
 *        generator number.
 * @todo Allocation of DIOB devices
 * @param fgNum ADDAC function generator number 0 or 1
 * @param type Type of SCU-bus slave.
 * @return Feedback DAQ-channel number for actual value.
 */
unsigned int daqGetActualDaqNumberOfFg( const unsigned int fgNum,
                                        const DAQ_DEVICE_TYP_T type );

#ifdef __cplusplus
} /* namespace daq */
} /* namespace Scu */
} /* extern "C" */
#endif
#endif /* ifndef _DAQ_FG_ALLOCATOR_H */
/*================================== EOF ====================================*/
