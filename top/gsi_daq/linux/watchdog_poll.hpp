/*!
 *  @file watchdog_poll.hpp
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
#ifndef _WATCHDOG_POLL_HPP
#define _WATCHDOG_POLL_HPP

#include <stdint.h>

namespace Scu
{

///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Very simple watchdog timer for including in polling loops.
 */
class Watchdog
{
   uint64_t  m_timeout;
   uint64_t  m_currentLimit;

public:
   /*!
    * @brief Constructor of watchdog object
    * @param timeout The watchdogs watching time in microseconds.
    */
   Watchdog( const uint64_t timeout = 0 ):
      m_timeout( timeout ),
      m_currentLimit( 0 )
   {}

   ~Watchdog( void ) {}

   /*!
    * @brief Starts the watchdog timer.
    *
    * This function has to be in the initialization part outside
    * of the polling loop.
    *
    * @param timeout The watchdogs watching time in microseconds.
    */
   void start( const uint64_t timeout = 0 );

   /*!
    * @brief Returns true if the watchdog is barking, that means when the
    *        timeout has been occurred.
    * 
    * This Function has to be periodically invoked within a polling loop.
    */
   bool isBarking( void );

   /*!
    * @brief Returns true if the watchdog is watching.
    */
   bool isActive( void ) const
   {
      return (m_currentLimit != 0);
   }

   /*!
    * @brief Deactivates the watchdog.
    */
   void stop( void )
   {
      m_currentLimit = 0;
   }

   /*!
    * @brief Returns the watchdogs watching time in microseconds.
    */
   uint64_t getWatchTime( void ) const
   {
      return m_timeout;
   }
};

} // namespace Scu
#endif // ifdef _WATCHDOG_POLL_HPP
//================================== EOF ======================================
