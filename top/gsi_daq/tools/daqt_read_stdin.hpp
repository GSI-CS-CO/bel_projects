/*!
 *  @file daqt_read_stdin.hpp
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
#ifndef _DAQT_READ_STDIN_HPP
#define _DAQT_READ_STDIN_HPP
#include <unistd.h>
#include <termios.h>

///////////////////////////////////////////////////////////////////////////////
class Terminal
{
   struct termios m_originTerminal;

public:
   Terminal( void );
   ~Terminal( void );
   void reset( void );
   static int readKey( void );
};

#endif // ifndef _DAQT_READ_STDIN_HPP
//================================== EOF ======================================

