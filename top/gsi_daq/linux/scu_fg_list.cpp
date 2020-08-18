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
   /*
    * Assuming the etherbone-connection has been already established.
    */
   assert( pEbAccess->isConnected() );

   //TODO Implement this function once a good rescan function call has
   //     been implemented in LM32.
   uint32_t tmpLm32SwVersion;
   pEbAccess->readLM32( &tmpLm32SwVersion, sizeof( tmpLm32SwVersion ),
                           offsetof( FG::SCU_SHARED_DATA_T, fg_version ) );

   m_lm32SoftwareVersion = gsi::convertByteEndian( tmpLm32SwVersion );
   if( m_lm32SoftwareVersion != 3 )
   {
      std::string errorMessage =
      "Expecting LM32 software major version 3 for now! But detected version is: ";
      errorMessage += std::to_string( m_lm32SoftwareVersion );
      errorMessage += "! Sorry!";
      throw Exception( errorMessage );
   }

   uint32_t tmpLm32MailboxSlot;
   pEbAccess->readLM32( &tmpLm32MailboxSlot, sizeof( tmpLm32MailboxSlot ),
                        offsetof( FG::SCU_SHARED_DATA_T, fg_mb_slot ) );

   /*!
    * @todo Using of mailbox-slots respectively software interrupts
    *       for the communication between host and LM32 are to complex
    *       not synchronous and perhaps not necessary.\n
    *       A better solution in the future will be the server-client
    *       command algorithm already implemented for ADAC-DAQs.
    */
   const uint lm32MailboxSlot = gsi::convertByteEndian( tmpLm32MailboxSlot );
   if( lm32MailboxSlot >= ARRAY_SIZE(MSI_BOX_T::slots) )
   {
      std::string errorMessage =
         "Mailbox slot of LM32 is out of range: ";
      errorMessage += std::to_string( lm32MailboxSlot );
      errorMessage += "!";
      throw Exception( errorMessage );
   }

   /*
    * CAUTION: _pMailBox is a foreign pointer and only valid within LM32 scope!
    *          It will use for address offset calculation only.
    */
   MSI_BOX_T* _pMailBox = reinterpret_cast<MSI_BOX_T*>
                         (
                           pEbAccess->getEbPtr()->findDeviceBaseAddress( DaqEb::gsiId,
                           static_cast<FeSupport::Scu::Etherbone::DeviceId>(MSI_MSG_BOX) )
                         );

   using SIGNAL_T = TYPEOF(MSI_SLOT_T::signal);
   SIGNAL_T signal = FG::FG_OP_RESCAN << (BIT_SIZEOF( SIGNAL_T ) / 2);

   /*!
    * @todo Flag fg_rescan_busy is a very dirty hack obtaining a
    *       synchronization!
    *       Remove this technique for a better solution!
    *       See comment above.
    */
   uint32_t scanBusy = 1;
   pEbAccess->writeLM32( &scanBusy, sizeof( uint32_t ),
                         offsetof( FG::SCU_SHARED_DATA_T, fg_rescan_busy ) );

   pEbAccess->getEbPtr()->write( reinterpret_cast<etherbone::address_t>
                                    (&_pMailBox->slots[lm32MailboxSlot].signal),
                                 reinterpret_cast<eb_user_data_t>(&signal),
                                 EB_BIG_ENDIAN | EB_DATA32 );

   /*
    * Timeout of 3 seconds.
    */
   const USEC_T timeout = getSysMicrosecs() + MICROSECS_PER_SEC * 3;
   do
   {
      pEbAccess->readLM32( &scanBusy, sizeof( scanBusy ),
                           offsetof( FG::SCU_SHARED_DATA_T, fg_rescan_busy ) );
      if( getSysMicrosecs() > timeout )
      {
         throw Exception( "Timeout while FG scanning!" );
      }
   }
   while( scanBusy != 0 );

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
