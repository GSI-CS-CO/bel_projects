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
   ,m_sequence( 0 )
{
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::AddacFb::Receive::~Receive( void )
{
}

/*! ---------------------------------------------------------------------------
 */
bool FgFeedbackChannel::AddacFb::Receive::onDataBlock( daq::DAQ_DATA_T* pData,
                                                       std::size_t wordLen )
{
   m_sequence = descriptorGetSequence();
   m_timestamp = descriptorGetTimeStamp();
   //TODO
   for( std::size_t i = 0; i < wordLen; i++ )
   {
   }
   m_pParent->finalizeBlock( this );
   return false;
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::AddacFb::AddacFb( FgFeedbackChannel* pParent )
   :Common( pParent )
   ,m_oReceiveSetValue( this, daq::daqGetSetDaqNumberOfFg( pParent->getFgNumber() ) )
   ,m_oReceiveActValue( this, daq::daqGetActualDaqNumberOfFg( pParent->getFgNumber() ) )
{
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackChannel::AddacFb::~AddacFb( void )
{
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackChannel::AddacFb::finalizeBlock( Receive* pReceive )
{
   //TODO
   if( m_oReceiveSetValue.getTimestamp() == 0 )
      return;
   if( m_oReceiveActValue.getTimestamp() == 0 )
      return;
   if( m_oReceiveSetValue.getSequence() != m_oReceiveActValue.getSequence() )
      return;
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
   MiLdaq::DaqDevice* pMilDev = dynamic_cast<MiLdaq::DaqDevice*>(m_poDevice);
   /*
    * Is this object a MIL device?
    */
   if( pMilDev != nullptr )
   {
      if( (pFeedbackChannel->getFgNumber() >= MAX_FG_MACROS) ||
          (pFeedbackChannel->getFgNumber() == 0) )
      {
         std::string str = "Function generator number for MIL-FG ";
         str += std::to_string( pFeedbackChannel->getFgNumber() );
         str += " is out of range from 1 to <" TO_STRING( MAX_FG_MACROS ) " !";
         throw daq::Exception( str );
      }
      /*
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
    * Here a ADDAC/ACU object is assumed.
    */
   assert( pAddacDev != nullptr );
   if( pFeedbackChannel->getFgNumber() >= MAX_FG_PER_SLAVE )
   {
      std::string str = "Function generator number for ADDAC/ACU-FG ";
      str += std::to_string( pFeedbackChannel->getFgNumber() );
      str += " is out of range from 0 to <" TO_STRING( MAX_FG_PER_SLAVE ) " !";
      throw daq::Exception( str );
   }

   /*
    * The feedback channel object becomes registered in a ADDAC/ACU device so
    * a ADDAC/ACU feedback object will created.
    */
   pFeedbackChannel->m_pCommon = new FgFeedbackChannel::AddacFb( pFeedbackChannel );

   /*
    * Register receive channel for set-values
    */
   pAddacDev->registerChannel( &static_cast<FgFeedbackChannel::AddacFb*>( pFeedbackChannel->m_pCommon )->m_oReceiveSetValue );
   /*
    * Register receive channel for actual-values
    */
   pAddacDev->registerChannel( &static_cast<FgFeedbackChannel::AddacFb*>( pFeedbackChannel->m_pCommon )->m_oReceiveActValue );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackAdministration::FgFeedbackAdministration( DaqEb::EtherboneConnection* poEtherbone )
   :m_oAddacDaqAdmin( this, poEtherbone )
#ifdef CONFIG_MIL_FG
   ,m_oMilDaqAdmin( this, m_oAddacDaqAdmin.getEbAccess() )
#endif
{
   scan();
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackAdministration::FgFeedbackAdministration( daq::EbRamAccess* poEbAccess )
   :m_oAddacDaqAdmin( this, poEbAccess )
#ifdef CONFIG_MIL_FG
  ,m_oMilDaqAdmin( this, poEbAccess )
#endif
{
   scan();
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackAdministration::~FgFeedbackAdministration( void )
{
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackAdministration::scan( void )
{
   m_vPollList.clear();
   m_oFoundFgs.scan( getEbAccess() );

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
//TODO
   poDevice->m_pParent = this;
}

/*! ---------------------------------------------------------------------------
 */
uint FgFeedbackAdministration::distributeData( void )
{
   for( const auto& poDaqAdmin: m_vPollList )
      poDaqAdmin->distributeData();
   return 0;
}

//================================== EOF ======================================
