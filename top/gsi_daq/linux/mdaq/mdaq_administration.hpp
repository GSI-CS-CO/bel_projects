/*!
 *  @file mdaq_administration.hpp
 *  @brief MIL-DAQ administration
 *
 *  @date 15.08.2019
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
#ifndef _MDAQ_ADMINISTRATION_HPP
#define _MDAQ_ADMINISTRATION_HPP

#include <list>
#include <mdaq_interface.hpp>
#include <daq_calculations.hpp>
#include <scu_fg_list.hpp>

namespace Scu
{
namespace MiLdaq
{

typedef uint32_t MIL_DAQ_T;

class DaqAdministration;
class DaqDevice;

///////////////////////////////////////////////////////////////////////////////
class DaqCompare
{
   friend class DaqDevice;
   friend class DaqAdministration;

   const uint   m_iterfaceAddress;

   /*!
    * @brief Pointer to the DAQ device object including this channel object.
    */
   DaqDevice*     m_pParent;

   bool           m_setValueInvalid;

public:
   /*!
    * @brief Constructor
    * @see DaqDevice::getMaxChannels
    * @see DaqDevice::registerChannel
    * @param number Desired channel number in the range of 1 to the
    *               return value of DaqDevice::getMaxChannels
    *               (at the moment that is 4).\n
    *               If the parameter 0 so the function
    *               DaqDevice::registerChannel gives this channel-object
    *               the next free number.
    */
   DaqCompare( uint iterfaceAddress );

   /*!
    * @brief Destructor
    */
   virtual ~DaqCompare( void );

   /*!
    * @brief Returns the channel number in the possible range from 1 to
    *        DaqDevice::getMaxChannels.
    */
   const uint getAddress( void ) const
   {
      return m_iterfaceAddress;
   }

   /*!
    * @brief Returns a pointer to the container of type DaqDevice
    *        in which this object has been registered.
    */
   DaqDevice* getParent( void )
   {
      assert( m_pParent != nullptr );
      return m_pParent;
   }

   /*!
    * @brief Returns "true" in the case of gap-reading when the last received
    *        set-value was invalid.
    * @note This function can be used within the the callback function "onData"
    *       to distinguish the actual data item is within a gap or not.
    * @retval false Set value is valid, data-item is outside of the gap.
    * @retval true  Set value is the last valid received set value,
    *               data item is inside of a gap
    */
   bool isSetValueInvalid( void ) const
   {
      return m_setValueInvalid;
   }

protected:
   /*!
    * @brief Callback function becomes invoked for each incoming data item which
    *        belongs to this object.
    * @param wrTimeStampTAI White rabbit time stamp TAI.
    * @param actlValue Actual value from the DAQ.
    * @param setValue Set value from function generator @see isSetValueInvalid
    */
   virtual void onData( uint64_t wrTimeStampTAI, MIL_DAQ_T actlValue,
                                                   MIL_DAQ_T setValue ) = 0;

   /*!
    * @brief Optional callback function becomes invoked once this object
    *        is registered in its container of type DaqDevice and this
    *        container is again registered in the administrator
    *        object of type DaqAdministration.
    */
   virtual void onInit( void ) {}

   /*!
    * @brief Optional callback function becomes invoked by a reset event.
    */
   virtual void onReset( void ) {}
};

///////////////////////////////////////////////////////////////////////////////
class DaqDevice: public DaqBaseDevice
{
   friend class DaqAdministration;
   #define MIL_CHANNEL_LIST_T_BASE std::list
   using CHANNEL_LIST_T = MIL_CHANNEL_LIST_T_BASE<DaqCompare*>;
   CHANNEL_LIST_T     m_channelPtrList;
   const uint         m_location;
   DaqAdministration* m_pParent;

public:
   /*!
    * @brief Constructor
    * @see DaqAdministration::registerDevice
    * @param location Desired slot number in the range of 1 to 12.
    *        If the parameter equal 0 so the function
    *        DaqAdministration::registerDevice will set the slot number of
    *        next unregistered DAQ seen from the left side of the SCU slots.
    */
   DaqDevice( uint location );

   /*!
    * @brief Destructor
    */
   ~DaqDevice( void ) override;

   /*!
    * @brief Returns the iterator to the begin of the pointer list of
    *        registered DAQ channel objects.
    */
   const CHANNEL_LIST_T::iterator begin( void )
   {
      return m_channelPtrList.begin();
   }

   /*!
    * @brief Returns the iterator to the end of the pointer list of
    *        registered DAQ channel objects.
    */
   const CHANNEL_LIST_T::iterator end( void )
   {
      return m_channelPtrList.end();
   }

   /*!
    * @brief Returns true if no channel object registered
    */
   const bool empty( void )
   {
      return m_channelPtrList.empty();
   }

   /*!
    * @brief Returns the slot number of the DAQ-device to which belongs this
    *        channel.
    * @note The counting of the slot numbers begins at 1 ends ends at 12.
    * @note The slot number is <b>not</b> identical with the device number,
    *       except the DAQ device is in slot 1!
    * @see getDeviceNumber
    */
   const unsigned int getLocation( void ) const
   {
      return m_location;
   }

   /*!
    * @brief Returns a pointer to the container of type DaqAdministration
    *        in which this object has been registered.
    */
   DaqAdministration* getParent( void )
   {
      assert( m_pParent != nullptr );
      return m_pParent;
   }

   /*!
    * @ingroup REGISTRATION
    * @brief Registering of a single DAQ-compare object in the
    *        internal list.\n
    *
    * Counterpart of unregisterDaqCompare()
    * @param poCompare Pointer of the DAQ compare object of type DaqCompare.
    * @retval true No registering because object is already registered.
    * @retval false Object successful registered.
    */
   bool registerDaqCompare( DaqCompare* poCompare );

   /*!
    * @ingroup REGISTRATION
    * @brief Removes the given object from the internal list.
    *
    * Counterpart of registerDaqCompare();
    * @retval true No unregistering because object is not registered.
    * @retval false Object successful unregistered.
    */
   bool unregisterDaqCompare( DaqCompare* poCompare );

   /*!
    * @brief Returns the pointer of a registered DAQ compare object which
    *        has the given address.
    * @param address Device address.
    * @retval !=nullptr Pointer to the DAQ compare object.
    * @retval ==nullptr Compare object not registered.
    */
   DaqCompare* getDaqCompare( const uint address );

protected:
   void initAll( void );

   /*!
    * @brief Optional callback function becomes invoked once this
    *        object is registered in its container of base-type
    *        DaqAdministration.
    */
   virtual void onInit( void ) {}

   /*!
    * @brief Optional callback function becomes invoked by a reset event.
    */
   virtual void onReset( void );
};

///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Container of registered MIL-DAQ-devices.
 *
 * Handles the communication between LM32-Firmware for MIL-DAQs
 * and all registered MIL-DAQ devices
 */
class DaqAdministration: public DaqInterface
{
   using USEC_T = daq::USEC_T;
#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
   /*!
    * @brief Keeps the function-pointer for reading out MIL-DAQ-data
    *        form LM32-shared memory or the DDR3-buffer.
    * It becomes invoked by distributeData().
    */
   uint (DaqAdministration::*m_pfPollDaqData)( void );
#endif

   /*!
    * @brief Temporary memory space for all received MIL raw data.
    */
   MIDDLE_BUFFER_T* m_pMiddleBufferMem;
   uint             m_middleBufferSize;
#ifndef __NEW__
   uint             m_lastReadIndex;
#endif
   USEC_T           m_nextReadOutTime;

protected:

   #define MIL_DEVICE_LIST_BASE std::list
   using DEVICE_LIST_T = MIL_DEVICE_LIST_BASE<DaqDevice*>;

   /*!
    * @brief List stores the pointer of the registered devices of type
    *        DaqDevice
    */
   DEVICE_LIST_T  m_devicePtrList;

public:
   DaqAdministration( DaqEb::EtherboneConnection* poEtherbone );
   DaqAdministration( DaqAccess* poEbAccess );
   virtual ~DaqAdministration( void );

   /*!
    * @brief Returns the iterator to the begin of the pointer list of
    *        registered DAQ device objects.
    */
   const DEVICE_LIST_T::iterator begin( void )
   {
      return m_devicePtrList.begin();
   }

   /*!
    * @brief Returns the iterator to the end of the pointer list of
    *        registered DAQ device objects.
    */
   const DEVICE_LIST_T::iterator end( void )
   {
      return m_devicePtrList.end();
   }

   /*!
    * @brief Returns true if no DAQ device object registered
    */
   const bool empty( void )
   {
      return  m_devicePtrList.empty();
   }

   /*!
    * @ingroup REGISTRATION
    * @brief Registering of a DAQ device object object.
    * @param pDevice Pointer to a object of type DaqDevice
    */
   bool registerDevice( DaqDevice* pDevice );

   DaqDevice* getDevice( const uint location );

   /*!
    * @ingroup REGISTRATION
    * @brief Removes the given object from the internal list.
    *
    * Counterpart of registerDevice();
    * @retval true No unregistering because object is not registered.
    * @retval false Object successful unregistered.
    */
   bool unregisterDevice( DaqDevice* pDevice );

   /*!
    * @brief Function reads the ring-buffer and invokes the corresponding
    *        callback function "DaqCompare::onData" if a data item belonging
    *        a registered object of base-class "DaqCompare" has
    *        been recognized.
    *
    * If a unknown data item was read, so the optional callback-function
    * "onUnregistered" becomes invoked.
    *
    * @return Number of unread ring buffer items.
    */
   uint distributeData( void ) override;

   void reset( void );

protected:
   virtual void onUnregistered( const FG_MACRO_T fg ) {}

private:
   DaqCompare* findDaqCompare( const FG_MACRO_T macro );

   void initPtr( void );

#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
   uint distributeDataNew( void );
   uint distributeDataOld( void );
#endif

};


///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Administration of all MIL-DAQs with additional list
 *        of function generators.
 */
class DaqAdministrationFgList: public DaqAdministration
{
protected:
   /*!
    * @brief Object contains the list of all found
    *        function generators.
    */
   FgList             m_oFgList;

public:
   DaqAdministrationFgList( DaqEb::EtherboneConnection* poEtherbone );

   DaqAdministrationFgList( DaqAccess* poEbAccess );

   virtual ~DaqAdministrationFgList( void );

   void scan( void )
   {
      m_oFgList.scan( getEbAccess() );
   }

   void scan( Lm32Swi* poSwi )
   {
      m_oFgList.scan( poSwi );
   }

   void sync( void )
   {
      m_oFgList.sync( getEbAccess() );
   }

   uint getLm32SoftwareVersion( void ) const
   {
      return m_oFgList.getLm32SoftwareVersion();
   }

   /*!
    * @brief Returns true if function generator with
    *        the given socket and given device number present.
    * @note A scan of function generators before assumed!
    */
   bool isPresent( const uint socket, const uint device )
   {
      return m_oFgList.isPresent( socket, device );
   }

   /*!
    * @brief Returns true if the given socket number is used by a
    *        function generator.
    * @note A scan of function generators before assumed!
    */
   bool isSocketUsed( const uint socket )
   {
      return m_oFgList.isSocketUsed( socket );
   }

   FgList& getFgList( void )
   {
      return m_oFgList;
   }
};

} // namespace MiLdaq
} // namespace Scu

#endif // ifndef _MDAQ_ADMINISTRATION_HPP
//================================== EOF ======================================
