/*!
 *  @file daq_administration.hpp
 *  @brief DAQ administration
 *
 *  @date 04.03.2019
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
#ifndef _DAQ_ADMINISTRATION_HPP
#define _DAQ_ADMINISTRATION_HPP

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <list>
#include <daq_interface.hpp>

namespace daq
{
class DaqAdministration;
class DaqDevice;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Object of this type represents a single DAQ channel.
 */
class DaqChannel
{
   friend class DaqDevice;
   friend class DaqAdministration;

   unsigned int m_number;
   DaqDevice*   m_pParent;

public:
   constexpr static std::size_t  c_discriptorWordSize = DaqInterface::c_discriptorWordSize;

   DaqChannel( unsigned int number = 0 );
   virtual ~DaqChannel( void );

   const unsigned int getNumber( void ) const
   {
      return m_number;
   }

   DaqDevice* getParent( void )
   {
      if( m_pParent == nullptr )
         throw( DaqException( "Object of DaqChannel is not registered in DaqDevice!" ) );
      SCU_ASSERT( m_pParent != nullptr );
      return m_pParent;
   }

   const unsigned int getSlot( void );
   const unsigned int getDeviceNumber( void );

   int sendEnablePostMortem( const bool restart = false );
   int sendEnableHighResolution( void );
   int sendEnableContineous( const DAQ_SAMPLE_RATE_T sampleRate,
                             const unsigned int maxBlocks = 0 );
   int sendDisableContinue( void );
   int sendDisablePmHires( const bool restart = false );

   int sendTriggerCondition( const uint32_t trgCondition );
   uint32_t receiveTriggerCondition( void );

   int sendTriggerDelay( const uint16_t delay );
   uint16_t receiveTriggerDelay( void );

   int sendTriggerMode( bool mode );
   bool receiveTriggerMode( void );

   int sendTriggerSourceContinue( bool extInput );
   bool receiveTriggerSourceContinue( void );

protected:
   virtual bool onDataBlock( DAQ_DATA_T* pData, std::size_t wordLen ) = 0;

};

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Object of this type represents one of the 12 possible DAQ devices
 *        on the SCU bus. It's the container of the DAQ channels.
 */
class DaqDevice
{
   friend class DaqAdministration;

   unsigned int       m_deviceNumber;
   unsigned int       m_slot;
   unsigned int       m_maxChannels;
   DaqAdministration* m_pParent;

protected:
   typedef std::list<DaqChannel*>  CHANNEL_LIST_T;
   CHANNEL_LIST_T     m_channelPtrList;

public:
   DaqDevice( unsigned int slot = 0 );
   virtual ~DaqDevice( void );

   const CHANNEL_LIST_T::iterator begin( void )
   {
      return m_channelPtrList.begin();
   }

   const CHANNEL_LIST_T::iterator end( void )
   {
      return m_channelPtrList.end();
   }

   const unsigned int getDeviceNumber( void ) const
   {
      return m_deviceNumber;
   }

   const unsigned int getSlot( void ) const
   {
      return m_slot;
   }

   const unsigned int getMaxChannels( void ) const
   {
      return m_maxChannels;
   }

   DaqAdministration* getParent( void )
   {
      if( m_pParent == nullptr )
         throw( DaqException( "Object of DaqDevice is not"
                              " registered in DaqAdministration!" ) );
      SCU_ASSERT( m_pParent != nullptr );
      return m_pParent;
   }

   unsigned int readMacroVersion( void );

   bool registerChannel( DaqChannel* pChannel );

   bool unregisterChannel( DaqChannel* pChannel );

   int sendEnablePostMortem( const unsigned int channel,
                             const bool restart = false );
   int sendEnableHighResolution( const unsigned int channel );
   int sendEnableContineous( const unsigned int channel,
                             const DAQ_SAMPLE_RATE_T sampleRate,
                             const unsigned int maxBlocks = 0 );
   int sendDisableContinue( const unsigned int channel );
   int sendDisablePmHires( const unsigned int channel,
                           const bool restart = false );

   int sendTriggerCondition( const unsigned int channel,
                             const uint32_t trgCondition );
   uint32_t receiveTriggerCondition( const unsigned int channel );

   int sendTriggerDelay( const unsigned int channel,
                         const uint16_t delay );
   uint16_t receiveTriggerDelay( const unsigned int channel );

   int sendTriggerMode( const unsigned int channel, bool mode );
   bool receiveTriggerMode( const unsigned int channel );

   int sendTriggerSourceContinue( const unsigned int channel, bool extInput );
   bool receiveTriggerSourceContinue( const unsigned int channel );

   DaqChannel* getChannel( const unsigned int number );

};

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Object of this type represents the container of all possible
 *        DAQ slaves on the SCU bus
 */
class DaqAdministration: public DaqInterface
{
   unsigned int   m_maxChannels;
   boost::thread* m_pThread;
   bool           m_finalizeThread;
   boost::mutex   m_oMutex;
   static std::exception_ptr c_exceptionPtr;

protected:
   typedef std::list<DaqDevice*> DEVICE_LIST_T;
   DEVICE_LIST_T  m_devicePtrList;

public:
   DaqAdministration( const std::string = DAQ_DEFAULT_WB_DEVICE );
   virtual ~DaqAdministration( void );

   const DEVICE_LIST_T::iterator begin( void )
   {
      return m_devicePtrList.begin();
   }

   const DEVICE_LIST_T::iterator end( void )
   {
      return m_devicePtrList.end();
   }

   unsigned int getMaxChannels( void ) const
   {
      return m_maxChannels;
   }

   unsigned int getMaxDevices( void ) const
   {
      return m_devicePtrList.size();
   }

   bool registerDevice( DaqDevice* pDevice );
   bool unregisterDevice( DaqDevice* pDevice );
   int redistributeSlotNumbers( void );

   DaqDevice* getDeviceByNumber( const unsigned int number );
   DaqDevice* getDeviceBySlot( const unsigned int slot );

   DaqChannel* getChannelByAbsoluteNumber( unsigned int absChannelNumber );
   DaqChannel* getChannelByDeviceNumber( const unsigned int deviceNumber,
                                         const unsigned int channelNumber );
   DaqChannel* getChannelBySlotNumber( const unsigned int slotNumber,
                                       const unsigned int channelNumber );

   int distributeData( void );

   void start( unsigned int toSleep = 100 );
   void stop( void );

protected:
   void lock( void )
   {
      m_oMutex.lock();
   }

   void unlock( void )
   {
      m_oMutex.unlock();
   }

private:
   DaqChannel* getChannelByDescriptor( DAQ_DESCRIPTOR_T& roDescriptor )
   {
      return getChannelBySlotNumber( ::daqDescriptorGetSlot( &roDescriptor ),
                                     ::daqDescriptorGetChannel( &roDescriptor )
                                     + 1 );
   }

   void thread( unsigned int toSleep );
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
inline unsigned int DaqDevice::readMacroVersion( void )
{
   return getParent()->readMacroVersion( m_deviceNumber );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendEnablePostMortem( const unsigned int channel,
                                            const bool restart )
{
   return getParent()->sendEnablePostMortem( m_deviceNumber, channel,
                                             restart );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendEnableHighResolution( const unsigned int channel )
{
   return getParent()->sendEnableHighResolution( m_deviceNumber, channel );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendEnableContineous( const unsigned int channel,
                                            const DAQ_SAMPLE_RATE_T sampleRate,
                                            const unsigned int maxBlocks
                                          )
{
   return getParent()->sendEnableContineous( m_deviceNumber, channel,
                                             sampleRate, maxBlocks );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendDisableContinue( const unsigned int channel )
{
   return getParent()->sendDisableContinue( m_deviceNumber, channel );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendDisablePmHires( const unsigned int channel,
                                          const bool restart )
{
   return getParent()->sendDisablePmHires( m_deviceNumber, channel,
                                           restart  );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendTriggerCondition( const unsigned int channel,
                                           const uint32_t trgCondition )
{
   return getParent()->sendTriggerCondition( m_deviceNumber, channel,
                                            trgCondition );
}

/*! ---------------------------------------------------------------------------
 */
inline uint32_t DaqDevice::receiveTriggerCondition(  const unsigned int channel )
{
   return getParent()->receiveTriggerCondition( m_deviceNumber, channel );
}


/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendTriggerDelay( const unsigned int channel,
                                       const uint16_t delay )
{
   return getParent()->sendTriggerDelay( m_deviceNumber, channel, delay );
}

/*! ---------------------------------------------------------------------------
 */
inline uint16_t DaqDevice::receiveTriggerDelay( const unsigned int channel )
{
   return getParent()->receiveTriggerDelay( m_deviceNumber, channel );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendTriggerMode( const unsigned int channel, bool mode )
{
   return getParent()->sendTriggerMode( m_deviceNumber, channel, mode );
}

/*! ---------------------------------------------------------------------------
 */
inline bool DaqDevice::receiveTriggerMode( const unsigned int channel )
{
   return getParent()->receiveTriggerMode( m_deviceNumber, channel );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendTriggerSourceContinue( const unsigned int channel,
                                         bool extInput )
{
   return getParent()->sendTriggerSourceContinue( m_deviceNumber, channel, extInput );
}

/*! ---------------------------------------------------------------------------
 */
inline bool DaqDevice::receiveTriggerSourceContinue( const unsigned int channel )
{
   return getParent()->receiveTriggerSourceContinue( m_deviceNumber, channel );
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
inline const unsigned int DaqChannel::getSlot( void )
{
   return getParent()->getSlot();
}

/*! ---------------------------------------------------------------------------
 */
inline const unsigned int DaqChannel::getDeviceNumber( void )
{
   return getParent()->getDeviceNumber();
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::sendEnablePostMortem( const bool restart )
{
   return getParent()->sendEnablePostMortem( m_number, restart );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::sendEnableHighResolution( void )
{
   return getParent()->sendEnableHighResolution( m_number );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::sendEnableContineous( const DAQ_SAMPLE_RATE_T sampleRate,
                                             const unsigned int maxBlocks )
{
   return getParent()->sendEnableContineous( m_number, sampleRate, maxBlocks );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::sendDisableContinue( void )
{
   return getParent()->sendDisableContinue( m_number );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::sendDisablePmHires( const bool restart )
{
   return getParent()->sendDisablePmHires( m_number, restart );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::sendTriggerCondition( const uint32_t trgCondition )
{
   return getParent()->sendTriggerCondition( m_number, trgCondition );
}

/*! ---------------------------------------------------------------------------
 */
inline uint32_t DaqChannel::receiveTriggerCondition( void )
{
   return getParent()->receiveTriggerCondition( m_number );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::sendTriggerDelay( const uint16_t delay )
{
   return getParent()->sendTriggerDelay( m_number, delay );
}

/*! ---------------------------------------------------------------------------
 */
inline uint16_t DaqChannel::receiveTriggerDelay( void )
{
   return getParent()->receiveTriggerDelay( m_number );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::sendTriggerMode( bool mode )
{
   return getParent()->sendTriggerMode( m_number, mode );
}

/*! ---------------------------------------------------------------------------
 */
inline bool DaqChannel::receiveTriggerMode( void )
{
   return getParent()->receiveTriggerMode( m_number );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::sendTriggerSourceContinue( bool extInput )
{
   return getParent()->sendTriggerSourceContinue( m_number, extInput );
}

/*! ---------------------------------------------------------------------------
 */
inline bool DaqChannel::receiveTriggerSourceContinue( void )
{
   return getParent()->receiveTriggerSourceContinue( m_number );
}


} //namespace daq

#endif //  ifndef _DAQ_ADMINISTRATION_HPP
//================================== EOF ======================================
