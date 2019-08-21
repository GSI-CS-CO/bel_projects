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

using namespace Scu::MiLdaq::MiLdaqt;
using namespace std;

/*! ----------------------------------------------------------------------------
 */
Plot::Plot( DaqMilCompare* pParent,
            const std::string gpOpt,
            const std::string gpExe,
            const std::size_t pipeSize )
   :gpstr::PlotStream( gpOpt, gpExe, pipeSize )
   ,m_pParent( pParent )
{
   *this << "set terminal X11 title \"" "\"" << endl;
   *this << "set grid" << endl;
}

/*! ----------------------------------------------------------------------------
 */
void Plot::plot( void )
{
}

//================================== EOF ======================================
