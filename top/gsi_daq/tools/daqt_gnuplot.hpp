/*!
 *  @file daqt_gnuplot.hpp
 *  @brief DAQ interface to gnuplot
 *
 *  @date 29.04.2019
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
#ifndef _DAQT_GNUPLOT_HPP
#define _DAQT_GNUPLOT_HPP

#include "gnuplot_i.h"
#include "daqt.hpp"

namespace daqt
{

using namespace daq;

class DaqPlot;

///////////////////////////////////////////////////////////////////////////////
class Plot
{
   friend class DaqPlot;
   gnuplot_ctrl*       m_poGnuplot;
   DaqPlot*            m_pParent;

protected:
   const std::size_t   m_maxPoints;
   double*             m_pX;
   double*             m_pY;

public:
   Plot( DaqPlot*, std::size_t );
   ~Plot( void );
   void plotXY( DAQ_DATA_T* pData, const std::string& rStrLegende );
};

///////////////////////////////////////////////////////////////////////////////
class PlotContinuous: public Plot
{
public:
   PlotContinuous( DaqPlot* pParent ):
      Plot( pParent,
            DaqAdministration::c_contineousDataLen -
            DaqAdministration::c_discriptorWordSize ) {}
};

///////////////////////////////////////////////////////////////////////////////
class PlotHighResPostMortem: public Plot
{
public:
   PlotHighResPostMortem( DaqPlot* pParent ):
      Plot( pParent,
            DaqAdministration::c_hiresPmDataLen -
            DaqAdministration::c_discriptorWordSize ) {}
};

///////////////////////////////////////////////////////////////////////////////
class DaqPlot
{
   PlotContinuous*         m_pContinuous;
   PlotHighResPostMortem*  m_pHighResPostMortem;
   Channel*                m_pParent;

public:
   DaqPlot( Channel* );
   ~DaqPlot( void );
   void plotXY( DAQ_DATA_T* pData );
};

} // namespace daqt
#endif // ifndef _DAQT_GNUPLOT_HPP
//================================== EOF ======================================
