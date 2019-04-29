/*!
 *  @file daqt_gnuplot.cpp
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
#include "daqt_gnuplot.hpp"


using namespace daqt;

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 */
Plot::Plot( DaqPlot* pParent, std::size_t maxPoints )
   :m_pParent( pParent )
   ,m_maxPoints( maxPoints )
   ,m_pX( nullptr )
   ,m_pY( nullptr )
{
   m_poGnuplot = ::gnuplot_init();
   if( m_poGnuplot == nullptr )
      throw Exception( "gnuplot_init" );

   ::gnuplot_cmd( m_poGnuplot, "set grid" );
   ::gnuplot_setstyle( m_poGnuplot, "lines" );
   ::gnuplot_cmd( m_poGnuplot, "set yrange [-10.0:10.0]" );
}

/*!----------------------------------------------------------------------------
 */
void Plot::plotXY( DAQ_DATA_T* pData, const std::string& rStrLegende )
{
   if( m_pX == nullptr )
   {
      m_pX = new double[m_maxPoints];
      for( std::size_t i = 0; i < m_maxPoints; i++ )
         m_pX[i] = static_cast<double>(i);
   }

   if( m_pY == nullptr )
      m_pY = new double[m_maxPoints];

   for( std::size_t i = 0; i < m_maxPoints; i++ )
      m_pY[i] = rawToVoltage( pData[i] );

   ::gnuplot_plot_xy( m_poGnuplot, m_pX, m_pY,
                      m_maxPoints, rStrLegende.c_str() );
}

/*!----------------------------------------------------------------------------
 */
Plot::~Plot( void )
{
   if( m_poGnuplot != nullptr )
      ::gnuplot_close( m_poGnuplot );
   if( m_pX != nullptr )
      delete [] m_pX;
   if( m_pY != nullptr )
      delete [] m_pY;
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 */
DaqPlot::DaqPlot( Channel* pParent )
   :m_pContinuous( nullptr )
   ,m_pHighResPostMortem( nullptr )
   ,m_pParent( pParent )
{

}

/*!----------------------------------------------------------------------------
 */
DaqPlot::~DaqPlot( void )
{
   if( m_pContinuous != nullptr )
      delete m_pContinuous;

   if( m_pHighResPostMortem != nullptr )
      delete m_pContinuous;
}

