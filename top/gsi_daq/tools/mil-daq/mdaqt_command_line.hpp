/*!
 *  @file mdaqt_command_line.hpp
 *  @brief Command line parser of MIL-DAQ-Test
 *
 *  @date 26.08.2019
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
#ifndef _MDAQT_COMMAND_LINE_HPP
#define _MDAQT_COMMAND_LINE_HPP
#ifndef __DOCFSM__
 #include "daqt_messages.hpp"
 #include "parse_opts.hpp"
 #include "mdaqt.hpp"
 #include "gnuplotstream.hpp"
#endif

using namespace CLOP;

namespace Scu
{
namespace MiLdaq
{
namespace MiLdaqt
{

///////////////////////////////////////////////////////////////////////////////
class CommandLine: public PARSER
{
   static std::vector<OPTION> c_optList;
   bool                       m_verbose;

   std::string    m_gnuplotBin;
   std::string    m_gnuplotTerminal;
   std::string    m_gnuplotOutput;

   static bool readInteger( unsigned int&, const std::string& );

public:
   CommandLine( int argc, char** ppArgv );
   virtual ~CommandLine( void );

   int onArgument( void ) override;

   bool isVerbose( void ) const
   {
      return m_verbose;
   }

   const std::string& getGnuplotBinary( void )
   {
      return m_gnuplotBin;
   }

   const std::string& getTerminal( void )
   {
      return m_gnuplotTerminal;
   }

   const std::string& getOutputName( void )
   {
      return m_gnuplotOutput;
   }

   bool isOutputFileDefined( void )
   {
      return !m_gnuplotOutput.empty();
   }

};

} // namespace MiLdaqt
} // namespace MiLdaq
} // namespace Scu


#endif
// ================================= EOF ======================================
