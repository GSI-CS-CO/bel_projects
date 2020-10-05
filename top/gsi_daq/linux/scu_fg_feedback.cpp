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

using namespace Scu;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgFeedbackDevice::FgFeedbackDevice( const uint socket )
   :m_pAddacDevice( nullptr )
#ifdef CONFIG_MIL_FG
   ,m_pMilDevice( nullptr )
#endif
{
   if( ::isAddacFg( socket ) )
      m_pAddacDevice = new daq::DaqDevice( socket );
   else if( ::isMilFg( socket ) )
      m_pMilDevice = new MiLdaq::DaqDevice( socket );
   else
      throw daq::Exception( "Unknown DAQ device type" );
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackDevice::~FgFeedbackDevice( void )
{
   if( m_pAddacDevice != nullptr )
      delete m_pAddacDevice;

#ifdef CONFIG_MIL_FG
   if( m_pMilDevice != nullptr )
      delete m_pMilDevice;
#endif
}

/*! ---------------------------------------------------------------------------
 */
void FgFeedbackDevice::registerChannel( FgFeedbackChannel* pFeedbackChannel )
{
   assert( pFeedbackChannel != nullptr );
#ifdef CONFIG_MIL_FG
   if( m_pMilDevice != nullptr )
   {
      //TODO
      return;
   }
#endif
   assert( m_pAddacDevice != nullptr );
}

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
uint FgFeedbackAdministration::distributeData( void )
{
   for( const auto& poDaqAdmin: m_vPollList )
      poDaqAdmin->distributeData();
   return 0;
}

//================================== EOF ======================================
