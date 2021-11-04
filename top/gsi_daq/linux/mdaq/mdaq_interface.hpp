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
#include <daq_ramBuffer.h>
#include <daq_calculations.hpp>

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
   constexpr static uint c_dataTimeout = 10 * daq::MICROSECS_PER_SEC;

public:
   // TODO Replace these naked numbers asap!!!
   constexpr static uint         c_maxDevices        = 40;
   constexpr static uint         c_maxChannels       = 254;

#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
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
         return getMilDaqDeviceOld( this );
      }

      uint getMilDaqLocation( void ) const
      {
         return getMilDaqSocketOld( this );
      }

      uint getMilDaqScuBusSlot( void ) const
      {
         return getMilDaqScuBusSlotOld( this );
      }

      uint getMilDaqScuMilExtention( void ) const
      {
         return getMilDaqScuMilExtentionOld( this );
      }
   };

   static constexpr RING_INDEX_T c_ringBufferCapacity = DAQ_RING_SIZE;
#endif /* CONFIG_MILDAQ_BACKWARD_COMPATIBLE */

   class BufferItem: public MIL_DAQ_RAM_ITEM_T
   {
      static_assert( sizeof( uint32_t ) >= sizeof( MIL_DAQ_VAL_T ), "" );
      constexpr static uint SHIFT = BIT_SIZEOF( uint32_t ) - BIT_SIZEOF( MIL_DAQ_VAL_T );

   public:
      uint64_t getTimestamp( void ) const
      {
         return timestamp;
      }

      MIL_DAQ_VAL_T getSetValue( void ) const
      {
         return setValue;
      }

      uint32_t getSetValue32( void ) const
      {
         return setValue << SHIFT;
      }

      MIL_DAQ_VAL_T getActValue( void ) const
      {
         return actValue;
      }

      uint32_t getActValue32( void ) const
      {
         return actValue << SHIFT;
      }

      FG_MACRO_T getChannel( void ) const
      {
         return fgMacro;
      }

      uint getMilDaqLocation( void ) const
      {
         return getMilDaqSocket( this );
      }

      uint getMilDaqScuBusSlot( void ) const
      {
         return MiLdaq::getMilDaqScuBusSlot( this );
      }

      uint getMilDaqScuMilExtention( void ) const
      {
         return MiLdaq::getMilDaqScuMilExtention( this );
      }
   };

   static_assert( sizeof( BufferItem ) == sizeof( MIL_DAQ_RAM_ITEM_T ), ":-(" );

   union MIDDLE_BUFFER_T
   {
      BufferItem             oData;
      daq::RAM_DAQ_PAYLOAD_T aPayload[RAM_ITEM_PER_MIL_DAQ_ITEM];
   };

   static_assert( sizeof( MIDDLE_BUFFER_T ) == sizeof( MIL_DAQ_RAM_ITEM_T ) +
                  !!(sizeof( MIL_DAQ_RAM_ITEM_T ) % sizeof( daq::RAM_DAQ_PAYLOAD_T )), ":-O" );

private:
#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
   struct DAQ_RING_T
   {
      RING_INDEX_T  m_head;
      RING_INDEX_T  m_tail;
   } PACKED_SIZE;
   static_assert( offsetof( DAQ_RING_T, m_head ) ==
                  offsetof( MIL_DAQ_BUFFER_T, ring_head ), "Offset-error!" );
   static_assert( offsetof( DAQ_RING_T, m_tail ) ==
                  offsetof( MIL_DAQ_BUFFER_T, ring_tail ), "Offset-error!" );

   /*!
    * @brief Object contains the write and read index when a old LM32-firmware
    *        runs where the DAQ-buffer is in the LM32 shared memory
    */
   DAQ_RING_T         m_oRing;

   void readRingData( RING_ITEM_T* ptr, uint len, uint offset = 0 );
#endif /* CONFIG_MILDAQ_BACKWARD_COMPATIBLE */

protected:
   /*!
    * @brief Object contains the write and read index of the RAM buffer
    */
    MIL_DAQ_ADMIN_T m_oBufferAdmin;

public:
   /*!
    * @brief Constructor variant for a object of type EtherboneConnection
    */
   DaqInterface( DaqEb::EtherboneConnection* poEtherbone );

   /*!
    * @brief Constructor variant for a object of type DaqAccess
    */
   DaqInterface( DaqAccess* poEbAccess );

   /*!
    * @brief Destructor
    */
   virtual ~DaqInterface( void );

#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
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
#endif
   uint getBufferSize( void );


   void clearBuffer( bool update = true )
#ifndef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
   override
#endif
   ;

protected:
#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
   bool readRingPosition( void );
   void updateRingTail( void );

   void incrementRingTail( void )
   {
      m_oRing.m_tail++;
      m_oRing.m_tail %= c_ringBufferCapacity;
   }

   uint readRingItems( RingItem* pItems, uint size );

   /*!
    * @brief Returns "true" if the LM32-firmware is a old one storing
    * the MIL-DAQ data in LM32 shared memory.
    */
   bool isMilDataInLm32Mem( void ) const
   {
      return getEbAccess()->isMilDataInLm32Mem();
   }
#endif /* ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE */

private:
   void init( void );
};

} // namespace MiLdaq
} // namespace Scu
#endif
//================================== EOF ======================================
