/*!
 *  @file daq_main.h
 *  @brief Main module for daq_control (including main())
 *
 *  @date 27.02.2019
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
#ifndef _DAQ_MAIN_H
#define _DAQ_MAIN_H

#include <daq_command_interface.h>
#include <daq_command_interface_uc.h>
#include <daq.h>
#include <scu_ramBuffer.h>

#ifdef __cplusplus
extern "C" {
#endif



int scanScuBus( DAQ_BUS_T* pDaqDevices );


#ifdef __cplusplus
}
#endif

#endif /* ifndef _DAQ_MAIN_H */
/*================================== EOF ====================================*/
