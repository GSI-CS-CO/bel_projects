/*!
 *  @file daqt_read_stdin.cpp
 *  @brief Class Terminal for reading key input events.
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
#include <stdlib.h>
#include <daqt_read_stdin.hpp>
#include <daqt_messages.hpp>

/*! ---------------------------------------------------------------------------
 */
Terminal::Terminal( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");

   struct termios newTerminal;

   ::tcgetattr( STDIN_FILENO, &m_originTerminal );
   newTerminal = m_originTerminal;
   newTerminal.c_lflag     &= ~(ICANON | ECHO);  /* Disable canonic mode and echo.*/
   newTerminal.c_cc[VMIN]  = 1;  /* Reading is complete after one byte only. */
   newTerminal.c_cc[VTIME] = 0; /* No timer. */
   ::tcsetattr( STDIN_FILENO, TCSANOW, &newTerminal );
}

/*! ---------------------------------------------------------------------------
 */
Terminal::~Terminal( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
   ::tcsetattr( STDIN_FILENO, TCSANOW, &m_originTerminal );
}

/*! ---------------------------------------------------------------------------
 */
void Terminal::reset( void )
{
   ::tcsetattr( STDIN_FILENO, TCSANOW, &m_originTerminal );
}

/*! ---------------------------------------------------------------------------
 */
int Terminal::readKey( void )
{
   int inKey;
   fd_set rfds;

   struct timeval sleepTime = {0, 10};
   FD_ZERO( &rfds );
   FD_SET( STDIN_FILENO, &rfds );

   if( ::select( STDIN_FILENO+1, &rfds, nullptr, nullptr, &sleepTime ) > 0 )
      ::read( STDIN_FILENO, &inKey, sizeof( inKey ) );
   else
      inKey = 0;

   return (inKey & 0xFF);
}

//================================== EOF ======================================
