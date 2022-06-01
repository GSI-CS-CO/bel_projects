/*!
 * @file scu_env.hpp
 * @brief Checks whether the application is running on SCU
 *
 * @note Header only.
 *
 * @date 16.11.2020
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>
 ******************************************************************************
 */
#ifndef _SCU_ENV_HPP
#define _SCU_ENV_HPP

#if 0

#include <daq_exception.hpp>
#include <stdlib.h>
#include <string.h>

namespace Scu
{

/*! ---------------------------------------------------------------------------
 * @brief Checks whether the application is running on SCU or not.
 * @retval true Application is running on SCU.
 * @retval false Application doesn't run on SCU.
 */
bool isRunningOnScu( void )
{
   constexpr const char* HOSTNAME = "HOSTNAME";
   constexpr const char* HOSTNOMEN = "HOSTNOMEN";
   constexpr const char* prefix = "scuxl";
   constexpr uint prefixLen = ::strlen( prefix );

   const char* hostname = ::getenv( HOSTNAME );
   if( hostname == nullptr )
      hostname = ::getenv( HOSTNOMEN );
   if( hostname == nullptr )
   {
      std::string str = "Environment variable: ";
      str += HOSTNAME;
      str += " or ";
      str += HOSTNOMEN;
      str += " not defined!";
      throw daq::Exception( str );
   }

   return static_cast<std::string>(hostname).substr( 0, prefixLen ) == prefix;
}

} // namespace Scu

#else

#include <unistd.h>

namespace Scu
{

/*! ---------------------------------------------------------------------------
 * @brief Checks whether the application is running on SCU or not.
 * @retval true Application is running on SCU.
 * @retval false Application doesn't run on SCU.
 */
inline bool isRunningOnScu( void )
{
   return ::access( "/dev/wbm0", F_OK ) == 0;
}

} // namespace Scu
#endif

#endif // #ifndef _SCU_ENV_HPP
//================================== EOF ======================================
