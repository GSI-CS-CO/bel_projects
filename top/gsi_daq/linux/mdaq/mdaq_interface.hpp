/*!
 *  @file mdaq_interface.hpp
 *  @brief MIL-DAQ Interface Library for Linux
 *
 *  @date 14.08.2019
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
#ifndef _MDAQ_INTERFACE_HPP
#define _MDAQ_INTERFACE_HPP
#include <stddef.h>
#include <string>
#include <daq_base_interface.hpp>
#include <scu_fg_list.hpp>

namespace Scu
{
namespace MiLdaq
{

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 * @ingroup DAQ_EXCEPTION
 */
class Exception: public daq::Exception
{
public:
   Exception( const std::string& rMsg ):
      daq::Exception( "MIL-DAQ: " + rMsg ) {}
};

///////////////////////////////////////////////////////////////////////////////
class DaqInterface: public DaqBaseInterface
{
public:
   // TODO Replace these naked numbers asap!!!
   constexpr static uint         c_maxDevices        = 40;
   //constexpr static uint         c_maxSlots          = 12;
  // constexpr static uint         c_startSlot         = 0;
   constexpr static uint         c_maxChannels       = 254;

   using RING_INDEX_T = RING_POS_T;
   using RING_ITEM_T = MIL_DAQ_OBJ_T;

   class RingItem: public RING_ITEM_T
   {
   public:
      uint64_t getTimestamp( void ) const
      {
         return (static_cast<uint64_t>(tmstmp_h) << BIT_SIZEOF(tmstmp_l)) |
                 tmstmp_l;
      }

      uint getSetValue( void ) const
      {
         return setvalue >> BIT_SIZEOF(uint16_t);
      }

      uint getSetValue32( void ) const
      {
         return setvalue;
      }

      uint getActValue( void ) const
      {
         return actvalue;
      }

      uint getActValue32( void ) const
      {
         return actvalue << BIT_SIZEOF(uint16_t);
      }

      FG_MACRO_T getChannel( void ) const
      {
         return fgMacro;
      }

      uint getMilDaqAddress( void ) const
      {
         return ::Scu::MiLdaq::getMilDaqDevice( this );
      }

      uint getMilDaqLocation( void ) const
      {
         return ::Scu::MiLdaq::getMilDaqSocket( this );
      }

      uint getMilDaqScuBusSlot( void ) const
      {
         return ::Scu::MiLdaq::getMilDaqScuBusSlot( this );
      }

      uint getMilDaqScuMilExtention( void ) const
      {
         return ::Scu::MiLdaq::getMilDaqScuMilExtention( this );
      }
   };

   static constexpr RING_INDEX_T c_ringBufferCapacity = DAQ_RING_SIZE;

private:
   struct DAQ_RING_T
   {
      RING_INDEX_T  m_head;
      RING_INDEX_T  m_tail;
   } PACKED_SIZE;
   static_assert( offsetof( DAQ_RING_T, m_head ) ==
                  offsetof( MIL_DAQ_BUFFER_T, ring_head ), "Offset-error!" );
   static_assert( offsetof( DAQ_RING_T, m_tail ) ==
                  offsetof( MIL_DAQ_BUFFER_T, ring_tail ), "Offset-error!" );

   DAQ_RING_T         m_oRing;

   void readRingData( RING_ITEM_T* ptr, uint len, uint offset = 0 );

public:
   DaqInterface( DaqEb::EtherboneConnection* poEtherbone );
   DaqInterface( daq::EbRamAccess* poEbAccess );
   virtual ~DaqInterface( void );

   RING_INDEX_T getHeadRingIndex( void ) const
   {
      return m_oRing.m_head;
   }

   RING_INDEX_T getTailRingIndex( void ) const
   {
      return m_oRing.m_tail;
   }

   bool areDataToRead( void ) const
   {
      return getHeadRingIndex() != getTailRingIndex();
   }

   uint getBufferSize( void );

protected:
   bool readRingPosition( void );
   void updateRingTail( void );

   void incrementRingTail( void )
   {
      m_oRing.m_tail++;
      m_oRing.m_tail %= c_ringBufferCapacity;
#if 0
      static int count = 0;
      if( m_oRing.m_tail == 0 )
         std::cerr << "new: " << count++ << std::endl;
#endif
   }

   uint readRingItems( RingItem* pItems, uint size );

private:
   void init( void );
};

} // namespace MiLdaq
} // namespace Scu
#endif
//================================== EOF ======================================
