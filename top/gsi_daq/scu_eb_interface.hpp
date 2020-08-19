/*!
 *  @file scu_eb_interface.hpp
 *  @brief SCU Wishbone/Etherbone Interface
 *
 *  @date 26.03.2019
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
#ifndef _SCU_EB_INTERFACE_HPP
#define _SCU_EB_INTERFACE_HPP


#include <eb_object_transfer.h>
#include <stddef.h>
#include <string>
#include <exception>

#ifndef DAQ_DEFAULT_WB_DEVICE
   #define DAQ_DEFAULT_WB_DEVICE "dev/wbm0"
#endif


namespace scu
{
///////////////////////////////////////////////////////////////////////////////
class EbInterface
{
   EB_HANDLE_T              m_oEbHandle;
   EB_HANDLE_T*             m_poEbHandle;

public:
   typedef eb_status_t          EB_STATUS_T;

   EbInterface( const std::string = DAQ_DEFAULT_WB_DEVICE );
   ~EbInterface( void );

   EB_STATUS_T ebSocketRun( void )
   {
      return ::ebSocketRun( m_poEbHandle );
   }

   void ebClose( void );

   EB_STATUS_T ebReadObjectCycleOpen( EB_CYCLE_OR_CB_ARG_T& rCArg )
   {
      return ::ebObjectReadCycleOpen( m_poEbHandle, &rCArg );
   }

   EB_STATUS_T ebWriteObjectCycleOpen( EB_CYCLE_OW_CB_ARG_T& rCArg )
   {
      return ::ebObjectWriteCycleOpen( m_poEbHandle, &rCArg );
   }

   void ebCycleClose( void )
   {
      ::ebCycleClose( m_poEbHandle );
   }
};

} // namespace scu

#endif // ifndef _SCU_EB_INTERFACE_HPP
//================================== EOF ======================================
