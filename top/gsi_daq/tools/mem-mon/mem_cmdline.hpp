/*!
 *  @file mem_cmdline.hpp
 *  @brief Command-line interpreter of memory monitor.
 *
 *  @date 12.04.2022
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
#ifndef _MEM_CMDLINE_HPP
#define _MEM_CMDLINE_HPP

#include <parse_opts.hpp>

namespace Scu
{
namespace mmu
{

///////////////////////////////////////////////////////////////////////////////
class CommandLine: public CLOP::PARSER
{
   using OPT_LIST_T = std::vector<CLOP::OPTION>;
   static OPT_LIST_T c_optList;

   bool              m_verbose;
   bool              m_tagInDecimal;
   bool              m_isOnScu;
   bool              m_isInBytes;
   std::string       m_scuUrl;

public:
   CommandLine( int argc, char** ppArgv );
   virtual ~CommandLine( void );

   std::string& operator()( void );

   bool isVerbose( void )
   {
      return m_verbose;
   }

   bool isTagInDecimal( void )
   {
      return m_tagInDecimal;
   }

   bool isRunningOnScu( void )
   {
      return m_isOnScu;
   }

   bool isInBytes( void )
   {
      return m_isInBytes;
   }

   std::string& getScuUrl( void )
   {
      return m_scuUrl;
   }

private:
   int onArgument( void ) override;
   int onErrorUnrecognizedShortOption( char unrecognized ) override;
   int onErrorUnrecognizedLongOption( const std::string& unrecognized ) override;

}; // class CommandLine

} // namespace Scu
} // namespace mmu

#endif // ifndef _MEM_CMDLINE_HPP
//================================== EOF ======================================
