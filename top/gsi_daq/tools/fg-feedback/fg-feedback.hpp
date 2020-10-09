/*!
 *  @file mdaqt.hpp
 *  @brief Main module of MIL-Data Acquisition Tool
 *
 *  @date 14.08.2019
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
#ifndef _MDAQT_HPP
#define _MDAQT_HPP

#ifndef __DOCFSM__
 #include <string>
 #include <stdlib.h>
 #include <iostream>
 #include <vector>
 #include <scu_fg_feedback.hpp>
 #include <daq_eb_ram_buffer.hpp>
 #include <daq_calculations.hpp>
 #include <fb_command_line.hpp>
#endif

#ifndef HOT_KEY_RECEIVE
  #define HOT_KEY_RECEIVE             'i'
#endif
#ifndef HOT_KEY_RESET
  #define HOT_KEY_RESET               'r'
#endif
#ifndef HOT_KEY_TOGGLE_SINGLE_SHOOT
  #define HOT_KEY_TOGGLE_SINGLE_SHOOT 's'
#endif
#ifndef HOT_KEY_TOGGLE_GAP_READING
  #define HOT_KEY_TOGGLE_GAP_READING  'g'
#endif
#ifndef HOT_KEY_PRINT_HISTORY
  #define HOT_KEY_PRINT_HISTORY       'h'
#endif


namespace Scu
{
//namespace MiLdaq
//{
//namespace MiLdaqt
//{

#ifndef GNUPLOT_DEFAULT_TERMINAL
  #define GNUPLOT_DEFAULT_TERMINAL "X11 size 1200,600"
#endif

#ifndef MAX_ITEMS_PER_SECOND
   #define MAX_ITEMS_PER_SECOND 200
#endif

#ifdef FSM_DECLARE_STATE
   #undef  FSM_DECLARE_STATE
#endif
#define FSM_DECLARE_STATE( state, attr... ) state

class Plot;
class Device;
class CommandLine;
class AllDaqAdministration;

//////////////////////////////////////////////////////////////////////////////
class DaqAllFeedbackChannel: public FgFeedbackChannel
{
   friend class Plot;
   constexpr static uint64_t c_minimumPlotInterval = daq::NANOSECS_PER_SEC / 10;
   struct PLOT_T
   {
      double  m_time;
      float   m_set;
      float   m_act;
      bool    m_setValid;
   };

   enum STATE_T
   {
      FSM_DECLARE_STATE( WAIT, label='waiting for single shoot\ndisabled',
                               color=red ),
      FSM_DECLARE_STATE( START, label='initialize new plot', color=blue ),
      FSM_DECLARE_STATE( COLLECT, label='collecting data', color=blue ),
      FSM_DECLARE_STATE( PLOT, label='plot final values', color=green )
   };

   STATE_T            m_state;

   typedef std::vector<PLOT_T> PLOT_LIST_T;

   Plot*                 m_pPlot;
   MiLdaq::MIL_DAQ_T     m_lastSetRawValue;
   MiLdaq::MIL_DAQ_T     m_lastActRawValue;
   uint64_t              m_startTime;
   uint64_t              m_lastTime;
   uint64_t              m_currentTime;
   uint64_t              m_minTime;
   uint64_t              m_maxTime;
   uint64_t              m_timeToPlot;
   uint64_t              m_plotIntervalTime;
   PLOT_LIST_T           m_aPlotList;
   PLOT_LIST_T::iterator m_iterator;
   bool                  m_singleShoot;

public:
   DaqAllFeedbackChannel( uint iterfaceAddress );
   virtual ~DaqAllFeedbackChannel( void );

   Device* getParent( void );

   uint64_t getPlotStartTime( void ) const
   {
      return m_startTime;
   }

   uint64_t getCurrentTime( void ) const
   {
      return m_currentTime;
   }

   uint64_t getTimeLimitNanoSec( void );

   uint64_t getPlotIntervalTime( void );

   void reset( void );

   std::size_t getItemLimit( void );

   std::string getOutputTerminal( void )
   {
      return GNUPLOT_DEFAULT_TERMINAL;
   }

   CommandLine* getCommandLine( void );

   bool plotDuringCollecting( void );

   bool isSingleShoot( void ) const
   {
      return m_singleShoot;
   }

   void setSingleShoot( bool enable )
   {
      m_singleShoot = enable;
   }

private:
   void onData( uint64_t wrTimeStamp, MiLdaq::MIL_DAQ_T actValue,
                                      MiLdaq::MIL_DAQ_T setValue ) override;

   void addItem( uint64_t time, MiLdaq::MIL_DAQ_T actValue,
                 MiLdaq::MIL_DAQ_T setValue,
                 bool setValueValid );

   void onInit( void ) override;
   void onReset( void ) override;

};


//////////////////////////////////////////////////////////////////////////////
class Device: public FgFeedbackDevice
{
public:
   Device( uint n ): FgFeedbackDevice( n ) {}

   DaqAllFeedbackChannel* getDaqCompare( const uint address )
   {
      return static_cast<DaqAllFeedbackChannel*>(FgFeedbackDevice::getChannel( address ));
   }

   AllDaqAdministration* getParent( void );
  // void setSingleShoot( enable );
};


//////////////////////////////////////////////////////////////////////////////
class AllDaqAdministration: public FgFeedbackAdministration
{
   friend class CommandLine;
  // Lm32Swi        m_oSwi;
   CommandLine*   m_poCommandLine;

public:
   AllDaqAdministration( CommandLine* m_poCommandLine, std::string ebAddress );
   virtual ~AllDaqAdministration( void );

   CommandLine* getCommandLine( void )
   {
      return m_poCommandLine;
   }

   Device* getDevice( const uint socket )
   {
      return static_cast<Device*>(FgFeedbackAdministration::getDevice( socket ));
   }

//TODO  void onUnregistered( RingItem* pUnknownItem )  override;

   bool showUngegistered( void );

   void setSingleShoot( bool enable );
#if 0
   void sendSwi( FG::FG_OP_CODE_T opCode, uint param = 0 )
   {
      m_oSwi.send( opCode, param );
   }
#endif
#if 0
   RAM_RING_INDEX_T getCurrentRamSize( bool update = true ) override
   {
      std::cerr << "Dummyfunction not used: " << __func__ << "()" << std::endl;
      return 0;
   }

   void clearBuffer( bool update = true ) override
   {
      std::cerr << "Dummyfunction not used: " << __func__ << "()" << std::endl;
   }
#endif
   void scan( void )
   {
     //!! DaqAdministrationFgList::scan( &m_oSwi );
   }
};

inline
CommandLine* DaqAllFeedbackChannel::getCommandLine( void )
{
   return getParent()->getParent()->getCommandLine();
}

#if 0
inline
bool DaqAllFeedbackChannel::plotDuringCollecting( void )
{
   return getCommandLine()->isContinuePlottingEnabled();
}
#endif

inline
Device* DaqAllFeedbackChannel::getParent( void )
{
   return static_cast<Device*>(FgFeedbackChannel::getParent());
}

inline
AllDaqAdministration* Device::getParent( void )
{
   return static_cast<AllDaqAdministration*>(FgFeedbackDevice::getParent());
}




//} // namespace MiLdaqt
//} // namespace MilDaq
} // namespace Scu
#endif // ifndef _MDAQT_HPP
//================================== EOF ======================================
