/*!
 * @file scu_fg_feedback.cpp
 * @brief Administration of data aquesition units for function generator
 *        feedback.
 *
 * @date 25.05.2020
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>
 ******************************************************************************
 */
#include <daq_exception.hpp>
#include <scu_fg_feedback.hpp>
#include <daqt_messages.hpp>

using namespace Scu;
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::Common::Common( FgFeedbackChannel* pParent )
   :m_pParent( pParent )
{
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::Common::~Common( void )
{
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::AddacFb::Receive::Receive( AddacFb* pParent, const uint n )
   :daq::DaqChannel( n )
   ,m_pParent( pParent )
   ,m_timestamp( 0 )
   ,m_sampleTime( 0 )
   ,m_blockLen( 0 )
   ,m_sequence( 0 )
{
   assert( n > 0 );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::AddacFb::Receive::~Receive( void )
{
}

/*! ---------------------------------------------------------------------------
 * @brief Storing of a incoming ADDAC/ACU-DAQ data block.
 */
bool FgFeedbackChannel::AddacFb::Receive::onDataBlock( daq::DAQ_DATA_T* pData,
                                                       std::size_t wordLen )
{
   if( !descriptorWasContinuous() )
   {
      return true;
   }

   if( wordLen >= ARRAY_SIZE(m_aBuffer) )
   {
      std::string str = "Size of received data out range. Actual: ";
      str += std::to_string( wordLen );
      str += ", maximum: ";
      str += std::to_string( ARRAY_SIZE(m_aBuffer) );
      throw daq::Exception( str );
   }

   m_blockLen = wordLen;
   m_sequence = descriptorGetSequence();
   m_sampleTime = descriptorGetTimeBase();
   m_timestamp = descriptorGetTimeStamp() - m_sampleTime * m_blockLen;
   ::memcpy( m_aBuffer, pData, m_blockLen * sizeof(daq::DAQ_DATA_T) );
   m_pParent->finalizeBlock();

   return false;
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackChannel::AddacFb::Receive::onInit( void )
{
   if( this == &m_pParent->m_oReceiveActValue )
      m_pParent->m_pParent->onInit();
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackChannel::AddacFb::Receive::onReset( void )
{
   if( this == &m_pParent->m_oReceiveActValue )
      m_pParent->m_pParent->onReset();
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::AddacFb::AddacFb( FgFeedbackChannel* pParent,
                                     daq::DAQ_DEVICE_TYP_T type )
   :Common( pParent )
   ,m_oReceiveSetValue( this, 1 + daq::daqGetSetDaqNumberOfFg( pParent->getFgNumber(), type ) )
   ,m_oReceiveActValue( this, 1 + daq::daqGetActualDaqNumberOfFg( pParent->getFgNumber(), type ) )
{
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::AddacFb::~AddacFb( void )
{
}

/*! ---------------------------------------------------------------------------
 * @brief Forwarding of actual- and set- values to the higher software-layer
 *        once both data blocks has been received.
 */
void FgFeedbackChannel::AddacFb::finalizeBlock( void )
{  /*
    * At the first time one of both channels doesn't received yet,
    * in this case it's block length is still zero.
    */
   if( m_oReceiveSetValue.getBlockLen() == 0 )
      return;
   if( m_oReceiveActValue.getBlockLen() == 0 )
      return;

   DEBUG_MESSAGE( "set sequence: " << (uint)m_oReceiveSetValue.getSequence() );
   DEBUG_MESSAGE( "act sequence: " << (uint)m_oReceiveActValue.getSequence() );
   /*
    * One of both channels has received first, in this case it has to be wait
    * for the second channel.
    * This will accomplished by comparing the sequence numbers.
    */
   if( m_oReceiveSetValue.getSequence() != m_oReceiveActValue.getSequence() )
      return;

   // TODO comparing of both time-stamps.
   /*
    * Safety check: The data length of both blocks have to be equal!
    */
   if( m_oReceiveSetValue.getBlockLen() != m_oReceiveActValue.getBlockLen() )
   {
      std::string str = "Different block sizes received: set-data: ";
      str += std::to_string( m_oReceiveSetValue.getBlockLen() );
      str += " actual data: ";
      str += std::to_string( m_oReceiveActValue.getBlockLen() );
      throw daq::Exception( str );
   }

   /*
    * Forwarding of set- and actual- values to the higher software layer.
    */
   uint64_t timeStamp = m_oReceiveSetValue.getTimestamp();
   for( std::size_t i = 0; i < m_oReceiveSetValue.getBlockLen();
        i++, timeStamp += m_oReceiveSetValue.getSampleTime() )
   {
      constexpr uint SHIFT = BIT_SIZEOF( MiLdaq::MIL_DAQ_T ) -
                             BIT_SIZEOF( daq::DAQ_DATA_T );
      m_pParent->onData( timeStamp,
                         m_oReceiveActValue[i] << SHIFT,
                         m_oReceiveSetValue[i] << SHIFT );
   }
}

#ifdef CONFIG_MIL_FG
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::MilFb::Receive::Receive( MilFb* pParent )
   :MiLdaq::DaqCompare( pParent->m_pParent->getFgNumber() )
   ,m_pParent( pParent )
{
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::MilFb::Receive::~Receive( void )
{
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackChannel::MilFb::Receive::onData( uint64_t wrTimeStampTAI,
                                                MiLdaq::MIL_DAQ_T actlValue,
                                                MiLdaq::MIL_DAQ_T setValue )
{
   /*
    * Just forwarding to the grandpa, that's all.
    */
   m_pParent->m_pParent->onData( wrTimeStampTAI, actlValue, setValue );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackChannel::MilFb::Receive::onInit( void )
{
   m_pParent->m_pParent->onInit();
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackChannel::MilFb::Receive::onReset( void )
{
   m_pParent->m_pParent->onReset();
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::MilFb::MilFb( FgFeedbackChannel* pParent )
   :Common( pParent )
   ,m_oReceive( this )
{
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::MilFb::~MilFb( void )
{
}

#endif // ifdef CONFIG_MIL_FG

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::~FgFeedbackChannel( void )
{
   if( m_pParent != nullptr )
      m_pParent->unregisterChannel( this );

   if( m_pCommon != nullptr )
      delete m_pCommon;
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackDevice* FgFeedbackChannel::getParent( void )
{
   if( m_pParent == nullptr )
   {
      std::string str = "Feedback channel number ";
      str += std::to_string( m_fgNumber );
      str += " isn't registered!";
      throw daq::Exception( str );
   }
   return m_pParent;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackDevice::FgFeedbackDevice( const uint socket )
   :m_poDevice( nullptr )
   ,m_pParent( nullptr )
{
   if( ::isAddacFg( socket ) )
   {
      DEBUG_MESSAGE( "Creating ADDAC-device on slot: " << ::getFgSlotNumber( socket ) );
      m_poDevice = new daq::DaqDevice( socket );
   }
#ifdef CONFIG_MIL_FG
   else if( ::isMilFg( socket ) && (getFgSlotNumber( socket ) <= MAX_SCU_SLAVES))
   {
      DEBUG_MESSAGE( "Creating MIL-device on slot: " << ::getFgSlotNumber( socket ) );
      m_poDevice = new MiLdaq::DaqDevice( socket );
   }
#endif
   else
   {
      std::string str = "Unknown DAQ device type with socket: ";
      str += std::to_string( socket );
      throw daq::Exception( str );
   }

   DEBUG_MESSAGE( typeid(m_poDevice).name() << " created" );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackDevice::~FgFeedbackDevice( void )
{
   if( m_pParent != nullptr )
       m_pParent->unregisterDevice( this );

   for( auto& channel: m_lChannelList )
      channel->m_pParent = nullptr;

   if( m_poDevice != nullptr )
   {
      DEBUG_MESSAGE( "Destructor of " << (m_poDevice->isAddac()? "ADDAC" : "MIL")
                     << "-device on slot: " << m_poDevice->getSlot() );

      delete m_poDevice;
   }
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackAdministration* FgFeedbackDevice::getParent( void )
{
   if( m_pParent == nullptr )
   {
      std::string str = "Feedback device socket number ";
      str += std::to_string( getSocket() );
      str += " isn't registered!";
      throw daq::Exception( str );
   }
   return m_pParent;
}

/*! ---------------------------------------------------------------------------
 * @brief Generates the channel kernel for all registered channels,
 *        depending on the device type if not already done.
 */
void FgFeedbackDevice::generateAll( void )
{
   for( const auto& pFeedbackChannel: m_lChannelList )
   {
      if( pFeedbackChannel->m_pCommon != nullptr )
         continue;
      generate( pFeedbackChannel );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Generates the channel kernel depending on the device type.
 */
void FgFeedbackDevice::generate( FgFeedbackChannel* pFeedbackChannel )
{
   assert( pFeedbackChannel->m_pCommon == nullptr );
   assert( m_pParent != nullptr );

   const daq::DAQ_DEVICE_TYP_T type = getTyp();
   DEBUG_MESSAGE( "generating channel for device type: " << deviceType2String( type ) );

#ifdef CONFIG_MIL_FG
   MiLdaq::DaqDevice* pMilDev = dynamic_cast<MiLdaq::DaqDevice*>(m_poDevice);
   /*
    * Is this object a MIL device?
    */
   if( pMilDev != nullptr )
   {  /*
       * The feedback channel object becomes registered in a MIL device so
       * a MIL feedback object will created.
       */
      pFeedbackChannel->m_pCommon = new FgFeedbackChannel::MilFb( pFeedbackChannel );
      pMilDev->registerDaqCompare( &static_cast<FgFeedbackChannel::MilFb*>(pFeedbackChannel->m_pCommon)->m_oReceive );
      return;
   }
#endif // ifdef CONFIG_MIL_FG

   daq::DaqDevice* pAddacDev = dynamic_cast<daq::DaqDevice*>(m_poDevice);
   /*
    * Here a ADDAC/ACU object is provided.
    */
   assert( pAddacDev != nullptr );


   /*
    * The feedback channel object becomes registered in a ADDAC/ACU device so
    * a ADDAC/ACU feedback object will created.
    */
   pFeedbackChannel->m_pCommon = new FgFeedbackChannel::AddacFb( pFeedbackChannel, type );

   /*
    * Register receive channel for set-values
    */
   pAddacDev->registerChannel( &static_cast<FgFeedbackChannel::AddacFb*>( pFeedbackChannel->m_pCommon )->m_oReceiveSetValue );
   /*
    * Register receive channel for actual-values
    */
   pAddacDev->registerChannel( &static_cast<FgFeedbackChannel::AddacFb*>( pFeedbackChannel->m_pCommon )->m_oReceiveActValue );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackDevice::registerChannel( FgFeedbackChannel* pFeedbackChannel )
{
   if( pFeedbackChannel->m_pParent != nullptr )
   {
      std::string str = "Feedback channel number ";
      str += std::to_string( pFeedbackChannel->getFgNumber() );
      str += " already registered in device ";
      str += std::to_string( pFeedbackChannel->getSocket() );
      throw daq::Exception( str );
   }

   assert( pFeedbackChannel->m_pCommon == nullptr );
   pFeedbackChannel->m_pParent = this;

#ifdef CONFIG_MIL_FG
   /*
    * Is this object a MIL device?
    */
   if( dynamic_cast<MiLdaq::DaqDevice*>(m_poDevice) != nullptr )
   {
      if( (pFeedbackChannel->getFgNumber() >= MAX_FG_MACROS) ||
          (pFeedbackChannel->getFgNumber() == 0) )
      {
         std::string str = "Function generator number for MIL-FG ";
         str += std::to_string( pFeedbackChannel->getFgNumber() );
         str += " is out of range from 1 to <" TO_STRING( MAX_FG_MACROS ) " !";
         throw daq::Exception( str );
      }
   }
   else
#endif // ifdef CONFIG_MIL_FG
   /*
    * Is this object a non-MIL device?
    */
   if( dynamic_cast<daq::DaqDevice*>(m_poDevice) != nullptr )
   {
      if( pFeedbackChannel->getFgNumber() >= MAX_FG_PER_SLAVE )
      {
         std::string str = "Function generator number for ADDAC/ACU-FG ";
         str += std::to_string( pFeedbackChannel->getFgNumber() );
         str += " is out of range from 0 to <" TO_STRING( MAX_FG_PER_SLAVE ) " !";
         throw daq::Exception( str );
      }
   }
   else
   {
      assert( false );
   }

   if( m_pParent != nullptr )
      generate( pFeedbackChannel );

   m_lChannelList.push_back( pFeedbackChannel );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackDevice::unregisterChannel( FgFeedbackChannel* pFeedbackChannel )
{
   if( pFeedbackChannel->m_pParent != this )
      return;

   m_lChannelList.remove( pFeedbackChannel );
   pFeedbackChannel->m_pParent = nullptr;
   DEBUG_MESSAGE( "Channel fg-" << getSocket() << '-'
                                << pFeedbackChannel->getFgNumber()
                                << " unregistered!" );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel* FgFeedbackDevice::getChannel( const uint number )
{
   for( const auto& i: m_lChannelList )
   {
      if( i->getFgNumber() == number )
         return i;
   }
   return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackAdministration::FgFeedbackAdministration( DaqEb::EtherboneConnection* poEtherbone,
                                                    const bool doRescan )
   :m_oAddacDaqAdmin( this, poEtherbone )
#ifdef CONFIG_MIL_FG
   ,m_oMilDaqAdmin( this, m_oAddacDaqAdmin.getEbAccess() )
#endif
   ,m_lm32Swi( m_oAddacDaqAdmin.getEbAccess() )
{
   scan( doRescan );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackAdministration::FgFeedbackAdministration( daq::EbRamAccess* poEbAccess,
                                                    const bool doRescan )
   :m_oAddacDaqAdmin( this, poEbAccess )
#ifdef CONFIG_MIL_FG
  ,m_oMilDaqAdmin( this, poEbAccess )
#endif
  ,m_lm32Swi( poEbAccess )
{
   scan( doRescan );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackAdministration::~FgFeedbackAdministration( void )
{
   for( const auto& dev: m_lDevList )
      unregisterDevice( dev );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::scan( const bool doRescan )
{
   m_vPollList.clear();

   if( doRescan )
      m_oFoundFgs.scan( &m_lm32Swi );
   else
      m_oFoundFgs.sync( getEbAccess() );

#ifdef CONFIG_MIL_FG
   if( getNumOfFoundMilFg() != 0 )
      m_vPollList.push_back( &m_oMilDaqAdmin );
#endif
   if( getNumOfFoundNonMilFg() != 0 )
      m_vPollList.push_back( &m_oAddacDaqAdmin );

   m_vPollList.shrink_to_fit();
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::registerDevice( FgFeedbackDevice* poDevice )
{
   assert( poDevice->m_pParent == nullptr );

   if( !isSocketUsed( poDevice->getSocket() ) )
   {
      std::string str = "Device on socket ";
      str += std::to_string( poDevice->getSocket() );
      str += " not present!";
      throw daq::Exception( str );
   }

   if( poDevice->isAddac() )
   {
      m_oAddacDaqAdmin.registerDevice( poDevice->getAddac() );
   }
#ifdef CONFIG_MIL_FG
   else if( poDevice->isMil() )
   {
      m_oMilDaqAdmin.registerDevice( poDevice->getMil() );
   }
#endif
   else
   {
      assert( false );
   }
   poDevice->m_pParent = this;
   poDevice->generateAll();
   m_lDevList.push_back( poDevice );
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::unregisterDevice( FgFeedbackDevice* poDevice )
{
   if( poDevice->m_pParent != this )
      return;

   m_lDevList.remove( poDevice );
   poDevice->m_pParent = nullptr;
   DEBUG_MESSAGE( "Feedback device " << poDevice->getSocket() << " unregistered!" );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackDevice* FgFeedbackAdministration::getDevice( const uint socket )
{
   for( const auto& i: m_lDevList )
   {
      if( i->getSocket() == socket )
         return i;
   }
   return nullptr;
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::distributeData( void )
{
#ifdef __DOXYGEN__
 /*
  * Necessary for Doxygen - caller graph,
  * it's not a part of the resulting binary.
  */
 #ifdef CONFIG_MIL_FG
   MiLdaq::DaqAdministration::distributeData();
 #endif
   daq::DaqAdministration::distributeData();
#endif

   for( const auto& poDaqAdmin: m_vPollList )
      poDaqAdmin->distributeData();
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::reset( void )
{
#ifdef __DOXYGEN__
 /*
  * Necessary for Doxygen - caller graph,
  * it's not a part of the resulting binary.
  */
 #ifdef CONFIG_MIL_FG
   MiLdaq::DaqAdministration::reset();
 #endif
   daq::DaqAdministration::reset();
#endif

   for( const auto& poDaqAdmin: m_vPollList )
      poDaqAdmin->reset();
}

//================================== EOF ======================================
