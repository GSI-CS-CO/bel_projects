/*!
 *  @file gnuplotstream.hpp
 *  @brief Minimal C++ interface for Gnuplot with stream ability.
 *
 *  @date 07.05.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *  @see gnuplotstream.cpp
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
#ifndef _GNUPLOTSTREAM_HPP
#define _GNUPLOTSTREAM_HPP

#include <iostream>
#include <iomanip>
#include <sstream>

/*!
 * @brief Namespace for GunPlotStream
 */
namespace gpstr
{
#ifndef GPSTR_DEFAULT_OPTIONS
  #define GPSTR_DEFAULT_OPTIONS ""
#endif
#ifndef GPSTR_DEFAULT_GNUPLOT_EXE
  #define GPSTR_DEFAULT_GNUPLOT_EXE "/usr/bin/gnuplot"
#endif
#ifndef GPSTR_DEFAULT_PIPE_BUFFER_SIZE
  #define GPSTR_DEFAULT_PIPE_BUFFER_SIZE BUFSIZ
#endif

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Exception class of the class PlotStream for the case that something
 *        has been gone wrong.
 */
class Exception
{
   const std::string m_message;

public:
   Exception( const std::string msg ):
      m_message( msg ) {}

   const std::string& what( void ) const
   {
      return m_message;
   }
};

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Gnuplot interface class with stream ability.
 *
 * Ecample:
 * @code
 * #include <gnuplotstream.hpp>
 *
 * using namespace std;
 * using namespace gps;
 *
 * int main( void )
 * {
 *    try
 *    {
 *       PlotStream plot;
 *       plot << "plot sin(x)/x" << endl;
 *       cout << "Press the enter key to end." << endl;
 *       ::fgetc( stdin );
 *    }
 *    catch( Exception& e )
 *    {
 *       cerr << e.what() << endl;
 *       return EXIT_FAILURE;
 *    }
 *    return EXIT_SUCCESS;
 * }
 * @endcode
 */
class PlotStream: public std::ostream
{
   class StringBuffer: public std::stringbuf
   {
      friend class PlotStream;
      PlotStream*   m_pParent;

   public:
      StringBuffer( PlotStream* pParent )
         :m_pParent( pParent ) {}

      int sync( void ) override;
   };

   StringBuffer  m_oBuffer;
   FILE*         m_pPipe;
   char*         m_pPipeBuffer;
   std::size_t   m_pipeBufferSize;

public:
   /*!
    * @brief Constructor of class PlotStream.
    *        Invokes the Gnuplot executable and establishes a inter process
    *        communication via a pipe.
    * @note In the case of an error a exception becomes thrown.
    * @param gpOpt Call options for Gnuplot executable.
    * @param gpExe Path and name of the Gnuplot executable.
    * @param pipeSize Pinesize in bytes for the communication to Gnuplot.
    */
   PlotStream( const std::string gpOpt    = GPSTR_DEFAULT_OPTIONS,
               const std::string gpExe    = GPSTR_DEFAULT_GNUPLOT_EXE,
               const std::size_t pipeSize = GPSTR_DEFAULT_PIPE_BUFFER_SIZE
             );

   /*!
    * @brief Destructor. Closes the pipe to Gnuplot and terminates it.
    */
   ~PlotStream( void );

private:
   void resizePipeBuffer( const std::size_t );
};

} // namespace gps
#endif // ifndef _GNUPLOTSTREAM_HPP
//================================== EOF ======================================
