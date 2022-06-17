/*!
 *  @file fb_command_line.hpp
 *  @brief Command line parser of MIL-DAQ-Test
 *
 *  @date 09.10.2020
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
#ifndef _FB_COMMAND_LINE_HPP
#define _FB_COMMAND_LINE_HPP

#ifndef __DOCFSM__
 #include "daqt_messages.hpp"
 #include "parse_opts.hpp"
 #include "fg-feedback.hpp"
 #include "gnuplotstream.hpp"
 #include "scu_fg_feedback.hpp"
#endif

using namespace CLOP;

namespace Scu
{

#ifdef FSM_DECLARE_STATE
   #undef  FSM_DECLARE_STATE
#endif
#define FSM_DECLARE_STATE( state, attr... ) state

#ifndef DEFAULT_X_AXIS_LEN
   #define DEFAULT_X_AXIS_LEN 10.0
#endif

#ifndef DEFAULT_LINE_STYLE
   #define DEFAULT_LINE_STYLE "lines"
#endif

#ifndef DEFAULT_PLOT_INTERVAL
  #define DEFAULT_PLOT_INTERVAL 5
#endif
#if DEFAULT_PLOT_INTERVAL == 0
  #error DEFAULT_PLOT_INTERVAL shall not be zer0!
#endif

class AllDaqAdministration; // Loest Henne-Ei Problem...
class FbChannel;

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
   STATE_T                    m_state;
   bool                       m_targetUrlGiven;
   uint                       m_numDevs;
   uint                       m_numChannels;
   bool                       m_optionError;
   bool                       m_verbose;
   bool                       m_autoBuilding;
   bool                       m_deviationEnable;
   bool                       m_continuePlotting;
   bool                       m_plotAlwaysSetValue;
   bool                       m_doClearBuffer;
   bool                       m_zoomYAxis;
   bool                       m_exitOnError;
   bool                       m_noPlot;
   bool                       m_pairingBySequence;
   float                      m_xAxisLen;
   uint                       m_plotInterval;
   uint                       m_throttleThreshold;
   uint                       m_throttleTimeout;
   uint                       m_maxEbCycleDataLen;
   uint                       m_blockReadEbCycleGapTimeUs;
   uint                       m_distributeDataPollIntervall;

   const bool                 m_isRunningOnScu;

   AllDaqAdministration*      m_poAllDaq;
   FgFeedbackDevice*          m_poCurrentDevice;
   FbChannel*                 m_poCurrentChannel;

   std::string                m_gnuplotBin;
   std::string                m_gnuplotTerminal;
   std::string                m_gnuplotOutput;
   std::string                m_gnuplotLineStyle;

   static bool readInteger( uint&, const std::string& );
   static bool readFloat( float&, const std::string& );
   static void readTwoIntegerParameters( uint& rParam1, uint& rParam2, const std::string& rArgStr );

public:
   CommandLine( int argc, char** ppArgv );
   virtual ~CommandLine( void );

   AllDaqAdministration* operator()( void );

   AllDaqAdministration* getDaqAdminPtr( void )
   {
      return m_poAllDaq;
   }

   int onArgument( void ) override;

   bool isVerbose( void ) const
   {
      return m_verbose;
   }

   bool isRunningOnScu( void ) const
   {
      return m_isRunningOnScu;
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

   const std::string& getLineStyle( void )
   {
      return m_gnuplotLineStyle;
   }

   bool isOutputFileDefined( void ) const
   {
      return !m_gnuplotOutput.empty();
   }

   bool isAutoBuilding( void ) const
   {
      return m_autoBuilding;
   }

   bool isDeviationPlottingEnabled( void ) const
   {
      return m_deviationEnable;
   }

   bool isContinuePlottingEnabled( void ) const
   {
      return m_continuePlotting && !isOutputFileDefined();
   }

   bool isPlotAlwaysSetValueEnabled( void ) const
   {
      return m_plotAlwaysSetValue;
   }

   bool isZoomYAxis( void ) const
   {
      return m_zoomYAxis;
   }

   float getXAxisLen( void ) const
   {
      return m_xAxisLen;
   }

   uint64_t getPoltTime( void ) const
   {
      return static_cast<uint64_t>(m_xAxisLen);
   }

   uint getPlotInterval( void ) const
   {
      return m_plotInterval;
   }

   bool isExitOnError( void ) const
   {
      return m_exitOnError;
   }

   bool doNotPlot( void ) const
   {
      return m_noPlot;
   }

   bool isPairingBySequence( void )
   {
      return m_pairingBySequence;
   }

   uint getPollInterwalTime( void )
   {
      return m_distributeDataPollIntervall;
   }

protected:
   int onErrorUnrecognizedShortOption( char unrecognized ) override;
   int onErrorUnrecognizedLongOption( const std::string& unrecognized ) override;

private:
   void autoBuild( void );
}; // class CommandLine

} // namespace Scu


#endif // _FB_COMMAND_LINE_HPP
// ================================= EOF ======================================
