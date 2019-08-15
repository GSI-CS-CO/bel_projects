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
#include <daq_exception.hpp>
#include <daq_eb_ram_buffer.hpp>
#include <scu_shared_mem.h>

#ifndef DAQ_DEFAULT_WB_DEVICE
   #define DAQ_DEFAULT_WB_DEVICE "dev/wbm0"
#endif

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
class DaqInterface
{
   struct DAQ_RING_T
   {
      ring_pos_t  m_head;
      ring_pos_t  m_tail;
   } PACKED_SIZE;
   static_assert( offsetof( DAQ_RING_T, m_head ) ==
                  offsetof( struct daq_buffer, ring_head ), "Offset-error!" );
   static_assert( offsetof( DAQ_RING_T, m_tail ) ==
                  offsetof( struct daq_buffer, ring_tail ), "Offset-error!" );

   bool               m_ebAccessSelfCreated;
   DAQ_RING_T         m_oRing;

protected:
   daq::EbRamAccess*  m_poEbAccess;

public:
   DaqInterface( DaqEb::EtherboneConnection* poEtherbone );
   DaqInterface( daq::EbRamAccess* poEbAccess );
   virtual ~DaqInterface( void );

protected:
   void readRingPosition( void );


private:
   void init( void );
};

} // namespace MiLdaq
} // namespace Scu
#endif
//================================== EOF ======================================