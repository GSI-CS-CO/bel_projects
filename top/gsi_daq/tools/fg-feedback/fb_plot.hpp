/*!
 *  @file fb_plot.hpp
 *  @brief Specialization of class PlotStream for plotting set and actual
 *         values of ADDAC/ACU- and MIL- DAQs
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
#ifndef _FB_PLOT_HPP
#define _FB_PLOT_HPP

#include <gnuplotstream.hpp>
#include "fg-feedback.hpp"

namespace Scu
{

///////////////////////////////////////////////////////////////////////////////
class Plot: public gpstr::PlotStream
{
   FbChannel*  m_pParent;

public:
   Plot( FbChannel* pParent );

   void plot( void );
   void operator()( void )
   {
      plot();
   }

   void init( void );
};


} // namespace Scu


#endif // ifndef _FB_PLOT_HPP
//================================== EOF ======================================
