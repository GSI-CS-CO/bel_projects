/*!
 *  @file daqt_messages.hpp
 *  @brief Macros for Error and warning messages.
 *
 *  @date 17.04.2019
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
#ifndef _DAQT_MESSAGES_HPP
#define _DAQT_MESSAGES_HPP

#include <iostream>
#include <eb_console_helper.h>

#define ERROR_MESSAGE( args... )                                              \
   std::cerr << ESC_FG_RED ESC_BOLD "ERROR: "                                 \
             << args << ESC_NORMAL << std::endl

#define WARNING_MESSAGE( args... )                                            \
   std::cerr << ESC_FG_YELLOW ESC_BOLD "WARNING: "                            \
             << args << ESC_NORMAL << std::endl

#ifdef DEBUGLEVEL
   #include <cxxabi.h>
   #include <string>

   #define DEBUG_MESSAGE( args... )                                           \
      std::cerr << ESC_FG_YELLOW "DBG: "                                      \
                << args << ESC_NORMAL << std::endl

   #define DEBUG_MESSAGE_FUNCTION( args... )                                  \
   {                                                                          \
      std::string __f = __FILE__;                                             \
      DEBUG_MESSAGE( __FUNCTION__ <<  "(" << args << ")"                      \
                     << " line: " << __LINE__                                 \
                     << " file: " << __f.substr(__f.find_last_of('/')+1));    \
   }

   #define DEBUG_MESSAGE_M_FUNCTION( args... )                                \
   {                                                                          \
      std::string __f = __FILE__;                                             \
      int __s;                                                                \
      std::string __c = abi::__cxa_demangle( typeid(this).name(),             \
                                             nullptr, nullptr, &__s );        \
      __c.pop_back();                                                         \
      DEBUG_MESSAGE( __c << "::" << __FUNCTION__ << "(" << args << ")"        \
                     << " line: " << __LINE__                                 \
                     << " file: " << __f.substr(__f.find_last_of('/')+1));    \
   }
#else
   #define DEBUG_MESSAGE( args... )
   #define DEBUG_MESSAGE_FUNCTION( args... )
   #define DEBUG_MESSAGE_M_FUNCTION( args... )
#endif

#endif // ifndef _DAQT_MESSAGES_HPP
//================================== EOF ======================================
