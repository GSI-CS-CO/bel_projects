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

namespace Scu
{
namespace MiLdaq
{

typedef uint32_t MIL_DAQ_T;

///////////////////////////////////////////////////////////////////////////////
class DaqCompare
{
   friend class DaqDevice;
   friend class DaqAdministration;

   uint   m_iterfaceAddress;

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
   DaqCompare( uint m_iterfaceAddress = 0 );

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

protected:
   virtual void onData( uint64_t wrTimeStamp, MIL_DAQ_T actualValue,
                                              MIL_DAQ_T referenceValue ) = 0;
};

///////////////////////////////////////////////////////////////////////////////
class DaqDevice
{
   #define MIL_CHANNEL_LIST_T_BASE std::list
   typedef MIL_CHANNEL_LIST_T_BASE<DaqCompare*>  CHANNEL_LIST_T;
   CHANNEL_LIST_T     m_channelPtrList;
   uint               m_slot;

public:
   /*!
    * @brief Constructor
    * @see DaqAdministration::registerDevice
    * @param slot Desired slot number in the range of 1 to 12.
    *        If the parameter equal 0 so the function
    *        DaqAdministration::registerDevice will set the slot number of
    *        next unregistered DAQ seen from the left side of the SCU slots.
    */
   DaqDevice( unsigned int slot = 0 );

   /*!
    * @brief Destructor
    */
   virtual ~DaqDevice( void );

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
   const unsigned int getSlot( void ) const
   {
      return m_slot;
   }

};

///////////////////////////////////////////////////////////////////////////////
class DaqAdministration: public DaqInterface
{
protected:

   #define MIL_DEVICE_LIST_BASE std::list
   typedef MIL_DEVICE_LIST_BASE<DaqDevice*> DEVICE_LIST_T;
   DEVICE_LIST_T  m_devicePtrList;

public:
   DaqAdministration( DaqEb::EtherboneConnection* poEtherbone );
   DaqAdministration( daq::EbRamAccess* poEbAccess );
   ~DaqAdministration( void );

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

   bool unregisterDevice( DaqDevice* pDevice );


};

} // namespace MiLdaq
} // namespace Scu

#endif // ifndef _MDAQ_ADMINISTRATION_HPP
//================================== EOF ======================================
