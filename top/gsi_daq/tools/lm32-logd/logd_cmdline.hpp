/*!
 *  @file logd_cmdline.hpp
 *  @brief Commandline parser for the LM32 log daemon.
 *
 *  @date 21.04.2022
 *  @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#ifndef _LOGD_CMDLINE_HPP
#define _LOGD_CMDLINE_HPP

#include <parse_opts.hpp>

namespace Scu
{

///////////////////////////////////////////////////////////////////////////////
class CommandLine: public CLOP::PARSER
{
   using OPT_LIST_T = std::vector<CLOP::OPTION>;
   static OPT_LIST_T c_optList;

   bool        m_verbose;
   bool        m_daemonize;
   const bool  m_isOnScu;
   bool        m_noTimestamp;
   uint        m_interval;
   std::string m_scuUrl;

   static bool readInteger( uint&, const std::string& );

public:
   CommandLine( int argc, char** ppArgv );
   virtual ~CommandLine( void );

   std::string& operator()( void );

   bool isVerbose( void )
   {
      return m_verbose;
   }

   bool isDemonize( void )
   {
      return m_daemonize;
   }

   bool isRuningOnScu( void )
   {
      return m_isOnScu;
   }

   bool isNoTimestamp( void )
   {
      return m_noTimestamp;
   }

   uint getIntervalTime( void )
   {
      return m_interval;
   }

   std::string& getScuUrl( void )
   {
      return m_scuUrl;
   }

private:
   int onArgument( void ) override;
   int onErrorUnrecognizedShortOption( char unrecognized ) override;
   int onErrorUnrecognizedLongOption( const std::string& unrecognized ) override;
};

} // namespace Scu
#endif // ifndef _LOGD_CMDLINE_HPP
//================================== EOF ======================================
