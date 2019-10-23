/*!
 *  @file scu_fg_list.hpp
 *  @brief Administration of found function generators by LM32 firmware
 *
 *  @date 22.10.2019
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>
 ******************************************************************************
 */
#ifndef _SCU_FG_LIST_HPP
#define _SCU_FG_LIST_HPP

#include <scu_shared_mem.h>
#include <daq_eb_ram_buffer.hpp>
#include <vector>

namespace Scu
{
using namespace std;

///////////////////////////////////////////////////////////////////////////////
class FgList
{
   class FgListItem: protected FG_MACRO_T
   {
   public:
      uint getSocket( void )     const { return socket;     }
      uint getSlot( void )       const { return socket & SCU_BUS_SLOT_MASK; }
      uint getDevice( void )     const { return device;     }
      uint getVersion( void )    const { return version;    }
      uint getOutputBits( void ) const { return outputBits; }
   };

   typedef vector<FgListItem> FG_LIST_T;

   FG_LIST_T           m_list;

public:
   constexpr static uint c_maxFgMacros = MAX_FG_MACROS;

   FgList( void );

   virtual ~FgList( void );

   const FG_LIST_T::iterator begin( void )
   {
      return m_list.begin();
   }

   const FG_LIST_T::iterator end( void )
   {
      return m_list.end();
   }

   const bool empty( void )
   {
      return m_list.empty();
   }

   uint size( void ) const
   {
      return m_list.size();
   }

   void scan( daq::EbRamAccess* );

   void sync( daq::EbRamAccess* );
};

} // nemespace Scu

#endif // ifndef _SCU_FG_LIST_HPP
//================================== EOF ======================================