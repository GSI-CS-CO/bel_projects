/*!
 *  @file daqt_onFoundProcess.hpp
 *  @brief Looking whether a concurrent process is already running.
 *
 *  @date 29.08.2019
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
#ifndef _DAQT_ONFOUNDPROCESS_HPP
#define _DAQT_ONFOUNDPROCESS_HPP

#ifndef __cplusplus
 #error This module is for C++ only!
#endif

#include <string>
#include <find_process.h>

namespace Scu
{
namespace daq
{

extern "C" {

int __onFoundProcess( ::OFP_ARG_T* pArg );

} // extern "C"

/*! ---------------------------------------------------------------------------
 * @brief Function checks whether a concurrent process accessing with the same
 *        ethernet-URL is running.
 * @param myProcess Program name of this process
 * @param ebAddress Etherbone address
 * @retval true A concurrent process accessing to the same etherbone address
 *                is running.
 * @retval false No concurrent process accessing to the same etherbone
 *               address is running.
 */
inline bool isConcurrentProcessRunning( std::string myProcess,
                                        std::string& ebAddress )
{
   return (::findProcesses( myProcess.c_str(), __onFoundProcess, &ebAddress,
           static_cast<FPROC_MODE_T>(::FPROC_BASENAME | ::FPROC_RLINK) ) < 0 );
}

} // namespace daq
} // namespace Scu
#endif // ifndef _DAQT_ONFOUNDPROCESS_HPP
//================================== EOF ======================================
