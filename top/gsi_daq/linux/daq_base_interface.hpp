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
#include <daq_access.hpp>
#include <scu_bus_defines.h>
#include <scu_function_generator.h>
#include <daqt_messages.hpp>
#include <daq_ring_admin.h>
#include <daq_fg_allocator.h>
#include <watchdog_poll.hpp>
#include <assert.h>

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
inline const std::string deviceType2String( const DAQ_DEVICE_TYP_T typ )
{
   return daqDeviceTypeToString( typ );
}

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

#define __NEW__

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
   DaqAccess*                   m_poEbAccess;
   const bool                   m_ebAccessSelfCreated;
   RAM_RING_SHARED_INDEXES_T*   m_poRingAdmin;
   uint                         m_lastReadIndex;
   std::size_t                  m_daqBaseOffset;
   Watchdog                     m_oWatchdog;

protected:
   static constexpr std::size_t c_defaultMaxEbCycleDataLen = 10;
   static constexpr uint        c_defaultBlockReadEbCycleGapTimeUs = 1000;

   std::size_t                  m_maxEbCycleDataLen;
   uint                         m_blockReadEbCycleGapTimeUs;

public:
   constexpr static uint c_maxSlots  = Bus::MAX_SCU_SLAVES;
   constexpr static uint c_startSlot = Bus::SCUBUS_START_SLOT;

   /*!
    * @brief Constructor variant for object of type DaqEb::EtherboneConnection
    */
   DaqBaseInterface( DaqEb::EtherboneConnection* poEtherbone, const uint64_t dataTimeout = 0 );

   /*!
    * @brief Constructor variant for object of type DaqAccess
    */
   DaqBaseInterface( DaqAccess* poEbAccess, const uint64_t dataTimeout = 0 );

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
    * @brief returns a pointer to an object of type DaqAccess
    */
   DaqAccess* getEbAccess( void ) const
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
    * @brief Returns the maximum capacity of the ADDAC or MIL DAQ data-buffer
    *        in minimum addressable payload units of the used RAM-type.
    */
   uint getRamCapacity( void )
   {
      assert( dynamic_cast<RAM_RING_SHARED_INDEXES_T*>(m_poRingAdmin) != nullptr );
      return m_poRingAdmin->indexes.capacity;
   }

   /*!
    * @brief Returns the offset in minimum addressable payload units of the
    *        used RAM type.
    */
   uint getRamOffset( void )
   {
      assert( dynamic_cast<RAM_RING_SHARED_INDEXES_T*>(m_poRingAdmin) != nullptr );
      return m_poRingAdmin->indexes.offset;
   }

#ifndef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
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
#endif
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

   /*!
    * @brief The function is called between divided data blocks for
    *        reading MIL and/or ADDAC DAQ data.
    *
    * When this callback function will not overwritten then a
    * default function will used which invokes the POSIX function
    * usleep() by the parameter m_blockReadEbCycleGapTimeUs,
    */
   virtual void onDataReadingPause( void );

#ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
   /*!
    * @brief Gives the LM32 the order to clear the data-buffer once he has enter
    *        the handling routine of this buffer.
    */
   void clearBufferRequest( void )
   {
    #ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
      if( dynamic_cast<RAM_RING_SHARED_INDEXES_T*>(m_poRingAdmin) == nullptr )
         return;
    #else
      assert( dynamic_cast<RAM_RING_SHARED_INDEXES_T*>(m_poRingAdmin) != nullptr );
    #endif
      updateMemAdmin();
      sendWasRead( ramRingSharedGetSize( m_poRingAdmin ) );
   }
#endif

private:
   void checkIntegrity( void );

   void readLM32( eb_user_data_t pData,
                  const std::size_t len,
                  const std::size_t offset = 0,
                  const etherbone::format_t format = EB_DATA8
                )
   {
      assert( m_daqBaseOffset != 0 );
      getEbAccess()->readLM32( pData, len, offset + m_daqBaseOffset, format );
   }

   void writeLM32( const eb_user_data_t pData,
                   const std::size_t len,
                   const std::size_t offset = 0,
                   const etherbone::format_t format = EB_DATA8 )
   {
      assert( m_daqBaseOffset != 0 );
      getEbAccess()->writeLM32( pData, len, offset + m_daqBaseOffset, format );
   }

public:
   void updateMemAdmin( void );

   /*!
    * @brief Returns the currently number of data items which are not read yet
    *         in the DDR3-RAM
    * @note CAUTION: Obtaining valid data so the function updateMemAdmin() has
    *                to be called before!
    */
   uint getCurrentNumberOfData( void )
   {
      return ramRingSharedGetSize( m_poRingAdmin );
   }

protected:
   void initRingAdmin( RAM_RING_SHARED_INDEXES_T* pAdmin, const std::size_t daqBaseOffset  );

   /*!
    * @brief Returns the number of DDR3- memory items which has not been read yet
    *        since the last call of sendWasRead() or program start.
    * @see sendWasRead()
    * @return Number of memory items, which are not copied yet.
    */
   uint getNumberOfNewData( void );

   /*!
    * @brief Sends the number DDR3-items back to the LM32.
    *
    * The LM32 will add this to the read-index once he has enter
    * the handling routine of this buffer.
    */
   void sendWasRead( const uint wasRead );

   /*!
    * @brief Reads the DDR3-RAM.
    * @param pData Target address in which the data elements shall be copied.
    * @param len Number of memory items to copy.
    */
   void readRam( daq::RAM_DAQ_PAYLOAD_T* pData, const std::size_t len )
   {
      assert( dynamic_cast<RAM_RING_SHARED_INDEXES_T*>(m_poRingAdmin) != nullptr );
      getEbAccess()->readRam( pData, len, m_poRingAdmin->indexes );
   }


   /*!
    * @brief Returns the number of data items which has been read by
    *        the last iteration step, but not handled by LM32 yet.
    * @note CAUTION: Obtaining valid data so the function updateMemAdmin() has
    *                to be called before!
    */
   uint getWasRead( void ) const
   {
      assert( dynamic_cast<RAM_RING_SHARED_INDEXES_T*>(m_poRingAdmin) != nullptr );
      return ramRingSharedGetWasRead( m_poRingAdmin );
   }

   /*!
    * @brief Returns the currently read-index of the DDR3-RAM.
    */
   uint getReadIndex( void )
   {
      assert( dynamic_cast<RAM_RING_SHARED_INDEXES_T*>(m_poRingAdmin) != nullptr );
      return m_poRingAdmin->indexes.start;
   }

   /*!
    * @brief Returns the currently write-index of the DDR3-RAM.
    * @note In this layer for debug purposes only.
    */
   uint getWriteIndex( void )
   {
      assert( dynamic_cast<RAM_RING_SHARED_INDEXES_T*>(m_poRingAdmin) != nullptr );
      return m_poRingAdmin->indexes.end;
   }

   /*!
    * @brief Function performs a block reading divided in smaller sub-blocks to
    *        reduce the maximum EB-cycle open time.
    *
    * That makes time gaps for making occasions for other EB-access devices
    * e.g.: SAFTLIB
    */
   void readDaqData( daq::RAM_DAQ_PAYLOAD_T* pData, std::size_t len );

   /*!
    * @brief Callback function becomes invoked when a data timeout
    *        has been detected.
    */
   virtual void onDataTimeout( void ) {}

public:
   /*!
    * @brief Becomes invoked if the received number of data units are not
    *        dividable by the expected number.
    */
   virtual void onDataError( void );
};

} // namespace Scu


#endif // ifndef _DAQ_BASE_INTERFACE_HPP
//================================== EOF ======================================
