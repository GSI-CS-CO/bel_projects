/*!
 *  @file daq_exception.hpp
 *  @brief Base-class of exception for all DAQ-applications
 *  @note Header only!
 *  @date 15.08.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *******************************************************************************
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
#ifndef _DAQ_EXCEPTION_HPP
#define _DAQ_EXCEPTION_HPP
#include <scu_control_config.h>
#include <exception>
#include <string>

namespace Scu
{
namespace daq
{
///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Base exception class for all DAQ- exceptions
 * @ingroup DAQ_EXCEPTION
 */
class Exception: public std::exception
{
   const std::string m_message;

public:
   Exception( const std::string& rMsg ):
      m_message( rMsg ) {}

   const char* what( void ) const noexcept override
   {
      return m_message.c_str();
   }
};

} // namespace daq
} // namespace Scu

#endif
//================================== EOF ======================================
