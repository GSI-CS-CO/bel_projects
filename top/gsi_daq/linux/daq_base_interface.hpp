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
#include <scu_function_generator.h>

#include <daq_ring_admin.h>


#ifndef DAQ_DEFAULT_WB_DEVICE
   #define DAQ_DEFAULT_WB_DEVICE "dev/wbm0"
#endif

namespace Scu
{

///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Administration of the socket number.
 */
class DaqBaseDevice
{
   const uint m_socket;

public:
   DaqBaseDevice( const uint socket )
      :m_socket( socket )
   {
   }

   virtual ~DaqBaseDevice( void )
   {
   }

   uint getSocket( void ) const
   {
      return m_socket;
   }

   /*!
    * @brief Returns the SCU bus slot number.
    */
   uint getSlot( void ) const
   {
      return getFgSlotNumber( m_socket );
   }

  /*!
   * @brief Returns "true" in the case the device is a ADDAC.
   */
   bool isAddac( void ) const
   {
      return isAddacFg( m_socket );
   }

   /*!
   * @brief Returns "true" in the case the function generator belonging to the
   *        given socket is a MIL function generator connected via SCU-bus
   *        slave.
   */
   bool isMilScuBus( void ) const
   {
      return isMilScuBusFg( m_socket );
   }

   /*!
    * @brief Returns "true" in the case the function generator belonging to the
    *        given socket is connected via MIL extension.
    */
   bool isMilExtention( void ) const
   {
      return isMilExtentionFg( m_socket );
   }

   /*!
    * @brief Returns "true" in the case the function generator is a MIL device.
    */
   bool isMil( void ) const
   {
      return isMilFg( m_socket );
   }
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

   const std::string getEbStatusString( void ) const
   {
      return static_cast<const std::string>("Noch nix");
   }

   /*!
    * @brief Returns the number of items which are currently in the
    *        data buffer.
    * @param update If true the indexes in the LM32 shared memory
    *               will read before.
    */
   virtual daq::RAM_RING_INDEX_T getCurrentRamSize( bool update = true ) = 0;

   /*!
    * @brief Makes the data buffer empty.
    * @param update If true the indexes in the LM32 shared memory
    *               becomes updated.
    */
   virtual void clearBuffer( bool update = true ) = 0;


   virtual uint distributeData( void ) = 0;
};

} // namespace Scu


#endif // ifndef _DAQ_BASE_INTERFACE_HPP
//================================== EOF ======================================
