/*!
 *  @file watchdog_poll.cpp
 *  @brief Simple watchdog timer for polling routines.
 *
 *  @date 02.11.2021
 *  @copyright (C) 2021 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>
 ******************************************************************************
 */
#include <daq_calculations.hpp>
#include "watchdog_poll.hpp"

using namespace Scu;

/*! ---------------------------------------------------------------------------
 * @see watchdog_poll.hpp
 */
void Watchdog::start( const uint64_t timeout )
{
   if( timeout != 0 )
      m_timeout = timeout;

   m_currentLimit = m_timeout + daq::getSysMicrosecs();
}

/*! ---------------------------------------------------------------------------
 * @see watchdog_poll.hpp
 */
bool Watchdog::isBarking( void )
{
   if( !isActive() )
      return false;

   if( daq::getSysMicrosecs() < m_currentLimit )
      return false;

   stop();
   return true;
}

//================================== EOF ======================================
