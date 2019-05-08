/*!
 *  @file gnuplotstream.cpp
 *  @brief Minimal C++ interface for Gnuplot with stream ability.
 *
 *  @date 07.05.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *  @see gnuplotstream.hpp
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
#include <string.h>
#include <unistd.h>
#include "gnuplotstream.hpp"

using namespace gpstr;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
int PlotStream::StringBuffer::sync( void )
{
   if( m_pParent->m_pipeBufferSize < (str().size() + 1) )
      m_pParent->resizePipeBuffer( str().size() + 1 );

   if( ::fputs( str().c_str(), m_pParent->m_pPipe ) == EOF )
   {
      throw Exception( "Can't write data in gnuplot-pipe: -> " +
                       static_cast<std::string>(::strerror( errno )) );
   }
   if( ::fflush( m_pParent->m_pPipe ) == EOF )
   {
      throw Exception( "Can't flush the gnuplot-pipe: -> " +
                       static_cast<std::string>(::strerror( errno )) );
   }
   str("");

   return std::stringbuf::sync();
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
PlotStream::PlotStream( const std::string gpOpt,
                        const std::string gpExe,
                        const std::size_t pipeSize
                      )
   :std::ostream( &m_oBuffer )
   ,m_oBuffer( this )
   ,m_pPipe( nullptr )
   ,m_pPipeBuffer( nullptr )
   ,m_pipeBufferSize( 0 )
{
   if( ::getenv( "DISPLAY" ) == nullptr )
   {
      throw Exception( "Environment variable DISPLAY not found!" );
   }

   if( ::access( gpExe.c_str(), X_OK ) != 0 )
   {
      throw Exception( "Executable file: \"" + gpExe + "\" not found! -> " +
                      ::strerror( errno ) );
   }

   std::string exeStr = gpExe;
   if( !gpOpt.empty() )
      exeStr += ' ' + gpOpt;

   m_pPipe = ::popen( exeStr.c_str(), "w" );
   if( m_pPipe == nullptr )
   {
      throw Exception( "Unable to open \"" + gpExe + "\" -> " +
                       ::strerror( errno ) );
   }

   resizePipeBuffer( pipeSize );
}

/*! ---------------------------------------------------------------------------
 */
PlotStream::~PlotStream( void )
{
   if( m_pPipeBuffer != nullptr )
      delete [] m_pPipeBuffer;

   if( m_pPipe == nullptr )
       return;

   if( ::pclose( m_pPipe ) < 0 )
   {
      throw Exception( "Can't close pipe to gnuplot -> " +
                        static_cast<std::string>(::strerror( errno )) );
   }
}

/*! ---------------------------------------------------------------------------
 */
void PlotStream::resizePipeBuffer( const std::size_t newSize )
{
   if( m_pipeBufferSize == newSize )
      return;

   if( m_pPipeBuffer != nullptr )
      delete [] m_pPipeBuffer;

   m_pPipeBuffer = new (std::nothrow) char[newSize];
   if( m_pPipeBuffer == nullptr )
   {
      throw Exception( "Unable to allocate " + std::to_string( newSize ) +
                       " bytes for gnuplot: -> " + ::strerror( errno ) );
   }
   m_pipeBufferSize = newSize;

   ::setbuf( m_pPipe, m_pPipeBuffer );
}

//================================== EOF ======================================
