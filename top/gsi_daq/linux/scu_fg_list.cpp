/*!
 *  @file scu_fg_list.cpp
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
#include <scu_fg_list.hpp>
#include <assert.h>
using namespace Scu;


///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgList::FgList( void )
{
}

/*! ---------------------------------------------------------------------------
 */
FgList::~FgList( void )
{
}

/*! ---------------------------------------------------------------------------
 */
void FgList::scan( daq::EbRamAccess* pEbAccess )
{
   //TODO Implement this function once a good rescan function call has
   //     been implemented in LM32.
   sync( pEbAccess );
}

/*! ---------------------------------------------------------------------------
 * @brief Synchronizing the the function generator list of this object by the
 *        list in the LM32 shared memory.
 */
void FgList::sync( daq::EbRamAccess* pEbAccess )
{
   assert( dynamic_cast<daq::EbRamAccess*>(pEbAccess) != nullptr );
   /*
    * Assuming the etherbone-connection has been already established.
    */
   assert( pEbAccess->isConnected() );

   /*
    * Unfortunately we don't know how long the list is
    * therefore the maximum size will assumed.
    * Deleting the old list if present.
    */
   m_list.clear();

   /*
    * This will accomplished in parts because reading the whole list will
    * produce a error in the etherbone-library when the device-list
    * is too long.
    */
   FgListItem tmpBuffer[ c_maxFgMacros / 32 ];
   for( uint i = 0; i < (c_maxFgMacros / ARRAY_SIZE(tmpBuffer)); i++ )
   {
      pEbAccess->readLM32( tmpBuffer, sizeof( tmpBuffer ),
                               offsetof( FG::SCU_SHARED_DATA_T, fg_macros ) +
                                 i * sizeof( tmpBuffer ) );

      for( uint j = 0; j < ARRAY_SIZE(tmpBuffer); j++ )
      {
         if( tmpBuffer[j].getOutputBits() == 0 )
         {
            m_list.shrink_to_fit();
            return;
         }
         m_list.push_back( tmpBuffer[j] );
      }
   }
}

//================================== EOF ======================================
