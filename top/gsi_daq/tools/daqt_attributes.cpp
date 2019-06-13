/*!
 *  @file daqt_attributes.cpp
 *  @brief Administration of attributes which has been set by
 *         the command line parser
 *
 *  @date 24.05.2019
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
#include <daqt_attributes.hpp>
using namespace Scu;
using namespace daq;
using namespace daqt;

///////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------------------------
 */
void Attributes::set( const Attributes& rMyContainer )
{
   #define __SET_MEMBER( member )  member.set( rMyContainer.member )
   __SET_MEMBER( m_highResolution );
   __SET_MEMBER( m_postMortem );
   __SET_MEMBER( m_continueMode );
   __SET_MEMBER( m_continueTriggerSouce );
   __SET_MEMBER( m_highResTriggerSource );
   __SET_MEMBER( m_triggerEnable );
   __SET_MEMBER( m_triggerDelay );
   __SET_MEMBER( m_triggerCondition );
   __SET_MEMBER( m_blockLimit );
   __SET_MEMBER( m_restart );
   __SET_MEMBER( m_zoomGnuPlot );
   #undef __SET_MEMBER
}

//================================== EOF ======================================
