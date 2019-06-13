/*!
 *  @file daqt_attributes.hpp
 *  @brief Administration of attributes which has been set by
 *         the command line parser
 *
 *  @date 24.05.2019
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
#ifndef _DAQT_ATTRIBUTES_HPP
#define _DAQT_ATTRIBUTES_HPP

#include <daq_command_interface.h>

namespace Scu
{
namespace daq
{
namespace daqt
{
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

   struct NumValue: public Value<unsigned int>
   {
      NumValue( void ) { m_value = 0; }
   };

   struct BoolValue: public Value<bool>
   {
      BoolValue( void ) { m_value = false; }
   };

   struct SampleValue: public Value<DAQ_SAMPLE_RATE_T>
   {
      SampleValue( void ) { m_value = DAQ_SAMPLE_1MS; }
   };

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
   BoolValue     m_zoomGnuPlot;
};

}  // namespace daqt
}  // namespace daq
}  // namespace Scu
#endif // ifndef _DAQT_ATTRIBUTES_HPP
//================================== EOF =====================================
