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

#include <scu_control_config.h>
#include <daq_exception.hpp>
#include <daq_eb_ram_buffer.hpp>
#include <scu_bus_defines.h>
#include <scu_function_generator.h>
#include <daqt_messages.hpp>
#include <daq_ring_admin.h>
#include <daq_fg_allocator.h>

#ifndef DAQ_DEFAULT_WB_DEVICE
   #define DAQ_DEFAULT_WB_DEVICE "dev/wbm0"
#endif

namespace Scu
{
namespace daq
{
/*! ----------------------------------------------------------------------------
 * @ingroup DEBUG
 * @brief Converts the device type into a string.
 */
const std::string deviceType2String( const DAQ_DEVICE_TYP_T typ );

} // namespace daq

///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Administration of the socket number.
 *
 * This can be slot-number of ADDAC-DAQs or slot-number and/or socket
 * of MIL DAQs
 */
class DaqBaseDevice
{
   /*!
    * @brief Socket number of all device types.
    * @note In the case of non-MIL is the socket-number equal
    *       to the slot number.
    */
   const uint m_socket;

protected:
   /*!
    * @brief Includes the information of the device type:
    *        ADDAC, ACU, DIOB or MIL.
    * @see DAQ_DEVICE_TYP_T
    * @see daq_fg_allocator.h
    */
   daq::DAQ_DEVICE_TYP_T m_deviceTyp;

public:
   DaqBaseDevice( const uint socket )
      :m_socket( socket )
      ,m_deviceTyp( daq::UNKNOWN )
   {
      DEBUG_MESSAGE( "Constructor of base device: socket: " << m_socket  );
   }

   virtual ~DaqBaseDevice( void )
   {
      DEBUG_MESSAGE( "Destructor of base device: socket: " << m_socket  );
   }

   /*!
    * @brief Returns the socket-number this is the constructors argument.
    * @note In the case of non-MIL is the socket-number equal
    *       to the slot number.
    */
   uint getSocket( void ) const
   {
      return m_socket;
   }

   /*!
    * @brief Returns the SCU bus slot number.
    * @note In the case of non-MIL is the socket-number equal
    *       to the slot number.
    */
   uint getSlot( void ) const
   {
      return getFgSlotNumber( m_socket );
   }

   /*!
    * @brief Returns the device type.
    * @note If the object isn't registered yet then this function will return
    *       UNKNOWN, else ADDAC, ACU, DOIB or MIL.
    */
   daq::DAQ_DEVICE_TYP_T getTyp( void ) const
   {
      return m_deviceTyp;
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
/*!----------------------------------------------------------------------------
 * @brief Handles the data access of MIL- and ADDAC DAQ's via
 *        wishbone/etherbone.
 */
class DaqBaseInterface
{
protected:
   using EB_STATUS_T  = eb_status_t;

private:
   daq::EbRamAccess*   m_poEbAccess;
   const bool          m_ebAccessSelfCreated;

protected:
   static constexpr std::size_t c_defaultMaxEbCycleDataLen = 10;
   static constexpr uint        c_defaultBlockReadEbCycleGapTimeUs = 1000;

   std::size_t       m_maxEbCycleDataLen;
   uint              m_blockReadEbCycleGapTimeUs;

public:
   constexpr static uint c_maxSlots  = Bus::MAX_SCU_SLAVES;
   constexpr static uint c_startSlot = Bus::SCUBUS_START_SLOT;

   /*!
    * @brief Constructor variant for object of type DaqEb::EtherboneConnection
    */
   DaqBaseInterface( DaqEb::EtherboneConnection* poEtherbone );

   /*!
    * @brief Constructor variant for object of type daq::EbRamAccess
    */
   DaqBaseInterface( daq::EbRamAccess* poEbAccess );

   /*!
    * @brief Destructor
    */
   virtual ~DaqBaseInterface( void );

   /*!
    * @brief returns a pointer of the object of type DaqEb::EtherboneConnection
    */
   DaqEb::EtherboneConnection* getEbPtr( void ) const
   {
      return m_poEbAccess->getEbPtr();
   }

   /*!
    * @brief returns a pointer to an object of type daq::EbRamAccess
    */
   daq::EbRamAccess* getEbAccess( void ) const
   {
      return m_poEbAccess;
   }

   /*!
    * @brief Returns the wishbone / etherbone device name.
    */
   const std::string& getWbDevice( void )
   {
      return m_poEbAccess->getNetAddress();
   }

   /*!
    * @brief Returns the SCU LAN domain name or the name of the wishbone
    *        device.
    */
   const std::string getScuDomainName( void )
   {
      return m_poEbAccess->getScuDomainName();
   }

   const std::string getEbStatusString( void ) const
   {
      return static_cast<const std::string>("Noch nix");
   }

   void setMaxEbCycleDataLen( const std::size_t len )
   {
      m_maxEbCycleDataLen = len;
   }

   std::size_t getMaxEbCycleDataLen( void ) const
   {
      return m_maxEbCycleDataLen;
   }

   void setBlockReadEbCycleTimeUs( const uint us )
   {
      m_blockReadEbCycleGapTimeUs = us;
   }

   uint getBlockReadEbCycleTimeUs( void ) const
   {
      return m_blockReadEbCycleGapTimeUs;
   }


   /*!
    * @brief Returns the number of items which are currently in the
    *        data buffer.
    * @param update If true the indexes in the LM32 shared memory
    *               will read before.
    */
   virtual RAM_RING_INDEX_T getCurrentRamSize( bool update = true ) = 0;

   /*!
    * @brief Makes the data buffer empty.
    * @param update If true the indexes in the LM32 shared memory
    *               becomes updated.
    */
   virtual void clearBuffer( bool update = true ) = 0;

   /*!
    * @brief Callback function shall be invoked within a polling-loop and looks
    *        whether enough data are present for forwarding to the higher
    *        layers.
    */
   virtual uint distributeData( void ) = 0;

   virtual void reset( void ) = 0;

   /*!
    * @brief Prepares all found DAQ-devices on SCU-bus to synchronizing its
    *        timestamp- counter.
    * @note Dummy-function in the case of MIL-DAQs!
    * @param timeOffset Time in milliseconds in which the timing-ECA with
    *                   the tag  ecaTag has to be emitted.
    * @param ecaTag The ECA-tag of the timing event.
    */
   virtual int sendSyncronizeTimestamps( const uint32_t timeOffset = daq::DEFAULT_SYNC_TIMEOFFSET,
                                         const uint32_t ecaTag = daq::DEFAULT_ECA_SYNC_TAG )
   {
      return 0;
   }

};

} // namespace Scu


#endif // ifndef _DAQ_BASE_INTERFACE_HPP
//================================== EOF ======================================
