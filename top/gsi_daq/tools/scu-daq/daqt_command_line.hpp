/*!
 *  @file daqt_command_line.hpp
 *  @brief Command line parser of DAQ-Test
 *
 *  @date 16.04.2019
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
#ifndef _DAQT_COMMAND_LINE_HPP
#define _DAQT_COMMAND_LINE_HPP
#ifndef __DOCFSM__
 #include "daqt_messages.hpp"
 #include "parse_opts.hpp"
 #include "daqt.hpp"
 #include "gnuplotstream.hpp"
#endif

using namespace CLOP;

namespace Scu
{
namespace daq
{
namespace daqt
{

#define FSM_DECLARE_STATE( state, attr... ) state
#define FSM_INIT_FSM( state, attr... )      m_state( state )
#define FSM_TRANSITION( newState, attr... ) m_state = newState

///////////////////////////////////////////////////////////////////////////////
class CommandLine: public PARSER
{
   enum STATE_T
   {
      FSM_DECLARE_STATE( READ_EB_NAME ),
      FSM_DECLARE_STATE( READ_SLOT ),
      FSM_DECLARE_STATE( READ_CHANNEL )
   };
   static std::vector<OPTION> c_optList;

   STATE_T        m_state;
   DaqContainer*  m_poAllDaq;
   Device*        m_poCurrentDevice;
   Channel*       m_poCurrentChannel;
   bool           m_verbose;
   bool           m_noReset;
   bool           m_noCommand;
   std::string    m_gnuplotBin;
   std::string    m_gnuplotTerminal;
   std::string    m_gnuplotOutput;

   static bool readInteger( unsigned int&, const std::string& );

public:
   static void specifiedBeforeErrorMessage( void );
   static DaqAdministration* getDaqAdmin( PARSER* poParser );

   const bool isLM32CommandsEnabled( void ) const
   {
      return !m_noCommand;
   }

   const bool isNoReset( void ) const
   {
      return m_noReset;
   }

   CommandLine( int argc, char** ppArgv );
   virtual ~CommandLine( void );

   DaqContainer* operator()(void);

   Attributes* getAttributesToSet( void );

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

} // namespace daqt
}
}
#endif // _DAQT_COMMAND_LINE_HPP
//================================== EOF ======================================
