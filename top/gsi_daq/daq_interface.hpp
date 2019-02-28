/*!
 *  @file daq_interface.hpp
 *  @brief DAQ Interface Library for Linux
 *
 *  @date 28.02.2019
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
#ifndef _DAQ_INTERFACE_HPP
#define _DAQ_INTERFACE_HPP

#include <daq_command_interface.h>
#include <scu_ramBuffer.h>
#include <daq_descriptor.h>
#include <eb_object_transfer.h>
#include <string>
#include <exception>

namespace daq
{

class Daq
{
   const std::string        m_wbDevice;
   RAM_SCU_T                m_oScuRam;
   EB_HANDLE_T              m_oEbHandle;
   EB_HANDLE_T*             m_poEbHandle;
   DAQ_SHARED_IO_T          m_oSharedData;

public:
   class Exception
   {
      const std::string m_message;

   public:
      Exception( const std::string msg ):
         m_message( msg ) {}

      const std::string& what( void ) const
      {
         return m_message;
      }
   };

   Daq( const std::string = "dev/wbm0" );

   ~Daq( void );

   const std::string& getWbDevice( void ) const { return m_wbDevice; }

private:
   void ebClose( void );
   void readSharedTotal( void );
   void setCommand( DAQ_OPERATION_CODE_T );
   DAQ_OPERATION_CODE_T getCommand( void );
};

} //namespace daq

#endif //ifndef _DAQ_INTERFACE_HPP
//================================== EOF ======================================
