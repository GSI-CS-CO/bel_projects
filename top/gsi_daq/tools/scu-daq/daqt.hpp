/*!
 *  @file daqt.hpp
 *  @brief Main module of Data Acquisition Tool
 *
 *  @date 11.04.2019
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
#ifndef _DAQT_HPP
#define _DAQT_HPP
#include <eb_console_helper.h>
#include <stdlib.h>
#include <daq_administration.hpp>
#include <daqt_attributes.hpp>
#include <string>
#include <iostream>
#include <gnuplotstream.hpp>


namespace Scu
{
namespace daq
{
namespace daqt
{

constexpr int INVALID_LIMIT = -1;

#ifndef HOT_KEY_SHOW_STATE
  #define HOT_KEY_SHOW_STATE      's'
#endif
#ifndef HOT_KEY_POST_MORTEM
  #define HOT_KEY_POST_MORTEM     'p'
#endif
#ifndef HOT_KEY_HIGH_RES
  #define HOT_KEY_HIGH_RES        'h'
#endif
#ifndef HOT_KEY_RESET
  #define HOT_KEY_RESET           'r'
#endif
#ifndef HOT_KEY_CLEAR_BUFFER
  #define HOT_KEY_CLEAR_BUFFER    'c'
#endif
#ifndef HOT_KEY_RECEIVE
  #define HOT_KEY_RECEIVE         'i'
#endif
#ifndef HOT_KEY_SHOW_RAM_LEVEL
  #define HOT_KEY_SHOW_RAM_LEVEL  'l'
#endif

#ifndef GNUPLOT_DEFAULT_TERMINAL
  #define GNUPLOT_DEFAULT_TERMINAL "X11 size 1200,600"
#endif

class CommandLine;
class Device;

///////////////////////////////////////////////////////////////////////////////
class DaqContainer: public DaqAdministration
{
   friend class CommandLine;
   friend class Device;

   CommandLine*   m_poCommandLine;
   Attributes     m_oAttributes;

public:
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
   DaqContainer( const std::string ebName, CommandLine* poCommandLine );
#else
   DaqContainer( DaqEb::EtherboneConnection* poEtherbone,
                 CommandLine* poCommandLine );
#endif
   ~DaqContainer( void );

   CommandLine* getCommandLinePtr( void )
   {
      return m_poCommandLine;
   }

   Device* getDeviceBySlot( unsigned int slot );

   bool checkCommandLineParameter( void );
   void prioritizeAttributes( void );
   bool checkForAttributeConflicts( void );
   bool checkWhetherChannelsBecomesOperating( void );

   void sendAttributes( void );
   void start( void );

   void showRunState( void );
   void doPostMortem( void );
   void doHighRes( void );
   void doReset( void );

   void onBlockReceiveError( void ) override;

};

///////////////////////////////////////////////////////////////////////////////
class Channel: public DaqChannel
{
   friend class  CommandLine;
   friend class  DaqContainer;

   class Mode
   {
      friend class Channel;

      Channel*          m_pParent;
      std::size_t       m_size;
      std::size_t       m_ramLevel;
      double*           m_pY;
      const std::string m_text;
      bool              m_notFirst;
      unsigned int      m_blockCount;
      unsigned int      m_sequence;
      unsigned int      m_sampleTime;
      uint64_t          m_timeStamp;
      double            m_frequency;

   public:
      Mode( Channel* pParent, std::size_t size, std::string text );
      ~Mode( void );
      void write( DAQ_DATA_T* pData, std::size_t wordLen );
      void plot( void );
      void reset( void );
   };

   Attributes        m_oAttributes;
   gpstr::PlotStream m_oPlot;
   Mode*             m_poModeContinuous;
   Mode*             m_poModePmHires;
   std::string       m_oOutputFileName;

public:
   Channel( unsigned int number, const std::string& rGnuplot );
   ~Channel( void );

   void sendAttributes( void );
   void start( void );

   bool isMultiplot( void )
   {
      return (m_poModeContinuous != nullptr) && (m_poModePmHires != nullptr);
   }

   bool isFgIntegrated( void );

   bool onDataBlock( DAQ_DATA_T* pData, std::size_t wordLen ) override;
   void showRunState( void );
   void doPostMortem( void );
   void doHighRes( void );
   void reset( void );

protected:
   static DAQ_DATA_T buildAverage( const DAQ_DATA_T* pData,
                                    const std::size_t wordLen );

   bool calcFrequency( double& rFrequency, const DAQ_DATA_T* pData,
                                                  const std::size_t wordLen );
};

//////////////////////////////////////////////////////////////////////////////
class Device: public DaqDevice
{
   friend class CommandLine;
   friend class DaqContainer;

   Attributes  m_oAttributes;

public:

   Device( unsigned int slot )
      :DaqDevice( slot )
   {}

   Channel* getChannel( const unsigned int number )
   {
      return static_cast<Channel*>(DaqDevice::getChannel( number ));
   }

   bool isFgIntegrated( void )
   {
      return getParent()->isFgIntegrated();
   }
};

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
inline Device* DaqContainer::getDeviceBySlot( unsigned int slot )
{
   return static_cast<Device*>(DaqAdministration::getDeviceBySlot( slot ));
}

/*! ---------------------------------------------------------------------------
 */
inline bool Channel::isFgIntegrated( void )
{
   return static_cast<Device*>(getParent())->isFgIntegrated();
}


} // namespace daqt
}
}
#endif // ifndef _DAQT_HPP
//================================== EOF ======================================
