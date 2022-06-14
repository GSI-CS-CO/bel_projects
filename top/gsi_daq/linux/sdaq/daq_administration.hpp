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
namespace Scu
{
namespace daq
{

class DaqAdministration;
class DaqDevice;

/*!
 * @brief Conversion factor: nanoseconds per second
 */
//constexpr double NanosecsPerSec = 1000000000.0;

/*!
 * @defgroup onDataBlock
 * @brief Functions belonging to this group can be invoked only within the
 *        call back function onDataBlock. Otherwise the system will crash or
 *        in the debug version a assertion becomes triggered!
 * @see onDataBlock
 * @todo Throwing a exception when one of these functions will invoked outside
 *       of its scope?
 */
/*!
 * @defgroup REGISTRATION
 * @brief Functions for registration of DAQ channels and DAQ devices
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
      uint m_lostCount;

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

      uint getLostCount( void ) const
      {
         return m_lostCount;
      }
   };

   /*!
    * @brief Channel number.
    */
   uint   m_number;

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
   DaqChannel( const uint number = 0 );

   /*!
    * @brief Destructor
    */
   virtual ~DaqChannel( void );

   /*!
    * @brief Returns the channel number in the possible range from 1 to
    *        DaqDevice::getMaxChannels.
    */
   const uint getNumber( void ) const
   {
      return m_number;
   }

   /*!
    * @brief Returns the pointer of the DAQ device in which is this channel
    *        object registered.
    * @see DaqDevice::registerChannel
    */
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

   /*!
    * @brief Returns the slot number of the DAQ-device to which belongs this
    *        channel.
    * @note The counting of the slot numbers begins at 1 ends ends at 12.
    * @note The slot number is <b>not</b> identical with the device number,
    *       except the DAQ device is in slot 1!
    * @see getDeviceNumber
    */
   const uint getSlot( void );

   /*!
    * @brief Returns the DAQ device number.
    * @note The counting of begins with the first DAQ on the left side
    *       of the SCU-bus with 1
    * @see getSlot
    */
   const uint getDeviceNumber( void );

   /*!
    * @brief Starts the Post-Mortem mode.
    * @param restart When true, restarting of the Post-Mortem mode after a
    *                trigger event, else it's a single shoot event.
    * @see sendDisablePmHires
    */
   int sendEnablePostMortem( const bool restart = false );

   /*!
    * @brief Starts the High-Resolution mode
    * @param restart When true, restarting of the High-Resolution mode after a
    *                trigger event, else it's a single shoot event.
    * @see sendDisablePmHires
    */
   int sendEnableHighResolution( const bool restart = false );

   /*!
    * @brief Starts the Continuous-Mode
    * @see sendDisableContinue
    * @see DAQ_SAMPLE_RATE_T
    * @param sampleRate Sample rate: 1ms, 100us or 10us
    * @param maxBlocks Limit of block receiving.
    *        if the number of received blocks equal this parameter,
    *        the Continuous-Mode becomes disabled. The value of zero is
    *        the endless mode.
    */
   int sendEnableContineous( const DAQ_SAMPLE_RATE_T sampleRate,
                             const uint maxBlocks = 0 );

   /*!
    * @brief Stops the continuous mode.
    * @see sendEnableContineous
    */
   int sendDisableContinue( void );

   /*!
    * @brief Stops the Post-Mortem and High-Resolution mode.
    * @see sendEnablePostMortem
    * @see sendEnableHighResolution
    */
   int sendDisablePmHires( const bool restart = false );

   /*!
    * @ingroup onDataBlock
    * @brief Returns true when the received block are Post_Mortem data.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   bool descriptorWasPostMortem( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns true when the received block are High-Resolution data.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   bool descriptorWasHighResolution( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns true when the received block are continuous data.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   bool descriptorWasContinuous( void );

   int sendTriggerCondition( const uint32_t trgCondition );
   uint32_t receiveTriggerCondition( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns the trigger condition of this block.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    * @see sendTriggerCondition
    * @see receiveTriggerCondition
    */
   uint32_t descriptorGetTriggerCondition( void );

   /*!
    * @brief Set a trigger delay in ADC-samples
    * @see receiveTriggerDelay
    * @param delay Trigger dalay in ADC-samples maximum is 65535 samples.
    */
   int sendTriggerDelay( const DAQ_REGISTER_T delay );

   /*!
    * @brief Queries the by the function sendTriggerDelay adjusted
    *        trigger delay in ADC-samples.
    * @see sendTriggerDelay
    * @return Trigger delay in ADC-samples
    */
   DAQ_REGISTER_T receiveTriggerDelay( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns the trigger delay of this block.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    * @see sendTriggerDelay
    * @see receiveTriggerDelay
    */
   DAQ_REGISTER_T descriptorGetTriggerDelay( void );

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

   /*!
    * @brief Setting the trigger source of the continuous mode to
    *        the external LEMO connector or to the internal
    *        White-Rabbit event.
    * @see receiveTriggerSourceContinue
    * @param extInput If true the trigger source is extern.
    */
   int sendTriggerSourceContinue( bool extInput );

   /*!
    * @brief Querying the by sendTriggerSource Continue currently adjusted
    *        trigger source of the continuous mode.
    * @see sendTriggerSourceContinue
    * @retval true External trigger source adjusted
    * @retval false Internal event trigger adjusted.
    */
   bool receiveTriggerSourceContinue( void );

   /*!
    * @brief Setting the trigger source of the high resolution mode to
    *        the external LEMO connector or to the internal
    *        White-Rabbit event.
    * @see receiveTriggerSourceHiRes
    * @param extInput If true the trigger source is extern.
    */
   int sendTriggerSourceHiRes( bool extInput );

   /*!
    * @brief Querying the by sendTriggerSource Continue currently adjusted
    *        trigger source of the high resolution mode.
    * @see sendTriggerSourceHiRes
    * @retval true External trigger source adjusted
    * @retval false Internal event trigger adjusted.
    */
   bool receiveTriggerSourceHiRes( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns the 8 bit sequence number of this block.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    * @see getExpectedSequence
    */
   uint8_t descriptorGetSequence( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns the 8 bit CRC check sum of this block.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   uint8_t descriptorGetCrc( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns the white rabbit time stamp of the last received
    *        data word of this block.
    *
    * By the help of the function descriptorGetTimeBase it becomes possible
    * to calculate the time stamp of all received data words.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    * @see descriptorGetTimeBase
    */
   uint64_t descriptorGetTimeStamp( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns the sample rate in nanoseconds of this block.
    * @see descriptorGetTimeStamp
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   uint descriptorGetTimeBase( void );

   /*!
    * @ingroup onDataBlock
    * @brief Returns the pointer of the currently used sequence
    *        number object.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   SequenceNumber* getSequencePtr( void ) const
   {
      SCU_ASSERT( m_poSequence != nullptr );
      return m_poSequence;
   }

   /*!
    * @ingroup onDataBlock
    * @brief Returns true when a lost between consecutive blocks was detected.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
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
   uint getLostCount( void ) const
   {
      return getSequencePtr()->getLostCount();
   }

protected:
   /*!
    * @ingroup onDataBlock
    * @brief Call back function becomes invoked when a data bock has been
    *        received.
    * @see DaqAdministration::distributeData
    * @param pData Pointer to the received raw payload data.\n
    *              <b>CAUTION:</b> This pointer is only within this function
    *              valid!
    *              If this data will be stored or used outside this scope,
    *              so a deep copy (e.g. memcpy()) becomes necessary.
    * @param wordLen Number of received payload raw data words of type
    *                DAQ_DATA_T (16 bit values).
    */
   virtual bool onDataBlock( DAQ_DATA_T* pData, std::size_t wordLen ) = 0;

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

private:
   void verifySequence( void );
}; // class DaqChannel

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Object of this type represents one of the 12 possible DAQ devices
 *        on the SCU bus. It's the container of the DAQ channels.
 */
class DaqDevice: public DaqBaseDevice
{
   friend class DaqAdministration;

   uint       m_deviceNumber;
   uint       m_slot;
   uint       m_maxChannels;
   DaqAdministration* m_pParent;

protected:
   #define CHANNEL_LIST_T_BASE std::list
   using CHANNEL_LIST_T = CHANNEL_LIST_T_BASE<DaqChannel*>;
   CHANNEL_LIST_T     m_channelPtrList;

public:
   /*!
    * @brief Constructor
    * @see DaqAdministration::registerDevice
    * @param slot Desired slot number in the range of 1 to 12.
    *        If the parameter equal 0 so the function
    *        DaqAdministration::registerDevice will set the slot number of
    *        next unregistered DAQ seen from the left side of the SCU slots.
    */
   DaqDevice( const uint slot = 0 );

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
    * @brief Returns the DAQ device number.
    * @note The counting of begins with the first DAQ on the left side
    *       of the SCU-bus with 1
    * @see getSlot
    */
   const uint getDeviceNumber( void ) const
   {
      return m_deviceNumber;
   }

   /*!
    * @brief Returns the slot number of the DAQ-device to which belongs this
    *        channel.
    * @note The counting of the slot numbers begins at 1 ends ends at 12.
    * @note The slot number is <b>not</b> identical with the device number,
    *       except the DAQ device is in slot 1!
    * @see getDeviceNumber
    */
   const uint getSlot( void ) const
   {
      return m_slot;
   }

   /*!
    * @brief Returns the number of channels of this DAQ device.
    */
   const uint getMaxChannels( void ) const
   {
      return m_maxChannels;
   }

   /*!
    * @brief Returns the pointer of the DAQ administration object in which
    *        this DAQ device is registered.
    */
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

   /*!
    * @brief Returns the VHDL macro version number of the DAQ device.
    */
   uint readMacroVersion( void );

   /*!
    * @ingroup REGISTRATION
    * @brief Registering of a channel object.
    * @param pChannel Pointer of the channel object to register.
    */
   bool registerChannel( DaqChannel* pChannel );

   bool unregisterChannel( DaqChannel* pChannel );

   /*!
    * @brief Starts the Post-Mortem mode.
    * @param channel Channel number
    * @param restart When true, restarting of the Post-Mortem mode after a
    *                trigger event, else it's a single shoot event.
    * @see DaqDevice::sendDisablePmHires
    */
   int sendEnablePostMortem( const uint channel,
                             const bool restart = false );

   /*!
    * @brief Starts the High-Resolution mode
    * @param channel Channel number
    * @param restart When true, restarting of the High-Resolution mode after a
    *                trigger event, else it's a single shoot event.
    * @see DaqDevice::sendDisablePmHires
    */
   int sendEnableHighResolution( const uint channel,
                                 const bool restart = false );
   /*!
    * @brief Starts the Continuous-Mode
    * @see sendDisableContinue
    * @see DAQ_SAMPLE_RATE_T
    * @param channel Channel number
    * @param sampleRate Sample rate: 1ms, 100us or 10us
    * @param maxBlocks Limit of block receiving.
    *        if the number of received blocks equal this parameter,
    *        the Continuous-Mode becomes disabled. The value of zero is
    *        the endless mode.
    */
   int sendEnableContineous( const uint channel,
                             const DAQ_SAMPLE_RATE_T sampleRate,
                             const uint maxBlocks = 0 );

   /*!
    * @brief Stops the continuous mode.
    * @param channel Channel number
    * @see DaqDevice::sendEnableContineous
    */
   int sendDisableContinue( const uint channel );

   /*!
    * @brief Stops the Post-Mortem and High-Resolution mode.
    * @see DaqDevice::sendEnablePostMortem
    * @see DaqDevice::sendEnableHighResolution
    * @param channel Channel number
    */
   int sendDisablePmHires( const uint channel,
                           const bool restart = false );


   int sendTriggerCondition( const uint channel,
                             const uint32_t trgCondition );
   uint32_t receiveTriggerCondition( const uint channel );

   /*!
    * @brief Set a trigger delay in ADC-samples
    * @param channel Channel number
    * @param delay Trigger dalay in ADC-samples maximum is 65535 samples.
    * @see DaqDevice::receiveTriggerDelay
    */
   int sendTriggerDelay( const uint channel,
                         const DAQ_REGISTER_T delay );

   /*!
    * @brief Queries the by the function sendTriggerDelay adjusted
    *        trigger delay in ADC-samples.
    * @see DaqDevice::sendTriggerDelay
    * @param channel Channel number
    * @return Trigger delay in ADC-samples
    */
   DAQ_REGISTER_T receiveTriggerDelay( const uint channel );

   /*!
    * @brief Enables/disables the receive trigger.
    * @see DaqDevice::receiveTriggerMode
    * @param channel Channel number
    * @param mode true: trigger enable; false; trigger disable (default)
    */
   int sendTriggerMode( const uint channel, bool mode );

   /*!
    * @brief Queries the by DaqDevice::sendTriggerMode adjusted trigger mode.
    * @see DaqDevice::sendTriggerMode
    * @param channel Channel number
    * @retval true: enabled
    * @retval false: disabled
    */
   bool receiveTriggerMode( const uint channel );

   /*!
    * @brief Setting the trigger source of the continuous mode to
    *        the external LEMO connector or to the internal
    *        White-Rabbit event.
    * @see DaqDevice::receiveTriggerSourceContinue
    * @param channel Channel number
    * @param extInput If true the trigger source is extern.
    */
   int sendTriggerSourceContinue( const uint channel, bool extInput );

   /*!
    * @brief Querying the by sendTriggerSource Continue currently adjusted
    *        trigger source of the continuous mode.
    * @see DaqDevice::sendTriggerSourceContinue
    * @param channel Channel number
    * @retval true External trigger source adjusted
    * @retval false Internal event trigger adjusted.
    */
   bool receiveTriggerSourceContinue( const uint channel );

   /*!
    * @brief Setting the trigger source of the high resolution mode to
    *        the external LEMO connector or to the internal
    *        White-Rabbit event.
    * @see DaqDevice::receiveTriggerSourceHiRes
    * @param channel Channel number
    * @param extInput If true the trigger source is extern.
    */
   int sendTriggerSourceHiRes( const uint channel, bool extInput );

   /*!
    * @brief Querying the by sendTriggerSource Continue currently adjusted
    *        trigger source of the high resolution mode.
    * @see DaqDevice::sendTriggerSourceHiRes
    * @param channel Channel number
    * @retval true External trigger source adjusted
    * @retval false Internal event trigger adjusted.
    */
   bool receiveTriggerSourceHiRes( const uint channel );

   /*!
    * @brief Returns the pointer of a registered channel object to which
    *        belongs the given channel number.
    * @param number Channel number.
    * @retval nullptr No channel with the given number registered.
    * @return Pointer to the registered channel object to which belongs the
    *         given number.
    */
   DaqChannel* getChannel( const uint number );

private:

   void init( void );

   void reset( void );

}; // class DaqDevice

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Object of this type represents the container of all possible
 *        DAQ slaves on the SCU bus
 */
class DaqAdministration: public DaqInterface
{
   friend class DaqChannel;

   uint              m_maxChannels;
   uint              m_receiveCount;

protected:
#ifdef CONFIG_DAQ_TIME_MEASUREMENT
   USEC_T            m_elapsedTime;
#endif
   DAQ_DESCRIPTOR_T* m_poCurrentDescriptor;

   #define DEVICE_LIST_BASE std::list
   using DEVICE_LIST_T = DEVICE_LIST_BASE<DaqDevice*>;
   DEVICE_LIST_T  m_devicePtrList;

public:
   DaqAdministration( DaqEb::EtherboneConnection* poEtherbone,
                      const bool doReset = true,
                      const bool doSendCommand = true
                    );

   DaqAdministration( DaqAccess* poEbAccess,
                      const bool doReset = true,
                      const bool doSendCommand = true
                    );

   ~DaqAdministration( void ) override;

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

#ifdef CONFIG_DAQ_TIME_MEASUREMENT
   /*!
    * @brief Returns the elapsed time of the last block-reading in
    *        microseconds.
    */
    uint64_t getElapsedTime( void ) const
    {
       return m_elapsedTime;
    }
#endif

   /*!
    * @brief Resets all existing DAQ's in this SCU.
    */
   void sendReset( void )
   {
      DaqInterface::sendReset();
      m_receiveCount = 0;
   }

   /*!
    * @brief Returns the number of received data-blocks after the last reset,
    *        doesn't matter whether the received blocks was valid or corrupt.
    */
   uint getReceiveCount( void ) const
   {
      return m_receiveCount;
   }

   /*!
    * @brief Returns the summation of registered channels of all
    *        registered DAQ devices.
    *
    * That means: the number of all DAQ-channels of this SCU.
    */
   uint getMaxChannels( void ) const
   {
      return m_maxChannels;
   }

   /*!
    * @brief Returns the number of all registered DAQ devices
    */
   uint getMaxDevices( void ) const
   {
      return m_devicePtrList.size();
   }

   /*!
    * @ingroup REGISTRATION
    * @brief Registering of a DAQ device object object.
    * @param pDevice Pointer to a object of type DaqDevice
    */
   bool registerDevice( DaqDevice* pDevice );

   bool unregisterDevice( DaqDevice* pDevice );

   /*!
    * @brief Reads the slot-state of DAQs in the SCU and
    *        actualized the slot number of all registered DAQ devices.
    */
   int redistributeSlotNumbers( void );

   /*!
    * @brief Returns the pointer of a registered DAQ-device with the given
    *        number.
    * @param number Device number in the range of 1 to 12.
    * @retval nullptr Device with the given number not registered.
    * @return Pointer to the DAQ device object.
    */
   DaqDevice* getDeviceByNumber( const uint number );

   /*!
    * @brief Returns the pointer of a registered DAQ-device which is
    *        in the SCU-bus-slot with the given slot number.
    * @param slot Slot number in the range of 1 to 12.
    * @retval nullptr No DAQ device in the given slot number respectively
    *                 Device is not registered.
    * @return Pointer to the DAQ device object which is in the given slot
    *         number.
    */
   DaqDevice* getDeviceBySlot( const uint slot );

   /*!
    * @brief Returns the pointer of a registered DAQ-channel object by
    *        the absolute channel number over all registered DAQ devices.
    *
    * The counting of channels begins at the first channel of the first
    * DAQ device seen from the left side of the SCU-bus slots.
    *
    * @param absChannelNumber Absolute channel number
    *        @note The lowest number begins at 1.
    * @retval nullptr No channel withe the given absolute number present,
    *                 respectively given number is out of the maximum of
    *                 existing channels.
    * @return Pointer to the DAQ channel object.
    */
   DaqChannel* getChannelByAbsoluteNumber( uint absChannelNumber );

   /*!
    * @brief Returns the pointer of a registered DAQ-channel object by
    *        the device number and channel number.
    *
    * The counting of the DAQ device number begins at the first device
    * in the SCU-bus seen from the left side. The range is 1 to 12.\n
    * The range of the channel number is 1 to DaqDevice::getMaxChannels().
    * @param deviceNumber DAQ device number
    * @param channelNumber Channel number of the concerning DAQ device.
    * @retval nullptr Channel not present.
    * @return Pointer to the DAQ channel object.
    */
   DaqChannel* getChannelByDeviceNumber( const uint deviceNumber,
                                         const uint channelNumber );

   /*!
    * @brief Returns the pointer of a registered DAQ-channel object by
    *        the slot number and channel number.
    *
    * The counting of slot numbers begins at the first SCU-bus slot
    * seen from the left side.\n
    * The range of the channel number is 1 to DaqDevice::getMaxChannels().
    * @param slotNumber Slot number in the range of 1 to 12.
    * @param channelNumber Channel number of the concerning DAQ device.
    * @retval nullptr Channel not present.
    * @return Pointer to the DAQ channel object.
    */
   DaqChannel* getChannelBySlotNumber( const uint slotNumber,
                                       const uint channelNumber );

   /*!
    * @ingroup onDataBlock
    * @brief Main-loop function.
    *
    * This polls the size of the DDR3 ring-buffer.
    * When the level of the ring buffer has at least reached the data volume
    * of one block, so this data will copied in the concerning channel object.
    * That means the corresponding call-back function
    * DaqChannel::onDataBlock() becomes invoked by this.
    * @see DaqChannel::onDataBlock
    */
   uint distributeData( void ) override;

   void reset( void ) override;

   /*!
    * @ingroup onDataBlock
    * @brief Returns the trigger condition of currently received block.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   uint32_t descriptorGetTriggerCondition( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return daqDescriptorGetTriggerCondition( m_poCurrentDescriptor );
   }

   /*!
    * @ingroup onDataBlock
    * @brief Returns the trigger delay of LM32 cycles of currently received
    *        block.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   DAQ_REGISTER_T descriptorGetTriggerDelay( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return daqDescriptorGetTriggerDelay( m_poCurrentDescriptor );
   }

   /*!
    * @ingroup onDataBlock
    * @brief Returns the sequence number of the currently received block.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   uint8_t descriptorGetSequence( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return daqDescriptorGetSequence( m_poCurrentDescriptor );
   }

   /*!
    * @ingroup onDataBlock
    * @brief Returns the CRC number of the currently received block.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   uint8_t descriptorGetCrc( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return daqDescriptorGetCRC( m_poCurrentDescriptor );
   }

   /*!
    * @ingroup onDataBlock
    * @brief Returns true when the received block are Post_Mortem data.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   bool descriptorWasPostMortem( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return daqDescriptorWasPM( m_poCurrentDescriptor );
   }

   /*!
    * @ingroup onDataBlock
    * @brief Returns true when the received block are High-Resolution data.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   bool descriptorWasHighResolution( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return daqDescriptorWasHiRes( m_poCurrentDescriptor );
   }

   /*!
    * @ingroup onDataBlock
    * @brief Returns true when the received block are continuous data.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   bool descriptorWasContinuous( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return daqDescriptorWasDaq( m_poCurrentDescriptor );
   }

   /*!
    * @ingroup onDataBlock
    * @brief Returns the white rabbit time stamp of the last received
    *        data word of the currently block.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   uint64_t descriptorGetTimeStamp( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return daqDescriptorGetTimeStamp( m_poCurrentDescriptor );
   }

   /*!
    * @ingroup onDataBlock
    * @brief Returns the sample rate in nanoseconds of the currently block.
    * @note This function can only be used within the validity range
    *       of the callback function DaqChannel::onDataBlock!
    */
   uint descriptorGetTimeBase( void )
   {
      SCU_ASSERT( m_poCurrentDescriptor != nullptr );
      return daqDescriptorGetTimeBase( m_poCurrentDescriptor );
   }

   /*!
    * @ingroup onDataBlock
    * @brief Optional callback function becomes invoked when a erroneous
    *        device descriptor has been received.
    * @note When this function will not overwritten than an exception
    *       becomes triggered if becomes invoked.
    */
   virtual void onErrorDescriptor( const DAQ_DESCRIPTOR_T& roDescriptor );

protected:
   virtual void onUnregistered( DAQ_DESCRIPTOR_T& roDescriptor ) {}

private:
   DaqChannel* getChannelByDescriptor( DAQ_DESCRIPTOR_T& roDescriptor )
   {
      return getChannelBySlotNumber( daqDescriptorGetSlot( &roDescriptor ),
                                     daqDescriptorGetChannel( &roDescriptor )
                                     + 1 );
   }

#ifndef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
   bool dataBlocksPresent( void );

   int readDaqDataBlock( RAM_DAQ_PAYLOAD_T* pData, std::size_t len );
#endif
}; // class DaqAdministration

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
inline uint DaqDevice::readMacroVersion( void )
{
   return getParent()->readMacroVersion( m_deviceNumber );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendEnablePostMortem( const uint channel,
                                            const bool restart )
{
   return getParent()->sendEnablePostMortem( m_deviceNumber, channel,
                                             restart );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendEnableHighResolution( const uint channel,
                                                const bool restart )
{
   return getParent()->sendEnableHighResolution( m_deviceNumber, channel,
                                                 restart );
}

/*! ---------------------------------------------------------------------------
 */
inline
int DaqDevice::sendEnableContineous( const uint channel,
                                     const DAQ_SAMPLE_RATE_T sampleRate,
                                     const uint maxBlocks )
{
   return getParent()->sendEnableContineous( m_deviceNumber, channel,
                                             sampleRate, maxBlocks );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendDisableContinue( const uint channel )
{
   return getParent()->sendDisableContinue( m_deviceNumber, channel );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendDisablePmHires( const uint channel,
                                          const bool restart )
{
   return getParent()->sendDisablePmHires( m_deviceNumber, channel,
                                           restart  );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendTriggerCondition( const uint channel,
                                           const uint32_t trgCondition )
{
   return getParent()->sendTriggerCondition( m_deviceNumber, channel,
                                            trgCondition );
}

/*! ---------------------------------------------------------------------------
 */
inline
uint32_t DaqDevice::receiveTriggerCondition( const uint channel )
{
   return getParent()->receiveTriggerCondition( m_deviceNumber, channel );
}


/*! ---------------------------------------------------------------------------
 */
inline int
DaqDevice::sendTriggerDelay( const uint channel,
                                                   const DAQ_REGISTER_T delay )
{
   return getParent()->sendTriggerDelay( m_deviceNumber, channel, delay );
}

/*! ---------------------------------------------------------------------------
 */
inline DAQ_REGISTER_T
DaqDevice::receiveTriggerDelay( const uint channel )
{
   return getParent()->receiveTriggerDelay( m_deviceNumber, channel );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendTriggerMode( const uint channel, bool mode )
{
   return getParent()->sendTriggerMode( m_deviceNumber, channel, mode );
}

/*! ---------------------------------------------------------------------------
 */
inline bool DaqDevice::receiveTriggerMode( const uint channel )
{
   return getParent()->receiveTriggerMode( m_deviceNumber, channel );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendTriggerSourceContinue( const uint channel,
                                                 bool extInput )
{
   return getParent()->sendTriggerSourceContinue( m_deviceNumber, channel,
                                                  extInput );
}

/*! ---------------------------------------------------------------------------
 */
inline bool DaqDevice::receiveTriggerSourceContinue( const uint channel )
{
   return getParent()->receiveTriggerSourceContinue( m_deviceNumber, channel );
}

/*! ---------------------------------------------------------------------------
 */
inline int DaqDevice::sendTriggerSourceHiRes( const uint channel,
                                              bool extInput )
{
   return getParent()->sendTriggerSourceHiRes( m_deviceNumber,
                                                           channel, extInput );
}

/*! ---------------------------------------------------------------------------
 */
inline bool DaqDevice::receiveTriggerSourceHiRes( const uint channel )
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
inline const uint DaqChannel::getSlot( void )
{
   return getParent()->getSlot();
}

/*! ---------------------------------------------------------------------------
 */
inline const uint DaqChannel::getDeviceNumber( void )
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
                                             const uint maxBlocks )
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
inline DAQ_REGISTER_T DaqChannel::receiveTriggerDelay( void )
{
   return getParent()->receiveTriggerDelay( m_number );
}

/*! ---------------------------------------------------------------------------
 */
inline DAQ_REGISTER_T DaqChannel::descriptorGetTriggerDelay( void )
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
inline uint DaqChannel::descriptorGetTimeBase( void )
{
   return getParent()->getParent()->descriptorGetTimeBase();
}

} //namespace daq
} //namespace Scu
#endif //  ifndef _DAQ_ADMINISTRATION_HPP
//================================== EOF ======================================
