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
public:
   using      FILTER_FLAG_T = uint32_t;

private:
   using OPT_LIST_T = std::vector<CLOP::OPTION>;
   static OPT_LIST_T c_optList;

   bool          m_verbose;
   const bool    m_isOnScu;
   bool          m_noTimestamp;
   bool          m_humanTimestamp;
   bool          m_isForConsole;
   bool          m_escSequences;
   bool          m_printFilter;
   bool          m_exit;
   bool          m_kill;
   bool          m_killOnly;
   bool          m_isDaemonized;
   bool          m_addBuildId;
   bool          m_readBuildId;
   uint          m_interval;
   uint          m_maxItemsPerInterval;
   FILTER_FLAG_T m_filterFlags;
   std::string   m_scuUrl;
   std::string   m_logFile;

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
      return m_isDaemonized;
   }

   bool isRuningOnScu( void )
   {
      return m_isOnScu;
   }

   bool isNoTimestamp( void )
   {
      return m_noTimestamp;
   }

   bool isHumanReadableTimestamp( void )
   {
      return m_humanTimestamp;
   }

   bool isForConsole( void )
   {
      return m_isForConsole;
   }

   bool isAllowedEscSequences( void )
   {
      return m_escSequences;
   }

   bool isPrintFilter( void )
   {
      return m_printFilter;
   }

   bool isSingleShoot( void )
   {
      return m_exit;
   }

   bool isKill( void )
   {
      return m_kill;
   }

   bool isKillOnly( void )
   {
      return m_killOnly;
   }

   bool isAddBuildId( void )
   {
      return m_addBuildId;
   }

   bool isReadBuildId( void )
   {
      return m_readBuildId;
   }

   uint getPollInterwalTime( void )
   {
      return m_interval;
   }

   uint getMaxItems( void )
   {
      return m_maxItemsPerInterval;
   }

   std::string& getScuUrl( void )
   {
      return m_scuUrl;
   }

   std::string& getLogfileName( void )
   {
      return m_logFile;
   }

   FILTER_FLAG_T getFilterFlags( void )
   {
      return m_filterFlags;
   }

private:
   int onArgument( void ) override;
   int onErrorUnrecognizedShortOption( char unrecognized ) override;
   int onErrorUnrecognizedLongOption( const std::string& unrecognized ) override;
   int onErrorShortMissingRequiredArg( void ) override;
   int onErrorLongMissingRequiredArg( void ) override;
   int onErrorShortOptionalArg( void ) override;
   int onErrorlongOptionalArg( void ) override;
};

} // namespace Scu
#endif // ifndef _LOGD_CMDLINE_HPP
//================================== EOF ======================================
