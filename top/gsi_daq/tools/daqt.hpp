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
#include <string>
#include "gnuplot_i.h"

namespace daqt
{
using namespace daq;

constexpr int INVALID_LIMIT = -1;

class CommandLine;
class Device;

///////////////////////////////////////////////////////////////////////////////
struct Attributes
{
   template<typename VT> struct Value
   {
      bool m_valid;
      VT   m_value;

      Value( void ): m_valid( false ) {}

      void set( const Value<VT>& rMyContainer )
      { /*
         * If the attribute of my container valid but my own not,
         * then making the value of my attribute to the value of my
         * containers attribute.
         */
         if( rMyContainer.m_valid && !m_valid )
            *this = rMyContainer;
      }

      void set( const VT value )
      {
         m_value = value;
         m_valid = true;
      }

      bool operator==( const Value<VT>& rMyContainer )
      {
         if( m_valid != rMyContainer.m_valid )
            return false;
         if( m_value != rMyContainer.m_value )
            return false;
         return true;
      }
   };

   typedef Value<bool>              BoolValue;
   typedef Value<unsigned int>      NumValue;
   typedef Value<DAQ_SAMPLE_RATE_T> SampleValue;

   void set( const Attributes& rMyContainer );

   BoolValue     m_highResolution;
   BoolValue     m_postMortem;
   SampleValue   m_continueMode;
   BoolValue     m_continueTriggerSouce;
   BoolValue     m_highResTriggerSource;
   BoolValue     m_triggerEnable;
   NumValue      m_triggerDelay;
   NumValue      m_triggerCondition;
   NumValue      m_blockLimit;
   BoolValue     m_restart;
};

///////////////////////////////////////////////////////////////////////////////
class DaqContainer: public DaqAdministration
{
   friend class CommandLine;
   friend class Device;

   CommandLine*   m_poCommandLine;
   Attributes     m_oAttributes;

public:
   DaqContainer( const std::string ebName, CommandLine* poCommandLine )
      :DaqAdministration( ebName )
      ,m_poCommandLine( poCommandLine )
      {}

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
};

///////////////////////////////////////////////////////////////////////////////
class Channel: public DaqChannel
{
   friend class  CommandLine;
   friend class  DaqContainer;

   Attributes    m_oAttributes;
   gnuplot_ctrl* m_poGnuplot;

public:
   Channel( unsigned int number );
   ~Channel( void );

   void sendAttributes( void );
   void start( void );

   bool onDataBlock( DAQ_DATA_T* pData, std::size_t wordLen ) override;
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
};

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
inline Device* DaqContainer::getDeviceBySlot( unsigned int slot )
{
   return static_cast<Device*>(DaqAdministration::getDeviceBySlot( slot ));
}



} // namespace daqt
#endif // ifndef _DAQT_HPP
//================================== EOF ======================================
