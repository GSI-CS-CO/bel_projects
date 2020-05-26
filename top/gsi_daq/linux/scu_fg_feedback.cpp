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
#include <scu_fg_feedback.hpp>

/*! ---------------------------------------------------------------------------
 */
FgFeedbackAdministration::FgFeedbackAdministration( DaqEb::EtherboneConnection* poEtherbone )
   :m_oAddacDaqAdmin( poEtherbone, false, false )
#ifdef CONFIG_MIL_FG
   ,m_oMilDaqAdmin( m_oAddacDaqAdmin.getEbAccess() )
#endif
{
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackAdministration::FgFeedbackAdministration( daq::EbRamAccess* poEbAccess )
   :m_oAddacDaqAdmin( poEbAccess, false, false )
#ifdef CONFIG_MIL_FG
   ,m_oMilDaqAdmin( poEbAccess )
#endif
{
}

/*! ---------------------------------------------------------------------------
 */
FgFeedbackAdministration::~FgFeedbackAdministration( void )
{
}

/*! ---------------------------------------------------------------------------
 */
uint FgFeedbackAdministration::distributeData( void )
{
   for( const auto& daqAdmin: m_vPollList )
      daqAdmin->distributeData();
   return 0;
}

//================================== EOF ======================================
