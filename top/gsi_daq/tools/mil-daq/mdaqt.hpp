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
 #include <mdaq_administration.hpp>
 #include <daq_eb_ram_buffer.hpp>
#endif

namespace Scu
{
namespace MiLdaq
{
namespace MiLdaqt
{
#define FSM_DECLARE_STATE( state, attr... ) state
#define FSM_INIT_FSM( startState, attr... ) m_state = startState
#define FSM_TRANSITION( nextState, attr... ) m_state = nextState

class Plot;

//////////////////////////////////////////////////////////////////////////////
class DaqMilCompare: public DaqCompare
{
   struct PLOT_T
   {
      double  m_time;
      float   m_set;
      float   m_act;
   };

   enum STATE_T
   {
      FSM_DECLARE_STATE( wait, label='waiting for trigger condition' ),
      FSM_DECLARE_STATE( start, label='initialize new window' ),
      FSM_DECLARE_STATE( collect, label='collecting data' ),
      FSM_DECLARE_STATE( plot, label='plot values' )
   };

   STATE_T            m_state;

   typedef std::vector<PLOT_T> PLOT_LIST_T;

   MIL_DAQ_T          m_lastSetRawValue;
   MIL_DAQ_T          m_lastActRawValue;
   uint64_t           m_startTime;
   uint64_t           m_lastTime;
   PLOT_LIST_T        m_aPlotList;
   Plot*              m_pPlot;

public:
   DaqMilCompare( uint iterfaceAddress );
   virtual ~DaqMilCompare( void );

   void reset( void );
private:
   void onData( uint64_t wrTimeStamp, MIL_DAQ_T actValue,
                                      MIL_DAQ_T setValue ) override;

   void onInit( void ) override;
};

//////////////////////////////////////////////////////////////////////////////
class MilDaqAdministration: public DaqAdministration
{
   bool m_showUnregistered;

public:
   MilDaqAdministration( std::string ebAddress );
   virtual ~MilDaqAdministration( void );
   void onUnregistered( RingItem* pUnknownItem ) override;
};


} // namespace MiLdaqt
} // namespace MilDaq
} // namespace Scu
#endif // ifndef _MDAQT_HPP
//================================== EOF ======================================
