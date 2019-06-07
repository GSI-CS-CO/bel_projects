/*!
 *  @file daq_administration.hpp
 *  @brief DAQ administration
 *
 * <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/DataAquisitionMacrofürSCUSlaveBaugruppen">
 *    Data Aquisition Macro fuer SCU Slave Baugruppen</a>
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
class DaqAdministration;
class DaqDevice;

/*!
 * @defgroup onDataBlock
 * @brief Functions belonging to this group can be invoked only within the
 *        call back function onDataBlock. Otherwise the system will crash or
 *        in the debug version a assertion becomes triggered!
 * @see onDataBlock
 * @todo Throwing a exception when one of these functions will invoked outside
 *       of its scope?
 */
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Object of this type represents a single DAQ channel.
 */
class DaqChannel
{
   friend class DaqDevice;
   friend class DaqAdministration;

   /*!
    * @brief Object type for the administration of the
    *        data block sequence numbers for the continuous and
    *        PostMortem/HighRes mode.
    */
   struct SequenceNumber
   {
      /*!
       * @brief Becomes true when the sequence numbers can be verified.
       */
      bool    m_continued;

      /*!
       * @brief Becomes true, when since the last compare a lost of
       *        data blocks was detected.
       */
      bool    m_blockLost;

      /*!
       * @brief Modulo 256 counter.
       */
      uint8_t m_sequence;

      /*!
       * @brief Becomes incremented with each lost data block.
       */
      unsigned int m_lostCount;

      SequenceNumber( void )
         :m_continued( false )
         ,m_blockLost( false )
         ,m_sequence( 0 )
         ,m_lostCount( 0 )
      {}

      /*!
       * @brief Returns true when a sequence error was detected.
       * @param sequence Sequence number form the actual received
       *                 device descriptor.
       */
      bool compare( uint8_t sequence );

      void reset( void )
      {
         m_continued = false;
         m_blockLost = false;
         m_lostCount = 0;
      }

      bool wasLost( void ) const
      {
         return m_blockLost;
      }

      unsigned int getLostCount( void ) const
      {
         return m_lostCount;
      }
   };

   /*!
    * @brief Channel number.
    */
   unsigned int   m_number;

   /*!
    * @brief Pointer to the DAQ device object including this channel object.
    */
   DaqDevice*     m_pParent;

   SequenceNumber m_oSequenceContinueMode;
   SequenceNumber m_oSequencePmHighResMode;

   /*!
    * @brief Pointer to object of the sequence number object of the last received
    *        block. If no block was received yet than the value is "nullptr".
    */
   SequenceNumber* m_poSequence;

public:
   constexpr static std::size_t  c_discriptorWordSize =
                                  DaqInterface::c_discriptorWordSize;

   DaqChannel( unsigned int number = 0 );
   virtual ~DaqChannel( void );

   const unsigned int getNumber( void ) const
   {
      return m_number;
   }

   DaqDevice* getParent( void )
   {
      if( m_pParent == nullptr )
         throw( DaqException( "Object of DaqChannel is not"
                                               " registered in DaqDevice!" ) );
      SCU_ASSERT( m_pParent != nullptr );
      return m_pParent;
   }
   const std::string& getWbDevice( void );
   const std::string getScuDomainName( void );
   const unsigned int getSlot( void );
   const unsigned int getDeviceNumber( void );

   /*!
    * @brief Enables the Post-Mortem mode.
    * @param restart When true, restarting of the Post-Mortem mode after a
    *                trigger event, else it's a single shoot event.
    * @see sendDisablePmHires
    */
   int sendEnablePostMortem( const bool restart = false );

   /*!
    * @brief Enables the High-Resolution mode
    * @param restart When true, restarting of the High-Resolution mode after a
    *                trigger event, else it's a single shoot event.
    * @see sendDisablePmHires
    */
   int sendEnableHighResolution( const bool restart = false );

   /*!
    * @brief Enables the Continuous-Mode
    * @see sendDisableContinue
    * @see DAQ_SAMPLE_RATE_T
    * @param sampleRate Sample rate: 1ms, 100us or 10us
    * @param maxBlocks Limit of block receiving.
    *        if the number of received blocks equal this parameter,
    *        the Continuous-Mode becomes disabled. The value of zero is
    *        the endless mode.
    */
   int sendEnableContineous( const DAQ_SAMPLE_RATE_T sampleRate,
                             const unsigned int maxBlocks = 0 );

   /*!
    * @brief Disables the continuous mode.
    * @see sendEnableContineous
    */
   int sendDisableContinue( void );

   /*!
    * @brief Disables the Post-Mortem and High-Resolution mode.
    * @see sendEnablePostMortem
    * @see sendEnableHighResolution
    */
   int sendDisablePmHires( const bool restart = false );

   /*!
    * @ingroup onDataBlock
    * @brief Returns true when the received block are Post_Mortem data.
    */
   bool descriptorWasPostMortem( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns true when the received block are High-Resolution data.
    */
   bool descriptorWasHighResolution( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns true when the received block are continuous data.
    */
   bool descriptorWasContinuous( void );

   int sendTriggerCondition( const uint32_t trgCondition );
   uint32_t receiveTriggerCondition( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns the trigger condition of this block.
    * @see sendTriggerCondition
    * @see receiveTriggerCondition
    */
   uint32_t descriptorGetTriggerCondition( void );

   /*!
    * @brief Set a trigger delay in LM32 tact cycles
    * @see receiveTriggerDelay
    */
   int sendTriggerDelay( const uint16_t delay );

   /*!
    * @brief Queries the by the function sendTriggerDelay adjusted
    *        trigger delay in LM32 tact cycles.
    * @see sendTriggerDelay
    * @return Trigger delay in LM32 tact cycles
    */
   uint16_t receiveTriggerDelay( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns the trigger delay of this block.
    * @see sendTriggerDelay
    * @see receiveTriggerDelay
    */
   uint16_t descriptorGetTriggerDelay( void );

   /*!
    * @brief Enables/disables the receive trigger.
    * @see receiveTriggerMode
    * @param mode true: trigger enable; false; trigger disable (default)
    */
   int sendTriggerMode( bool mode );

   /*!
    * @brief Queries the by sendTriggerMode adjusted trigger mode.
    * @see sendTriggerMode
    * @retval true: enabled
    * @retval false: disabled
    */
   bool receiveTriggerMode( void );

   int sendTriggerSourceContinue( bool extInput );
   bool receiveTriggerSourceContinue( void );

   int sendTriggerSourceHiRes( bool extInput );
   bool receiveTriggerSourceHiRes( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns the 8 bit sequence number of this block.
    * @see getExpectedSequence
    */
   uint8_t descriptorGetSequence( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns the 8 bit CRC check sum of this block.
    */
   uint8_t descriptorGetCrc( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns the white rabbit time stamp of the last received
    *        data word of this block.
    *
    * By the help of the function descriptorGetTimeBase it becomes possible
    * to calculate the time stamp of all received data words.
    *
    * @see descriptorGetTimeBase
    */
   uint64_t descriptorGetTimeStamp( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns the sample rate in nanoseconds of this block.
    * @see descriptorGetTimeStamp
    */
   unsigned int descriptorGetTimeBase( void );

   SequenceNumber* getSequencePtr( void ) const
   {
      SCU_ASSERT( m_poSequence != nullptr );
      return m_poSequence;
   }

   /*!
    * @ingroup onDataBlock
    * @brief Returns true when a lost between consecutive blocks was detected.
    * @see descriptorGetSequence
    * @see getExpectedSequence
    * @see getLostCount
    */
   bool descriptorWasBlockLost( void ) const
   {
      return getSequencePtr()->wasLost();
   }

   /*!
    * @ingroup onDataBlock
    * @brief Returns the expected 8 bit sequence number.
    */
   uint8_t getExpectedSequence( void ) const
   {
      return getSequencePtr()->m_sequence;
   }

   /*!
    * @ingroup onDataBlock
    * @brief Returns the number of detected lost data blocks.
    */
   unsigned int getLostCount( void ) const
   {
      return getSequencePtr()->getLostCount();
   }

protected:
   /*!
    * @ingroup onDataBlock
    * @brief Call back function becomes invoked when a data bock has been
    *        received.
    *
    * @param pData Pointer to the received raw payload data.\n
    *              <b>CAUTION:</b> This pointer is only within this function
    *              valid!
    *              If this data will be stored or used outside this scope,
    *              so a deep copy (e.g. memcpy()) becomes necessary.
    * @param wordLen Number of received payload raw data words of type
    *                DAQ_DATA_T (16 bit values).
    */
   virtual bool onDataBlock( DAQ_DATA_T* pData, std::size_t wordLen ) = 0;

private:
   void verifySequence( void );

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
   #define CHANNEL_LIST_T_BASE std::list
   typedef CHANNEL_LIST_T_BASE<DaqChannel*>  CHANNEL_LIST_T;
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

   const bool empty( void )
   {
      return m_channelPtrList.empty();
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

   const std::string& getWbDevice( void );

   const std::string getScuDomainName( void );

   unsigned int readMacroVersion( void );

   bool registerChannel( DaqChannel* pChannel );

   bool unregisterChannel( DaqChannel* pChannel );

   int sendEnablePostMortem( const unsigned int channel,
                             const bool restart = false );
   int sendEnableHighResolution( const unsigned int channel,
                                 const bool restart = false );
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

   int sendTriggerSourceHiRes( const unsigned int channel, bool extInput );
   bool receiveTriggerSourceHiRes( const unsigned int channel );

   DaqChannel* getChannel( const unsigned int number );
};

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Object of this type represents the container of all possible
 *        DAQ slaves on the SCU bus
 */
class DaqAdministration: public DaqInterface
{
   friend class DaqChannel;

   unsigned int      m_maxChannels;
   DAQ_DESCRIPTOR_T* m_poCurrentDescriptor;

   static std::exception_ptr c_exceptionPtr;

protected:
   #define DEVICE_LIST_BASE std::list
   typedef DEVICE_LIST_BASE<DaqDevice*> DEVICE_LIST_T;
   DEVICE_LIST_T  m_devicePtrList;

public:
   DaqAdministration( const std::string = DAQ_DEFAULT_WB_DEVICE,
                                                         bool doReset = true );
   virtual ~DaqAdministration( void );

   const DEVICE_LIST_T::iterator begin( void )
   {
      return m_devicePtrList.begin();
   }

   const DEVICE_LIST_T::iterator end( void )
   {
      return m_devicePtrList.end();
   }

   const bool empty( void )
   {
      return  m_devicePtrList.empty();
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

   uint32_t descriptorGetTriggerCondition( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return ::daqDescriptorGetTriggerCondition( m_poCurrentDescriptor );
   }

   uint16_t descriptorGetTriggerDelay( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return ::daqDescriptorGetTriggerDelay( m_poCurrentDescriptor );
   }

   uint8_t descriptorGetSequence( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return ::daqDescriptorGetSequence( m_poCurrentDescriptor );
   }

   uint8_t descriptorGetCrc( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return ::daqDescriptorGetCRC( m_poCurrentDescriptor );
   }

   bool descriptorWasPostMortem( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return ::daqDescriptorWasPM( m_poCurrentDescriptor );
   }

   bool descriptorWasHighResolution( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return ::daqDescriptorWasHiRes( m_poCurrentDescriptor );
   }

   bool descriptorWasContinuous( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return ::daqDescriptorWasDaq( m_poCurrentDescriptor );
   }

   uint64_t descriptorGetTimeStamp( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return ::daqDescriptorGetTimeStamp( m_poCurrentDescriptor );
   }

   unsigned int descriptorGetTimeBase( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return ::daqDescriptorGetTimeBase( m_poCurrentDescriptor );
   }

   void start( unsigned int toSleep = 100 );
   void stop( void );

private:
   DaqChannel* getChannelByDescriptor( DAQ_DESCRIPTOR_T& roDescriptor )
   {
      return getChannelBySlotNumber( ::daqDescriptorGetSlot( &roDescriptor ),
                                     ::daqDescriptorGetChannel( &roDescriptor )
                                     + 1 );
   }

   bool dataBlocksPresent( void );
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
inline const std::string& DaqDevice::getWbDevice( void )
{
   return getParent()->getWbDevice();
}

/*! ---------------------------------------------------------------------------
 */
inline const std::string DaqDevice::getScuDomainName( void )
{
   return getParent()->getScuDomainName();
}

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
inline int DaqDevice::sendEnableHighResolution( const unsigned int channel,
                                                const bool restart )
{
   return getParent()->sendEnableHighResolution( m_deviceNumber, channel,
                                                 restart );
}

/*! ---------------------------------------------------------------------------
 */
inline
int DaqDevice::sendEnableContineous( const unsigned int channel,
                                     const DAQ_SAMPLE_RATE_T sampleRate,
                                     const unsigned int maxBlocks )
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
inline
uint32_t DaqDevice::receiveTriggerCondition( const unsigned int channel )
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
   return getParent()->sendTriggerSourceContinue( m_deviceNumber, channel,
                                                  extInput );
}

/*! ---------------------------------------------------------------------------
 */
inline bool DaqDevice::receiveTriggerSourceContinue( const unsigned int channel )
{
   return getParent()->receiveTriggerSourceContinue( m_deviceNumber, channel );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendTriggerSourceHiRes( const unsigned int channel,
                                              bool extInput )
{
   return getParent()->sendTriggerSourceHiRes( m_deviceNumber, channel, extInput );
}

/*! ---------------------------------------------------------------------------
 */
inline bool DaqDevice::receiveTriggerSourceHiRes( const unsigned int channel )
{
   return getParent()->receiveTriggerSourceHiRes( m_deviceNumber, channel );
}


///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
inline const std::string& DaqChannel::getWbDevice( void )
{
   return getParent()->getWbDevice();
}

/*! ---------------------------------------------------------------------------
 */
inline const std::string DaqChannel::getScuDomainName( void )
{
   return getParent()->getScuDomainName();
}


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
   m_oSequencePmHighResMode.reset();
   return getParent()->sendEnablePostMortem( m_number, restart );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::sendEnableHighResolution( const bool restart )
{
   m_oSequencePmHighResMode.reset();
   return getParent()->sendEnableHighResolution( m_number, restart );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::sendEnableContineous( const DAQ_SAMPLE_RATE_T sampleRate,
                                             const unsigned int maxBlocks )
{
   m_oSequenceContinueMode.reset();
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
inline uint32_t DaqChannel::descriptorGetTriggerCondition( void )
{
   return getParent()->getParent()->descriptorGetTriggerCondition();
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
inline uint16_t DaqChannel::descriptorGetTriggerDelay( void )
{
   return getParent()->getParent()->descriptorGetTriggerDelay();
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

/*! ---------------------------------------------------------------------------
 */
inline int DaqChannel::sendTriggerSourceHiRes( bool extInput )
{
   return getParent()->sendTriggerSourceHiRes( m_number, extInput );
}

/*! ---------------------------------------------------------------------------
 */
inline bool DaqChannel::receiveTriggerSourceHiRes( void )
{
   return getParent()->receiveTriggerSourceHiRes( m_number );
}

/*! ---------------------------------------------------------------------------
 */
inline bool DaqChannel::descriptorWasPostMortem( void )
{
   return getParent()->getParent()->descriptorWasPostMortem();
}

/*! ---------------------------------------------------------------------------
 */
inline bool DaqChannel::descriptorWasHighResolution( void )
{
   return getParent()->getParent()->descriptorWasHighResolution();
}

/*! ---------------------------------------------------------------------------
 */
inline bool DaqChannel::descriptorWasContinuous( void )
{
   return getParent()->getParent()->descriptorWasContinuous();
}

/*! ---------------------------------------------------------------------------
 */
inline uint8_t DaqChannel::descriptorGetSequence( void )
{
   return getParent()->getParent()->descriptorGetSequence();
}

/*! ---------------------------------------------------------------------------
 */
inline uint8_t DaqChannel::descriptorGetCrc( void )
{
   return getParent()->getParent()->descriptorGetCrc();
}

/*! ---------------------------------------------------------------------------
 */
inline uint64_t DaqChannel::descriptorGetTimeStamp( void )
{
   return getParent()->getParent()->descriptorGetTimeStamp();
}

/*! ---------------------------------------------------------------------------
 */
inline unsigned int DaqChannel::descriptorGetTimeBase( void )
{
   return getParent()->getParent()->descriptorGetTimeBase();
}

} //namespace daq

#endif //  ifndef _DAQ_ADMINISTRATION_HPP
//================================== EOF ======================================
