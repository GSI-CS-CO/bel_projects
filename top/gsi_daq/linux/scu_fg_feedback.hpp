/*!
 * @file scu_fg_feedback.hpp
 * @brief Administration of data aquesition units for function generator
 *        feedback.  Fusion of MIL- and ADDAC DAQ.
 *
 * @date 25.05.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * example feedback-example.cpp
 * @include feedback-example.cpp
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>
 ******************************************************************************
 */
#ifndef _SCU_FG_FEEDBACK_HPP
#define _SCU_FG_FEEDBACK_HPP

#include <list>
#include <scu_control_config.h>
#include <daq_calculations.hpp>
#include <daq_administration.hpp>
#ifdef CONFIG_MIL_FG
 #include <mdaq_administration.hpp>
#endif
#include <scu_fg_list.hpp>
#include <daq_base_interface.hpp>

namespace Scu
{
using namespace gsi;

class FgFeedbackDevice;
class FgFeedbackAdministration;

///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Object type of feedback-channel for all DAQ-types.
 *
 * Polymorphic object type, after registration in FgFeedbackDevice it
 * converts to a MIL- or to a ADDAC/ACU- feedback channel, depending whether
 * the FgFeedbackDevice is a MIL or ADDAC/ACU device.
 */
class FgFeedbackChannel
{
   friend class FgFeedbackDevice;
   friend class FgFeedbackAdministration;

public:
 #ifdef CONFIG_MIL_FG
   using DAQ_T = MiLdaq::MIL_DAQ_T;
 #else
   using DAQ_T = uint32_t;
 #endif

private:
   /*!
    * @brief Common object type for ADDAC/ACU- and MIL- feedback channel
    */
   class Common
   {
      friend class FgFeedbackAdministration;

      /*!
       * @brief Object-type for reducing the forwarding to the higher
       *        software layer.
       */
      class Throttle
      {
         Common*  m_pParent;
         DAQ_T    m_lastForwardedValue;
         uint64_t m_timeThreshold;

      public:
         Throttle( Common* pParent );
         ~Throttle( void );
         bool operator()( const uint64_t timestamp, const DAQ_T value );
      }; // class Throttle

   protected:
      FgFeedbackChannel* m_pParent;

   private:
      /*!
       * @brief Throttle object for the set data stream.
       */
      Throttle  m_oSetThrottle;

      /*!
       * @brief Throttle object for the actual data stream.
       */
      Throttle  m_oActThrottle;

      /*!
       * @brief Storage of the last suppressed timestamp.
       */
      uint64_t  m_lastSupprTimestamp;

      /*!
       * @brief Storage of the last suppressed set-value.
       */
      DAQ_T     m_lastSupprSetValue;

      /*!
       * @brief Storage of the last suppressed actual-value.
       */
      DAQ_T     m_lastSupprActValue;

   public:
      Common( FgFeedbackChannel* pParent );
      virtual ~Common( void );
      void evaluate( const uint64_t wrTimeStampTAI,
                     const DAQ_T actValue,
                     const DAQ_T setValue );
   }; // class Common

   /*!
    * @brief Object type handling ADDAC/ACU-DAQ set- and actual- channel.
    */
   class AddacFb: public Common
   {
      friend class FgFeedbackDevice;

      /*!
       * @brief Object type containing the data buffer for received actual or
       *        set values of a ADDAC-DAQ data block.
       */
      class Receive: public daq::DaqChannel
      {
         AddacFb*            m_pParent;
         uint64_t            m_timestamp;
         uint                m_sampleTime;
         std::size_t         m_blockLen;
         daq::DAQ_SEQUENCE_T m_sequence;
         daq::DAQ_DATA_T     m_aBuffer[daq::DaqAdministration::c_contineousDataLen];

      public:
         Receive( AddacFb* pParent, const uint n );
         virtual ~Receive( void );

         uint64_t getTimestamp( void ) const
         {
            return m_timestamp;
         }

         uint const getSampleTime( void ) const
         {
            return m_sampleTime;
         }

         daq::DAQ_SEQUENCE_T getSequence( void ) const
         {
            return m_sequence;
         }

         std::size_t getBlockLen( void ) const
         {
            return m_blockLen;
         }

         DAQ_T operator[]( const std::size_t i ) const;

      protected:
         bool onDataBlock( daq::DAQ_DATA_T* pData, std::size_t wordLen ) override;
         void onInit( void ) override;
         void onReset( void ) override;
      }; // class Receive

      /*!
       * @brief Data buffer of the last received set values.
       */
      Receive m_oReceiveSetValue;

      /*!
       * @brief Data buffer of the last received actual values.
       */
      Receive m_oReceiveActValue;

   #ifdef _CONFIG_PATCH_PHASE
      uint64_t m_expectedTimestamp;
      DAQ_T    m_lastValue;
   #endif
   public:
      AddacFb( FgFeedbackChannel* pParent, const daq::DAQ_DEVICE_TYP_T type );
      virtual ~AddacFb( void );

   private:
      void finalizeBlock( void );
   }; // class AddacFb

#ifdef CONFIG_MIL_FG
   /*!
    * @brief Object type handling MIL- channel.
    */
   class MilFb: public Common
   {
      friend class FgFeedbackDevice;

      /*!
       * @brief Object type to forwarding incoming MIL-data
       *        to a higher software layer.
       */
      class Receive: public MiLdaq::DaqCompare
      {
         MilFb*  m_pParent;
      public:
         Receive( MilFb* pParent );
         virtual ~Receive( void );
         void onData( uint64_t wrTimeStampTAI,
                      MiLdaq::MIL_DAQ_T actlValue,
                      MiLdaq::MIL_DAQ_T setValue ) override;
         void onInit( void ) override;
         void onReset( void ) override;
      }; // class Receive

      Receive m_oReceive;

   public:
      MilFb( FgFeedbackChannel* pParent );
      virtual ~MilFb( void );
      bool isSetValueInvalid( void )
      {
         return m_oReceive.isSetValueInvalid();
      }
   }; // class MilFb
#endif // ifdef CONFIG_MIL_FG

   /*!
    * @brief Number of function-generator
    */
   const uint         m_fgNumber;

   /*!
    * @brief Pointer to the parent object in which this object becomes
    *        registered.
    */
   FgFeedbackDevice*  m_pParent;

   /*!
    * @brief Pointer keeps either the address of a object of type AddacFb
    *        or MilFb.
    */
   Common*            m_pCommon;

   /*!
    * @brief Last received timestamp.
    */
   uint64_t           m_lastTimestamp;

public:
   /*!
    * @brief Constructor of a single function generator feedback channel.
    * @code
    * fg-10-3 In this example is the function generator number 3.
    * @endcode
    * @param fgNumber Number of function generator.
    */
   FgFeedbackChannel( const uint fgNumber );

   /*!
    * @brief Destructor, if a instance of this type has been registered
    *        then it will deregistered by its self.
    */
   virtual ~FgFeedbackChannel( void );

   /*!
    * @ingroup REGISTRATION
    * @brief Returns the object of type FgFeedbackDevice
    *        in which this object has been registered, if
    *        not registered then a exception will thrown.
    */
   FgFeedbackDevice* getParent( void );

   /*!
    * @brief Returns the function generator number.
    *
    * That is the constructors argument.
    */
   uint getFgNumber( void ) const
   {
      return m_fgNumber;
   }

   /*!
    * @brief Returns the socket number if this object has been registered
    *        in a object of type FgFeedbackDevice else a exception will thrown.
    */
   uint getSocket( void );

   /*!
    * @brief Returns the function generator name.
    *
    * E.g.: "fg-4-0"
    * @return Name of function generator.
    */
   std::string getFgName( void );

#ifdef CONFIG_MIL_FG
   /*!
    * @brief Returns true if this object communicates with a MIL-DAQ.
    */
   bool isMil( void );
#endif

   /*!
    * @brief Returns "true" in the case of gap-reading when the last received
    *        set-value was invalid.
    * @note This function can be used within the the callback function "onData"
    *       to distinguish the actual data item is within a gap or not.
    * @retval false Set value is valid, data-item is outside of the gap.
    * @retval true  Set value is the last valid received set value,
    *               data item is inside of a gap
    */
   bool isSetValueInvalid( void );

   /*!
    * @brief Returns the time-stamp of the last data tuple independently
    *        whether the tuple was suppressed by throttling or not.
    * @note When the returned value is zero then no data was received yet.
    */
   uint64_t getLastTimestamp( void ) const
   {
      return m_lastTimestamp;
   }

protected:
   /*!
    * @brief Callback function becomes invoked for each incoming data item which
    *        belongs to this object.
    * @param wrTimeStampTAI White rabbit time stamp TAI.
    * @param actlValue Actual value from the DAQ.
    * @param setValue Set value from function generator
    */
   virtual void onData( uint64_t wrTimeStampTAI,
                        DAQ_T actlValue,
                        DAQ_T setValue ) = 0;

   /*!
    * @brief Optional callback function becomes invoked once this object
    *        is registered in its container of type DaqDevice and this
    *        container is again registered in the administrator
    *        object of type DaqAdministration.
    *
    * That means once the connection-path of this object to
    * the LM32-application has been established.
    */
   virtual void onInit( void ) {}

   /*!
    * @brief Optional callback function becomes invoked by a reset event.
    */
   virtual void onReset( void ) {}

   /*!
    * @brief Optional callback function for debug purposes becomes
    *        invoked for all incoming ADDAC/ACO-DAQ data blocks.
    */
   virtual void onAddacDataBlock( const bool isSetData,
                                  const uint64_t timestamp,
                                  daq::DAQ_DATA_T* pData,
                                  std::size_t wordLen ) {}

   /*!
    * @brief Optional callback function becomes invoked by incoming non-
    *        continuous ADDAC_DAQ-blocks that means for high resolution or
    *        post mortem blocks.
    */
   virtual void onHighResPostMortemBlock( const bool isSetData,
                                          const uint64_t timestamp,
                                          daq::DAQ_DATA_T* pData,
                                          std::size_t wordLen ) {}

   /*!
    * @brief Over-writable callback function becomes invoked if the deviation
    *        of sequence-numbers of actual- and set -values greater than one.
    * @param setSequ Sequence number of last received set-value block
    * @param actSequ Sequence number of last received actual-value block
    */
   virtual void onActSetBlockDeviation( const uint setSequ, const uint actSequ );

   /*!
    * @brief Over-writable callback function becomes invoked if the deviation
    *        of timestamp of actual- and set -values not equal
    *        respectively a synchronization isn't possible.
    * @param setTimeStamp Time stamp of set values.
    * @param actTimestamp Time stamp of actual values.
    */
   virtual void onActSetTimestampDeviation( const uint64_t setTimeStamp,
                                            const uint64_t actTimestamp );

   /*!
    * @brief Optional over-writable callback function becomes invoked in the
    *        case if a time stamp error has been detected.
    * @note For debug purposes.
    * @param timestamp Possibly wrong time stamp.
    */
   virtual void onTimestampError( const uint64_t timestamp,
                                  DAQ_T actlValue,
                                  DAQ_T setValue ) {}

#ifdef CONFIG_MIL_FG
   /*!
    * @brief Optional callback function for debug purposes becomes
    *        invoked for all incoming MIL data tuples.
    */
   virtual void onMilData( const uint64_t timestamp,
                           DAQ_T actlValue,
                           DAQ_T setValue ) {}
#endif
}; // class FgFeedbackChannel

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Object type for MIL-or ADDAC/ACU devices.
 *
 * Polymorphic object type, depending on the socket number
 * - the constructors argument - it converts to a MIL- or to a ADDAC/ACU-
 * device.
 *
 * A object of this type can contain one or more feedback channels
 * in the shape of pointers of the base type FgFeedbackChannel*.
 */
class FgFeedbackDevice
{
   friend class FgFeedbackChannel;
   friend class FgFeedbackAdministration;

   using CHANNEL_LIST_T = std::list<FgFeedbackChannel*>;

   DaqBaseDevice*            m_poDevice;
   FgFeedbackAdministration* m_pParent;
   CHANNEL_LIST_T            m_lChannelList;

public:
   using DAQ_T = FgFeedbackChannel::DAQ_T;

   /*!
    * @brief Constructor of a feedback device which can contain one or
    *        more feedback-channels.
    * @note If the given socket number invalid, so a exception will thrown!
    * @param socked Socket-number
    * @code
    * fg-10-3 In this example the socket number is 10.
    * @endcode
    */
   FgFeedbackDevice( const uint socket );

   /*!
    * @brief Destructor, if a instance of this type has been registered
    *        then it will deregistered by its self.
    */
   ~FgFeedbackDevice( void );


   /*!
    * @ingroup REGISTRATION
    * @brief Returns a pointer to the parent object in which this
    *        object has been registered.
    * @note If this object not registered, then a exception will thrown!
    */
   FgFeedbackAdministration* getParent( void );

   /*!
    * @ingroup REGISTRATION
    * @brief Registering of a single feedback channel associated
    *        to a function generator.
    * @note If the channel has been already registered or its channel-number
    *       is invalid then a exception will thrown.
    * @param pFeedbackChannel Pointer of type FgFeedbackChannel to register.
    */
   void registerChannel( FgFeedbackChannel* pFeedbackChannel );

   /*!
    * @brief Counterpart to registerChannel.
    * @see registerChannel
    */
   void unregisterChannel( FgFeedbackChannel* pFeedbackChannel );

   /*!
    * @brief Returns the socket number.
    *
    * It is the constructors argument.
    * @note In the case of a ADDAC/ACU device the slot number is identical to
    *       the socket number.
    */
   uint getSocket( void ) const
   {
      assert( dynamic_cast<DaqBaseDevice*>( m_poDevice ) != nullptr );
      return m_poDevice->getSocket();
   }

   /*!
    * @brief Returns the SCU- bus slot number occupying this device.
    * @note In the case of a ADDAC/ACU device the slot number is identical to
    *       the socket number.
    */
   uint getSlot( void ) const
   {
      assert( dynamic_cast<DaqBaseDevice*>( m_poDevice ) != nullptr );
      return m_poDevice->getSlot();
   }

   /*!
    * @brief Returns the device type.
    * @note If the object isn't registered yet then this function will return
    *       UNKNOWN, else ADDAC, ACU, DOIB or MIL.
    * @retval UNKNOWN Maybe no connection to the LM32 app yet.
    * @retval ADDAC   This object communicates with a ADDAC device
    * @retval ACU     This object communicates with a ACU device
    * @retval DIOB    This object communicates with a DIOB device
    * @retval MIL     This object communicates with a MIL device
    */
   daq::DAQ_DEVICE_TYP_T getTyp( void ) const
   {
      assert( dynamic_cast<DaqBaseDevice*>( m_poDevice ) != nullptr );
      return m_poDevice->getTyp();
   }

#ifdef CONFIG_MIL_FG
   /*!
    * @brief Returns the pointer to the MIL device if this object has been
    *        mutated to a MIL- object after registration in
    *        object of FgFeedbackAdministration, else nullptr.
    */
   MiLdaq::DaqDevice* getMil( void ) const
   {
      return dynamic_cast<MiLdaq::DaqDevice*>(m_poDevice);
   }

   /*!
    * @brief Returns "true" if this object has been mutated to a MIL- object
    *        after registration in object of FgFeedbackAdministration,
    *        else "false".
    */
   bool isMil( void ) const
   {
      return (getMil() != nullptr);
   }
#endif /* ifdef CONFIG_MIL_FG */

   /*!
    * @brief Returns the pointer to the ADDAC/ACU device if this object
    *        has been mutated to a ADDAC/ACU- object after registration in
    *        object of FgFeedbackAdministration, else nullptr.
    */
   daq::DaqDevice* getAddac( void ) const
   {
      return dynamic_cast<daq::DaqDevice*>(m_poDevice);
   }

   /*!
    * @brief Returns "true" if this object has been mutated to a
    *        ADDAC/ACU- object after registration in
    *        object of FgFeedbackAdministration, else "false".
    */
   bool isAddac( void ) const
   {
      return (getAddac() != nullptr);
   }

   /*!
    * @brief Returns the pointer to the channel object tu which belongs
    *        the given number
    * @param nunber Channel number
    * @retval !=nullptr Pointer of channel object
    * @retval ==nullptr Chnnel not present respectively not registered.
    */
   FgFeedbackChannel* getChannel( const uint number );

   /*!
    * @ingroup REGISTRATION
    * @brief Helper function for browsing in the list of registered channel objects.
    * @see FgFeedbackAdministration::begin
    * @returns Start-iterator of the pointer of registered
    *          channel objects of type FgFeedbackChannel*
    *
    */
   const CHANNEL_LIST_T::iterator begin( void )
   {
      return m_lChannelList.begin();
   }

   /*!
    * @ingroup REGISTRATION
    * @brief Helper function for browsing in the list of registered channel objects.
    * @see FgFeedbackAdministration::end
    * @returns Stop-iterator of the pointer of registered
    *          channel objects of type FgFeedbackChannel*
    */
   const CHANNEL_LIST_T::iterator end( void )
   {
      return m_lChannelList.end();
   }

private:

   void generateAll( void );
   void generate( FgFeedbackChannel* pFeedbackChannel );
}; // class FgFeedbackDevice

#ifdef CONFIG_MIL_FG
inline   bool FgFeedbackChannel::isMil( void )
{
  return getParent()->isMil();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Object type for the feedback administration of all types of
 *        SCU function generators
 *
 * A object of this type can contain one or more feedback channels
 * in the shape of pointers of the base type FgFeedbackDevice*.
 */
class FgFeedbackAdministration
{
   friend class FgFeedbackChannel::Common::Throttle;

   using DAQ_POLL_T     = std::vector<DaqBaseInterface*>;
   using GEN_DEV_LIST_T = std::list<FgFeedbackDevice*>;

public:
   using DAQ_T          = FgFeedbackDevice::DAQ_T;

#ifdef CONFIG_EB_TIME_MEASSUREMENT
   /*!
    * @ingroup TIME_MEASUREMENT_T
    * @brief Access constants to inform which wishbone/etherbone access was made.
    */
   enum WB_ACCESS_T
   {  /*!
       * @brief Unknown, that means no wishbone access has been made.
       */
      UNKNOWN    = daq::EbRamAccess::TIME_MEASUREMENT_T::UNKNOWN,

      /*!
       * @brief Read access from LM32 shared memory has been made.
       */
      LM32_READ  = daq::EbRamAccess::TIME_MEASUREMENT_T::LM32_READ,

      /*!
       * @brief Write access from LM32 shared memory has been made.
       */
      LM32_WRITE = daq::EbRamAccess::TIME_MEASUREMENT_T::LM32_WRITE,

      /*!
       * @brief Read access from DDR3-ram has been made.
       */
      DDR3_READ  = daq::EbRamAccess::TIME_MEASUREMENT_T::DDR3_READ
   };
#endif /* ifdef CONFIG_EB_TIME_MEASSUREMENT */

   static constexpr uint  VALUE_SHIFT = (BIT_SIZEOF( DAQ_T ) - BIT_SIZEOF( daq::DAQ_DATA_T ));
   static constexpr DAQ_T DEFAULT_THROTTLE_THRESHOLD = 10;
   static constexpr uint  DEFAULT_THROTTLE_TIMEOUT   = 10;

private:
   /*!
    * @brief List of function generators found by the LM32 application.
    */
   FgList m_oFoundFgs;

   /*!
    * @brief Object type for ADDAC-DAQ administration
    */
   class AddacAdministration: public daq::DaqAdministration
   {
      FgFeedbackAdministration* m_pParent;

      /*!
       * @brief If true than the pairing of actual- and set- values
       *        for non-MIL DAQs will made by the block- sequence number.
       */
      bool                      m_pairingBySequence;

   public:
      AddacAdministration( FgFeedbackAdministration* pParent, DaqEb::EtherboneConnection* poEtherbone )
        :daq::DaqAdministration( poEtherbone, false, false )
        ,m_pParent( pParent )
        ,m_pairingBySequence( false )
      {
      }

      AddacAdministration( FgFeedbackAdministration* pParent, DaqAccess* poEbAccess )
        :daq::DaqAdministration( poEbAccess, false, false )
        ,m_pParent( pParent )
        ,m_pairingBySequence( false )
      {
      }

      /*!
       * @brief Enables or disables the pairing of set and actual values
       *        of non-MIL DAQs by sequence number.
       * @param pairingBySequence If true than the pairing by sequence number
       *                          is active, else the pairing will made by
       *                          the timestamp.
       */
      void setPairingBySequence( const bool pairingBySequence )
      {
         m_pairingBySequence = pairingBySequence;
      }

      /*!
       * @brief Asks whether the pairing of set and actual values
       *        of non-MIL DAQs  will made by sequence number
       *        or by timestamp.
       */
      bool isPairingBySequence( void )
      {
         return m_pairingBySequence;
      }


      void onDataReadingPause( void ) override;

      void onUnregistered( daq::DAQ_DESCRIPTOR_T& roDescriptor ) override;

      void onBlockReceiveError( void ) override;

      void onDataTimeout( void ) override;

      void onDataError( void ) override;

      void onErrorDescriptor( const daq::DAQ_DESCRIPTOR_T& roDescriptor ) override;
   }; // class AddacAdministration
   /*!
    * @brief Object for ADDAC DAQ administration.
    */
   AddacAdministration     m_oAddacDaqAdmin;

#ifdef CONFIG_MIL_FG
   /*!
    * @brief Object type for MIL-DAQ administration
    */
   class MilDaqAdministration: public MiLdaq::DaqAdministration
   {
      FgFeedbackAdministration* m_pParent;
   public:
      MilDaqAdministration( FgFeedbackAdministration* pParent, DaqAccess* poEbAccess )
        :MiLdaq::DaqAdministration( poEbAccess )
        ,m_pParent( pParent )
      {
      }
      virtual ~MilDaqAdministration( void ) {}

      void onUnregistered( const FG_MACRO_T fg )  override;
#ifndef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
      RAM_RING_INDEX_T getCurrentRamSize( bool update = true ) override
      {
         return 0; //TODO
      }
#endif
      void onDataReadingPause( void ) override;

      void onDataTimeout( void ) override;

      void onDataError( void ) override;
   }; // class MilDaqAdministration

   /*!
    * @brief Object for MIL DAQ administration.
    */
   MilDaqAdministration  m_oMilDaqAdmin;
#endif /* ifdef CONFIG_MIL_FG */

   /*!
    * @brief Object triggering software interrupts to LM32-firmware
    */
   Lm32Swi                    m_lm32Swi;

   DAQ_POLL_T                 m_vPollList;
   GEN_DEV_LIST_T             m_lDevList;

   DAQ_T                      m_throttleThreshold;
   uint64_t                   m_throttleTimeout;

protected:
   #define DEVICE_LIST_BASE std::list
   using DEVICE_LIST_T = DEVICE_LIST_BASE<FgFeedbackDevice*>;
   DEVICE_LIST_T m_devicePtrList;

public:
   /*!
    * @brief First constructor variant: Establishes a wishbone/etherbone
    *        connection and initialized the communication between
    *        Linux-host and LM32 application.
    * @note If the wishbone/etherbone connection not already open then
    *       this constructor will do that.
    * @param poEtherbone Pointer to a object of type EtherboneConnection
    * @param doRescan If true then a rescan command will send to the
    *                 LM32 -application.
    */
   FgFeedbackAdministration( DaqEb::EtherboneConnection* poEtherbone, const bool doRescan = false );

   /*!
    * @brief Second constructor variant:
    * @param poEbAccess Pointer to a object of type DaqAccess
    * @param doRescan If true then a rescan command will send to the
    *                 LM32 -application.
    */
   FgFeedbackAdministration( DaqAccess* poEbAccess, const bool doRescan = false );

   /*!
    * @brief Destructor:
    * @note If the constructor has opened the wishbone/etherbone- connection
    *       then this destructor will disconnect this,
    *       otherwise it will nothing do.
    */
   virtual ~FgFeedbackAdministration( void );

   /*!
    * @brief Returns the SCU LAN domain name or the name of the wishbone
    *        device.
    */
   const std::string getScuDomainName( void )
   {
      return m_oAddacDaqAdmin.getScuDomainName();
   }

   /*!
    * @brief Returns a pointer to an object of type daq::EbRamAccess
    */
   daq::EbRamAccess* getEbAccess( void )
   {
      return m_oAddacDaqAdmin.getEbAccess();
   }

   /*!
    * @brief Enables or disables the pairing of set and actual values
    *        of non-MIL DAQs by sequence number.
    * @param pairingBySequence If true than the pairing by sequence number
    *                          is active, else the pairing will made by
    *                          the timestamp.
    */
   void setPairingBySequence( const bool pairingBySequence )
   {
      m_oAddacDaqAdmin.setPairingBySequence( pairingBySequence );
   }

   /*!
    * @brief Asks whether the pairing of set and actual values
    *        of non-MIL DAQs  will made by sequence number
    *        or by timestamp.
    */
   bool isPairingBySequence( void )
   {
      return m_oAddacDaqAdmin.isPairingBySequence();
   }

   /*!
    * @brief Returns "true" if the LM32 firmware supports ADDAC/ACU DAQs.
    */
   bool isAddacDaqSupport( void ) const
   {
      return m_oAddacDaqAdmin.isAddacDaqSupport();
   }

   /*!
    * @brief Returns the throttle threshold in DAQ- raw-value
    * @see setThrottleThreshold
    */
   DAQ_T getThrottleThreshold( void ) const
   {
      return m_throttleThreshold >> VALUE_SHIFT;
   }

   /*!
    * @brief Sets the throttle threshold in DAQ raw-value.
    * @see getThrottleThreshold
    */
   void setThrottleThreshold( const DAQ_T throttleThreshold = DEFAULT_THROTTLE_THRESHOLD )
   {
      m_throttleThreshold = throttleThreshold << VALUE_SHIFT;
   }

   /*!
    * @brief Returns the currently throttle timeout in milliseconds.
    * @note A value of zero means the timeout is infinite.
    * @see setThrottleTimeout
    */
   uint getThrottleTimeout( void ) const
   {
      return m_throttleTimeout / daq::NANOSECS_PER_MILISEC;
   }

   /*!
    * @brief Sets the throttle timeout in in milliseconds.
    * @note A value of zero means the timeout is infinite.
    * @see getThrottleTimeout
    */
   void setThrottleTimeout( const uint throttleTimeout = DEFAULT_THROTTLE_TIMEOUT )
   {
      m_throttleTimeout = throttleTimeout * daq::NANOSECS_PER_MILISEC;
   }

   /*!
    * @brief Sets the maximum number of DDR3-RAM payload data base items per etherbone cycle.
    *
    * That means the etherbone-cycle will divided in "maxLen/len" smaller etherbone cycles.
    * @note The value of zero has a special meaning, in this case no divisions
    *       in smaller cycles will made.
    * @note In the case of MIL-devices this function has no effect.
    * @see getMaxEbCycleDataLen
    * @param len Number of smaller cycles, or no divisions if zero.
    */
   void setMaxEbCycleDataLen( const std::size_t len = 0 )
   {
    #ifdef CONFIG_MIL_FG
      m_oMilDaqAdmin.setMaxEbCycleDataLen( len );
    #endif
      m_oAddacDaqAdmin.setMaxEbCycleDataLen( len );
   }

   /*!
    * @brief Returns the maximum number of DDR3-RAM payload data base items per etherbone cycle.
    * Counterpart to setMaxEbCycleDataLen
    * @see setMaxEbCycleDataLen
    * @return Number of divided smaller etherbone cycles or zero.
    */
   std::size_t getMaxEbCycleDataLen( void ) const
   {
      return m_oAddacDaqAdmin.getMaxEbCycleDataLen();
   }

   /*!
    * @brief Sets the waiting time between two etherbone partial blocks in
    *        microseconds.
    * @note In the case of MIL-devices this function has no effect.
    * @see setMaxEbCycleDataLen
    * @see getBlockReadEbCycleTimeUs
    * @param us Waiting time in microseconds.
    */
   void setBlockReadEbCycleTimeUs( const uint us )
   {
    #ifdef CONFIG_MIL_FG
      m_oMilDaqAdmin.setBlockReadEbCycleTimeUs( us );
    #endif
      m_oAddacDaqAdmin.setBlockReadEbCycleTimeUs( us );
   }

   /*!
    * @brief Returns the waiting time in microseconds between two partial blocks in
    *        microseconds.
    * @see setBlockReadEbCycleTimeUs
    * @return Block waiting time in microseconds.
    */
   uint getBlockReadEbCycleTimeUs( void ) const
   {
      return m_oAddacDaqAdmin.getBlockReadEbCycleTimeUs();
   }

#ifdef CONFIG_EB_TIME_MEASSUREMENT
   /*!
    * @ingroup TIME_MEASUREMENT
    * @brief Get the maximum time of a wishbone cycle in microseconds since the
    *        last call of this function or program start.
    * @param rTimestamp Timestamp of the maximum value.
    * @param rDuration  Maximum value in microseconds.
    * @param rSize      Number of transfer data in bytes.
    * @retval UNKNOWN
    * @retval LM32_READ
    * @retval LM32_WRITE
    * @retval DDR3_READ
    */
   WB_ACCESS_T getWbMeasurementMaxTime( daq::USEC_T& rTimestamp, daq::USEC_T& rDuration, std::size_t& rSize )
   {
      return static_cast<WB_ACCESS_T>(getEbAccess()->getWbMeasurementMaxTime( rTimestamp, rDuration, rSize ));
   }

   /*!
    * @ingroup TIME_MEASUREMENT
    * @brief Get the minimum time of a wishbone cycle in microseconds since the
    *        last call of this function or program start.
    * @param rTimestamp Timestamp of the minimum value.
    * @param rDuration  Minimum value in microseconds.
    * @param rSize      Number of transfer data in bytes.
    * @retval UNKNOWN
    * @retval LM32_READ
    * @retval LM32_WRITE
    * @retval DDR3_READ
    */
   WB_ACCESS_T getWbMeasurementMinTime( daq::USEC_T& rTimestamp, daq::USEC_T& rDuration, std::size_t& rSize )
   {
      return static_cast<WB_ACCESS_T>(getEbAccess()->getWbMeasurementMinTime( rTimestamp, rDuration, rSize ));
   }

   /*!
    * @ingroup TIME_MEASUREMENT
    * @brief Static class helper function converts the access constant
    *        (return value of getWbMeasurementMaxTime and
    *        getWbMeasurementMinTime) in a zero terminated ASCII string.
    */
   static const char* accessConstantToString( const WB_ACCESS_T access );
#endif /* #ifdef CONFIG_EB_TIME_MEASSUREMENT */

   /*!
    * @brief Returns the major version number of the
    *        LM32 firmware after a scan has been made.
    */
   uint getLm32SoftwareVersion( void ) const
   {
      return m_oFoundFgs.getLm32SoftwareVersion();
   }

   /*!
    * @brief Returns the maximum capacity of the ADDAC-DAQ data-buffer
    *        in minimum addressable payload units of the used RAM-type.
    */
   uint getAddacBufferCapacity( void )
   {
      return m_oAddacDaqAdmin.getRamCapacity();
   }

   /*!
    * @brief Returns the offset in minimum addressable payload units of the
    *        used RAM type for ADDAC-DAQs.
    */
   uint getAddacBufferOffset( void )
   {
      return m_oAddacDaqAdmin.getRamOffset();
   }

#ifdef CONFIG_MIL_FG
   /*!
    * @brief Returns the maximum capacity of the MIL-DAQ data-buffer
    *        in minimum addressable payload units of the used RAM-type.
    */
   uint getMilBufferCapacity( void )
   {
      return m_oMilDaqAdmin.getRamCapacity();
   }

   /*!
    * @brief Returns the offset in minimum addressable payload units of the
    *        used RAM type for MIL-DAQs.
    */
   uint getMilBufferOffset( void )
   {
      return m_oMilDaqAdmin.getRamOffset();
   }
#endif

   /*!
    * @brief Triggering a software interrupt in LM32 firmware
    * @param opCode Operation code
    * @param param  Optional parameter
    */
   void sendSwi( FG::FG_OP_CODE_T opCode, uint param = 0 )
   {
      m_lm32Swi.send( opCode, param );
   }

   /*!
    * @brief Enabling and adjusting the gap-reading for all
    *        MIL- function generators or disabling the gap reading.
    * @param gapInterval Gap read interval in milliseconds.
    *                    The value of zero disables the gap reading.
    */
   void sendGapReadingInterval( const uint gapInterval = 0 )
   {
      sendSwi( FG::FG_OP_MIL_GAP_INTERVAL, gapInterval );
   }

   /*!
    * @brief Scanning and synchronizing of the function-generator list found
    *        by the LM32 application.
    * @note This function performances a re-scan by the LM32!
    */
   void scan( const bool doRescan = false );

   /*!
    * @brief Synchronizing of the function-generator list found by the
    *        LM32 application.
    */
   void sync( void )
   {
      m_oFoundFgs.sync( m_oAddacDaqAdmin.getEbAccess() );
   }

   /*!
    * @brief Returns a reference to the function generator list.
    */
   FgList& getFgList( void )
   {
      return m_oFoundFgs;
   }

   /*!
    * @brief Returns the number of found MIL function generators after
    *        a scan has been made.
    */
   uint getNumOfFoundMilFg( void )
   {
      return m_oFoundFgs.getNumOfFoundMilFg();
   }

   /*!
    * @brief Returns the number of ADDAC and/or ACO function generators after
    *        a scan has been made.
    */
   uint getNumOfFoundNonMilFg( void )
   {
      return m_oFoundFgs.getNumOfFoundNonMilFg();
   }

   /*!
    * @brief Returns the total number of found function generators after
    *        a scan has been made.
    */
   uint getNumOfFoundFg( void )
   {
      return m_oFoundFgs.getNumOfFoundFg();
   }

   /*!
    * @brief Returns true if function generator with
    *        the given socket and given device number present.
    * @note A scan of function generators before assumed!
    */
   bool isPresent( const uint socket, const uint device )
   {
      return m_oFoundFgs.isPresent( socket, device );
   }

   /*!
    * @brief Returns true if the given socket number is used by a
    *        function generator.
    * @note A scan of function generators before assumed!
    */
   bool isSocketUsed( const uint socket )
   {
      return m_oFoundFgs.isSocketUsed( socket );
   }

   /*!
    * @ingroup REGISTRATION
    * @brief Returns a pointer to a registered device object.
    * @param socket Device number
    * @retval !=nullptr Pointer to the device object
    * @retval ==nullptr Device not present respectively not registered.
    */
   FgFeedbackDevice* getDevice( const uint socket );

   /*!
    * @ingroup REGISTRATION
    * @brief Registering of a device containing function generators.
    * @note If the given device or one of its containing function generators
    *       are not present on the SCU, than an exception will throw.
    */
   void registerDevice( FgFeedbackDevice* poDevice );

   /*!
    * @ingroup REGISTRATION
    * @brief Counterpart to registerDevice
    * @see registerDevice
    */
   void unregisterDevice( FgFeedbackDevice* poDevice );

   /*!
    * @ingroup REGISTRATION
    * @brief Helper function for browsing in the list of registered device objects.
    * @see FgFeedbackDevice::begin
    * @returns Start iterator object of the list of registered
    *          objects of base-type FgFeedbackDevice*.
    */
   const GEN_DEV_LIST_T::iterator begin( void )
   {
      return m_lDevList.begin();
   }

   /*!
    * @ingroup REGISTRATION
    * @brief Helper function for browsing in the list of registered device objects.
    * @see FgFeedbackDevice::end
    * @returns Stop iterator object of the list of registered
    *          objects of base-type FgFeedbackDevice*.
    */
   const GEN_DEV_LIST_T::iterator end( void )
   {
      return m_lDevList.end();
   }

   /*!
    * @brief Prepares all found DAQ-devices on SCU-bus to synchronizing its
    *        timestamp- counter.
    * @note Dummy-function in the case of MIL-DAQs!
    * @param timeOffset Time in milliseconds in which the timing-ECA with
    *                   the tag  ecaTag has to be emitted.
    * @param ecaTag The ECA-tag of the timing event.
    */
   int sendSyncronizeTimestamps( const uint32_t timeOffset = daq::DEFAULT_SYNC_TIMEOFFSET,
                                         const uint32_t ecaTag = daq::DEFAULT_ECA_SYNC_TAG );

   /*!
    * @brief Central polling routine of all feedback channels.
    *
    * This function checks whether data from a DAQ channel in the appropriate
    * shared LM32-memory and - if there - invokes the on "onData" function of
    * the associated channel object.
    *
    * @note This function should run in a polling-loop of a own thread.
    * @return Number of remaining data in the DDR3-RAM which are still not evaluated
    *         by the callback functions "onData".
    */
   uint distributeData( void );

   /*!
    * @brief Makes the data buffer empty.
    * @param update If true the indexes in the LM32 shared memory
    *               becomes updated.
    */
   void clearBuffer( const bool update = true );

   void reset( void );

protected:
   /*!
    * @brief The function is called between divided data blocks for
    *        reading MIL and/or ADDAC DAQ data.
    *
    * When this callback function will not overwritten then a
    * default function will used which invokes the POSIX function
    * usleep() by the parameter m_blockReadEbCycleGapTimeUs,
    * @param isMil If true then the function has been invoked by a
    *              MIL data-transfer.
    */
   virtual void onDataReadingPause( const bool isMil );

#ifdef CONFIG_MIL_FG
   /*!
    * @brief Optional callback function becomes invoked if a unregistered
    *        MIL-device has been detected.
    * @param fg Properties of MIL-device @see FG_MACRO_T
    */
   virtual void onUnregisteredMilDevice( FG_MACRO_T fg ) {}
#endif

   /*!
    * @brief Optional callback function becomes invoked if a unregistered
    *        ADDAC/ACU DAQ device has been detected.
    * @param slot Slot number of SCU-bus
    * @param daqNumber DAQ channel number within the SCU-slave.
    */
   virtual void onUnregisteredAddacDaq( uint slot, uint daqNumber ) {}

   /*!
    * @brief Optional callback function becomes invoked in the case of a
    *        possible loss of one or more ADDAC-DAQ date blocks.
    *
    * This is the case when the expected block sequence number doesn't fit
    * to the received sequence number.
    * @param slot Slot number of SCU-bus
    * @param daqNumber DAQ channel number within the SCU-slave.
    */
   virtual void onAddacBlockError( uint slot, uint daqNumber ) {}

   /*!
    * @brief Optional callback function becomes invoked when a data timeout
    *        has been detected.
    *
    * @param isMil If true then the function has been invoked by a
    *              MIL data-transfer.
    */
   virtual void onDataTimeout( const bool isMil ) {}

   virtual void onDataError( const bool isMil );

   /*!
    * @brief Optional callback function becomes invoked when a erroneous
    *        device descriptor has been received.
    * @note When this function will not overwritten than an exception
    *       becomes triggered if becomes invoked.
    */
   virtual void onErrorDescriptor( const daq::DAQ_DESCRIPTOR_T& roDescriptor );
}; // class FgFeedbackAdministration

/*! ---------------------------------------------------------------------------
 */
inline
void FgFeedbackAdministration::AddacAdministration::onDataReadingPause( void )
{
   m_pParent->onDataReadingPause( false );
}

#ifdef CONFIG_MIL_FG
/*! ---------------------------------------------------------------------------
 */
inline
void FgFeedbackAdministration::MilDaqAdministration::onDataReadingPause( void )
{
   m_pParent->onDataReadingPause( true );
}
#endif

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
inline uint FgFeedbackChannel::getSocket( void )
{
   return getParent()->getSocket();
}



} // End namespace Scu

#endif // ifndef _SCU_FG_FEEDBACK_HPP
//================================== EOF ======================================
