/*!
 *  @file fgw_commandline.hpp
 *  @brief Commandline parser for program fg-wave
 *
 *  @date 10.12.2020
 *  @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#ifndef _FGW_COMMANDLINE_HPP
#define _FGW_COMMANDLINE_HPP
#include <fgw_parser.hpp>
#include <parse_opts.hpp>

namespace fgw
{

#ifndef DEFAULT_LINE_STYLE
   #define DEFAULT_LINE_STYLE "lines"
#endif

   
///////////////////////////////////////////////////////////////////////////////
class CommandLine: CLOP::PARSER
{
   using OPT_LIST_T = std::vector<CLOP::OPTION>;
   static OPT_LIST_T c_optList;

   bool              m_verbose;
   bool              m_zoomYAxis;
   bool              m_noSquareTerm;
   bool              m_noLinearTerm;
   bool              m_doStrip;
   bool              m_doInfo;
   bool              m_doQuit;
   uint              m_repetitions;
   uint              m_dotsPerTuple;

   std::string       m_gnuplotBin;
   std::string       m_gnuplotTerminal;
   std::string       m_gnuplotOutput;
   std::string       m_gnuplotLineStyle;
   std::string       m_gnuplotCoeffCLineStyle;

   std::string       m_fileName;
   std::ifstream*    m_pInStream;  

   static bool readInteger( uint&, const std::string& );

public:
   CommandLine( int argc, char** ppArgv );
   virtual ~CommandLine( void );

   bool isVerbose( void )
   {
      return m_verbose;
   }
   
   bool isZoomYAxis( void ) const
   {
      return m_zoomYAxis;
   }
   
   bool isNoSquareTerm( void ) const
   {
      return m_noSquareTerm;
   }

   bool isNoLinearTerm( void ) const
   {
      return m_noLinearTerm;
   }

   bool isDoStrip( void ) const
   {
      return m_doStrip;
   }

   bool isDoInfo( void ) const
   {
      return m_doInfo;
   }

   bool isDoQuit( void ) const
   {
      return m_doQuit;
   }

   bool isPlotCoeffC( void ) const
   {
      return !m_gnuplotCoeffCLineStyle.empty();
   }

   uint getRepetitions( void ) const
   {
      return m_repetitions;
   }
   
   uint getDotsPerTuple( void ) const
   {
      return m_dotsPerTuple;
   }
   
   const std::string& getLineStyle( void )
   {
      return m_gnuplotLineStyle;
   }
   
   const std::string& getCoeffCLineStyle( void )
   {
      return m_gnuplotCoeffCLineStyle;
   }
   
   const std::string& getGnuplotTerminal( void )
   {
      return m_gnuplotTerminal;
   }
   
   const std::string& getGnuplotBinary( void )
   {
      return m_gnuplotBin;
   }

   const std::string& getFileName( void )
   {
      return m_fileName;
   }
   
   std::istream& getIStream( void );
   std::istream* operator()( void );
   
private:
   int onArgument( void ) override;
};

} // namespace fgw
#endif // ifndef _FGW_COMMANDLINE_HPP
//================================== EOF ======================================
