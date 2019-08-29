/*!
 *  @file mdaqt_plot.hpp
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
#ifndef _MDAQ_PLOT_HPP
#define _MDAQ_PLOT_HPP

#include <gnuplotstream.hpp>
#include "mdaqt.hpp"

namespace Scu
{
namespace MiLdaq
{
namespace MiLdaqt
{

///////////////////////////////////////////////////////////////////////////////
class Plot: public gpstr::PlotStream
{
   DaqMilCompare*  m_pParent;

public:
   Plot( DaqMilCompare* pParent );

   void plot( void );
   void operator()( void )
   {
      plot();
   }

   void init( void );
};


} // namespace MiLdaqt
} // namespace MilDaq
} // namespace Scu


#endif
//================================== EOF ======================================
