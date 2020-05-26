/*!
 * @file daq_base_interface.hpp
 * @brief DAQ common base interface for ADDAC-DAQ and MIL-DAQ.
 *
 * @date 26.05.2020
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
#ifndef _DAQ_BASE_INTERFACE_HPP
#define _DAQ_BASE_INTERFACE_HPP

#include <daq_exception.hpp>
#include <daq_eb_ram_buffer.hpp>
#include <scu_bus_defines.h>

#ifndef DAQ_DEFAULT_WB_DEVICE
   #define DAQ_DEFAULT_WB_DEVICE "dev/wbm0"
#endif

namespace Scu
{

class DaqBaseDevice
{
};

///////////////////////////////////////////////////////////////////////////////
class DaqBaseInterface
{
protected:
   using EB_STATUS_T  = eb_status_t;

private:
   daq::EbRamAccess*   m_poEbAccess;
   const bool          m_ebAccessSelfCreated;

public:
   constexpr static uint c_maxSlots  = MAX_SCU_SLAVES;
   constexpr static uint c_startSlot = SCUBUS_START_SLOT;

   DaqBaseInterface( DaqEb::EtherboneConnection* poEtherbone );
   DaqBaseInterface( daq::EbRamAccess* poEbAccess );
   virtual ~DaqBaseInterface( void );

   DaqEb::EtherboneConnection* getEbPtr( void ) const
   {
      return m_poEbAccess->getEbPtr();
   }

   daq::EbRamAccess* getEbAccess( void ) const
   {
      return m_poEbAccess;
   }

   const std::string& getWbDevice( void )
   {
      return m_poEbAccess->getNetAddress();
   }

   const std::string getScuDomainName( void )
   {
      return m_poEbAccess->getScuDomainName();
   }

   virtual uint distributeData( void ) = 0;
};

} // namespace Scu


#endif // ifndef _DAQ_BASE_INTERFACE_HPP
//================================== EOF ======================================
