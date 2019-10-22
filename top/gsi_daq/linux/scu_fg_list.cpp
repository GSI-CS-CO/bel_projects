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

using namespace Scu;



FgList::FgList( daq::EbRamAccess* poSharedMem ):
   m_poSharedMem( poSharedMem )
{
}

FgList::~FgList( void )
{
}

void FgList::scan( void )
{
}

void FgList::sync( void )
{
   m_list.clear();

   /*
    * Unfortunately we don't know how long the list is
    * therefore the maximum size will assumed.
    */
   FgListItem tmpBuffer[ c_maxFgMacros ];

   m_poSharedMem->readLM32( &tmpBuffer, sizeof( tmpBuffer ),
                            offsetof( FG::SCU_SHARED_DATA_T, fg_macros ) );

   for( uint i = 0; i < ARRAY_SIZE( tmpBuffer ); i++ )
   {
      if( tmpBuffer[i].getOutputBits() == 0 )
         break;
      m_list.push_back( tmpBuffer[i] );
   }
   //m_list.resize();
}

//================================== EOF ======================================
