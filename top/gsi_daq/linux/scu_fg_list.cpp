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
#include <daq_exception.hpp>
#include <daq_calculations.hpp>
#include <sdb_ids.h>
#include <assert.h>

using namespace Scu;
using namespace daq;
using namespace gsi;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FgList::FgList( void )
   :m_lm32SoftwareVersion( 0 )
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
   Lm32Swi swi( pEbAccess );
   scan( &swi );
}

/*! ---------------------------------------------------------------------------
 */
#ifdef CONFIG_FW_VERSION_3
  #define busy fg_rescan_busy
  #define _FG_VERSION_ 3
#else
  #define _FG_VERSION_ 4
#endif

void FgList::scan( Lm32Swi* poSwi )
{
   /*
    * Assuming the etherbone-connection has been already established.
    */
   assert( poSwi->getEbRamAcess()->isConnected() );

   //TODO Implement this function once a good rescan function call has
   //     been implemented in LM32.
   uint32_t tmpLm32SwVersion;
   poSwi->getEbRamAcess()->readLM32( &tmpLm32SwVersion, sizeof( tmpLm32SwVersion ),
                           offsetof( FG::SCU_SHARED_DATA_T, oSaftLib.oFg.version ) );

   m_lm32SoftwareVersion = gsi::convertByteEndian( tmpLm32SwVersion );
   if( m_lm32SoftwareVersion != _FG_VERSION_ )
   {
      std::string errorMessage =
      "Expecting LM32 software major version " TO_STRING(_FG_VERSION_)
      " for now! But detected version is: ";
      errorMessage += std::to_string( m_lm32SoftwareVersion );
      errorMessage += "! Sorry!";
      throw Exception( errorMessage );
   }

   uint32_t scanBusy = 1;
   poSwi->getEbRamAcess()->writeLM32( &scanBusy, sizeof( uint32_t ),
                         offsetof( FG::SCU_SHARED_DATA_T, oSaftLib.oFg.busy ) );


   /*
    * Trigger of LM32 software interrupt.
    */
   poSwi->send( FG::FG_OP_RESCAN );

   /*
    * Timeout of 3 seconds.
    */
   const USEC_T timeout = getSysMicrosecs() + MICROSECS_PER_SEC * 3;
   do
   {
      poSwi->getEbRamAcess()->readLM32( &scanBusy, sizeof( scanBusy ),
                           offsetof( FG::SCU_SHARED_DATA_T, oSaftLib.oFg.busy ) );
      if( getSysMicrosecs() > timeout )
      {
         throw Exception( "Timeout while FG scanning!" );
      }
   }
   while( scanBusy != 0 );

   sync( poSwi->getEbRamAcess() );
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
                               offsetof( FG::SCU_SHARED_DATA_T, oSaftLib.oFg.aMacros ) +
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

/*! ---------------------------------------------------------------------------
 */
bool FgList::isPresent( const uint socket, const uint device )
{
   for( const auto& fg: m_list )
   {
      if( (fg.getSocket() == socket) && (fg.getDevice() == device) )
         return true;
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 */
bool FgList::isSocketUsed( const uint socket )
{
   for( const auto& fg: m_list )
      if( fg.getSocket() == socket )
         return true;
   return false;
}

/*! ---------------------------------------------------------------------------
 */
uint FgList::getNumOfFoundMilFg( void )
{
   uint foundFgs = 0;

   for( const auto& fg: m_list )
      if( fg.isMIL() )
         foundFgs++;

   return foundFgs;
}

/*! ---------------------------------------------------------------------------
 */
uint FgList::getNumOfFoundNonMilFg( void )
{
   uint foundFgs = 0;

   for( const auto& fg: m_list )
      if( !fg.isMIL() )
         foundFgs++;

   return foundFgs;
}

//================================== EOF ======================================
