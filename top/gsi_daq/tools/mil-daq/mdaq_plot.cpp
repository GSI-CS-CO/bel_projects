/*!
 *  @file mdaqt_plot.cpp
 *  @brief Specialization of class PlotStream for plotting set and actual
 *         values of MIL-DAQs
 *
 *  @date 19.08.2019
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
#include "mdaq_plot.hpp"
#include <daq_calculations.hpp>

using namespace Scu::MiLdaq::MiLdaqt;
using namespace std;

/*!
 * @brief Establishing a upper and lower margin of Y axis, so that the
 *        maximum voltages can be plot as well.
 *
 * The dimension is voltage.
 */
constexpr float Y_PADDING = 0.5;

/*! ----------------------------------------------------------------------------
 */
Plot::Plot( DaqMilCompare* pParent )
   :gpstr::PlotStream( "-noraise", pParent->getCommandLine()->getGnuplotBinary() )
   ,m_pParent( pParent )
{
   init();
}

/*! ----------------------------------------------------------------------------
 */
void Plot::init( void )
{
   *this << "set terminal "
         << m_pParent->getCommandLine()->getTerminal()
         << " title \"SCU: "
         << m_pParent->getParent()->getParent()->getScuDomainName()
         << "\"" << endl;
   *this << "set grid" << endl;
   *this << "set ylabel \"Voltage\"" << endl;
   if( !m_pParent->getCommandLine()->isZoomYAxis() )
      *this << "set yrange [" << -(DAQ_VPP_MAX/2 + Y_PADDING) << ':'
                              << (DAQ_VPP_MAX/2 + Y_PADDING) << ']' << endl;
   *this << "set xrange [0.0:"
         << m_pParent->getCommandLine()->getXAxisLen() << ']' << endl;
   //*this << "set style line 1 linecolor rgb \"red\"" << endl;
}

/*! ----------------------------------------------------------------------------
 */
void Plot::plot( void )
{
   constexpr float MILISECS_PER_NANOSEC = 1000000.0;

   *this << "set title \"fg-" << m_pParent->getParent()->getLocation()
         << '-' << m_pParent->getAddress()
         << "  Date: "
         << daq::wrToTimeDateString( m_pParent->getCurrentTime() );
         if( m_pParent->isSingleShoot() )
            *this << " Single shoot!";
         *this << endl;


   *this << "set xlabel \"Plot start time: " << m_pParent->getPlotStartTime()
         << " ns; interval min: " << (m_pParent->m_minTime / MILISECS_PER_NANOSEC)
         << " ms, interval max: " << (m_pParent->m_maxTime / MILISECS_PER_NANOSEC)
         << " ms; Samples: " << m_pParent->m_aPlotList.size() << "\"" << endl;

   //if( m_pParent->m_aPlotList.empty() )
   //   return;
   bool isDeviationPlottingEnabled =
                  m_pParent->getCommandLine()->isDeviationPlottingEnabled();

   bool validSetDataPresent = false;
   for( auto& i: m_pParent->m_aPlotList )
       if( i.m_setValid )
          validSetDataPresent = true;

   const string& style = m_pParent->getCommandLine()->getLineStyle();
   *this << "plot ";
   if( validSetDataPresent )
      *this << "'-' title 'set value' with "    << style << " lc rgb 'red', ";

   *this << "'-' title 'actual value' with " << style << " lc rgb 'green'";

   if( isDeviationPlottingEnabled && validSetDataPresent )
      *this << ", '-' title 'deviation' with "    << style << " lc rgb 'blue'";

   *this << endl;

   if( validSetDataPresent )
   {
      for( const auto& i: m_pParent->m_aPlotList )
      {
         if( i.m_setValid )
            *this << i.m_time << ' ' << i.m_set << endl;
      }
      *this << 'e' << endl;
   }

   for( const auto& i: m_pParent->m_aPlotList )
   {
      *this << i.m_time << ' ' << i.m_act << endl;
   }
   *this << 'e' << endl;

   if( !isDeviationPlottingEnabled || !validSetDataPresent )
      return;

   for( const auto& i: m_pParent->m_aPlotList )
   {
      if( i.m_setValid )
         *this << i.m_time << ' ' << (i.m_set - i.m_act) << endl;
   }
   *this << 'e' << endl;
}

//================================== EOF ======================================
