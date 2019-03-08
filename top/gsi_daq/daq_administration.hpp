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

#include <list>
#include <daq_interface.hpp>

namespace daq
{
class DaqAdmin;
class DaqDevice;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Object of this type represents a single DAQ channel.
 */
class DaqChannel
{
   friend class DaqDevice;
   unsigned int m_number;
   DaqDevice*   m_pParent;

public:
   DaqChannel( unsigned int number );
   ~DaqChannel( void );

   const unsigned int getNumber( void ) const
   {
      return m_number;
   }

   DaqDevice* getParent( void )
   {
      SCU_ASSERT( m_pParent != nullptr );
      return m_pParent;
   }

   int enablePostMortem( void );
   int enableHighResolution( void );
   int enableContineous( const DAQ_SAMPLE_RATE_T sampleRate );
   int disable( void );
   int setTriggerCondition( const uint32_t trgCondition );
   int getTriggerCondition( uint32_t& rTrgCondition );

   int setTriggerDelay( const uint16_t delay );
   int getTriggerDelay( uint16_t& rDelay );

};

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Object of this type represents one of the 12 possible DAQ devices
 *        on the SCU bus. It's the container of the DAQ channels.
 */
class DaqDevice
{
   friend class DaqAdmin;
   std::vector<DaqChannel*> m_channelPtrList[DaqInterface::c_maxChannels];
   unsigned int             m_deviceNumber;
   unsigned int             m_slot;
   DaqAdmin*                m_pParent;

public:
   DaqDevice( unsigned int slot );
   ~DaqDevice( void );

   const unsigned int getDeviceNumber( void ) const
   {
      return m_deviceNumber;
   }

   const unsigned int getSlot( void ) const
   {
      return m_slot;
   }

   DaqAdmin* getParent( void )
   {
      SCU_ASSERT( m_pParent != nullptr );
      return m_pParent;
   }

   bool registerChannel( DaqChannel* pChannel );

   int enablePostMortem( const unsigned int channel );
   int enableHighResolution( const unsigned int channel );
   int enableContineous( const unsigned int channel,
                         const DAQ_SAMPLE_RATE_T sampleRate );
   int disable( const unsigned int channel );
   int setTriggerCondition( const unsigned int channel,
                            const uint32_t trgCondition );
   int getTriggerCondition( const unsigned int channel,
                            uint32_t& rTrgCondition );

   int setTriggerDelay( const unsigned int channel,
                        const uint16_t delay );
   int getTriggerDelay( const unsigned int channel,
                        uint16_t& rDelay );

};

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Object of this type represents the container of all possible
 *        DAQ slaves on the SCU bus
 */
class DaqAdmin: public DaqInterface
{
   std::list<DaqDevice*>  m_devicePtrList[DaqInterface::c_maxDevices];

public:
   DaqAdmin( const std::string = DAQ_DEFAULT_WB_DEVICE );
   virtual ~DaqAdmin( void );

   bool registerDevice( DaqDevice* pDevice );
   bool unregisterDevice( DaqDevice* pDevice );
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::enablePostMortem( const unsigned int channel )
{
   return getParent()->enablePostMortem( m_deviceNumber, channel );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::enableHighResolution( const unsigned int channel )
{
   return getParent()->enableHighResolution( m_deviceNumber, channel );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::enableContineous( const unsigned int channel,
                                        const DAQ_SAMPLE_RATE_T sampleRate )
{
   return getParent()->enableContineous( m_deviceNumber, channel, sampleRate );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::disable( const unsigned int channel )
{
   return getParent()->disable( m_deviceNumber, channel );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::setTriggerCondition( const unsigned int channel,
                                           const uint32_t trgCondition )
{
   return getParent()->setTriggerCondition( m_deviceNumber, channel,
                                            trgCondition );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::getTriggerCondition(  const unsigned int channel,
                                            uint32_t& rTrgCondition )
{
   return getParent()->getTriggerCondition( m_deviceNumber, channel,
                                            rTrgCondition );
}


/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::setTriggerDelay( const unsigned int channel,
                                       const uint16_t delay )
{
   return getParent()->setTriggerDelay( m_deviceNumber, channel, delay );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::getTriggerDelay( const unsigned int channel,
                                       uint16_t& rDelay )
{
   return getParent()->getTriggerDelay( m_deviceNumber, channel, rDelay );
}


///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::enablePostMortem( void )
{
   return getParent()->enablePostMortem( m_number );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::enableHighResolution( void )
{
   return getParent()->enableHighResolution( m_number );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::enableContineous( const DAQ_SAMPLE_RATE_T sampleRate )
{
   return getParent()->enableContineous( m_number, sampleRate );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::disable( void )
{
   return getParent()->disable( m_number );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::setTriggerCondition( const uint32_t trgCondition )
{
   return getParent()->setTriggerCondition( m_number, trgCondition );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::getTriggerCondition( uint32_t& rTrgCondition )
{
   return getParent()->getTriggerCondition( m_number, rTrgCondition );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::setTriggerDelay( const uint16_t delay )
{
   return getParent()->setTriggerDelay( m_number, delay );
}

} //namespace daq

#endif //  ifndef _DAQ_ADMINISTRATION_HPP
//================================== EOF ======================================
