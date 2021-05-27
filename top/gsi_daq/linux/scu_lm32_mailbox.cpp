/*!
 *  @file scu_lm32_mailbox.cpp
 *  @brief Module performs a software-interrupt (SWI) LM32 target
 *
 *  @date 24.09.2020
 *  @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#include <daq_exception.hpp>
#include <sdb_ids.h>
#include <scu_lm32_mailbox.hpp>

using namespace Scu;
using namespace gsi;
using namespace std;
using namespace daq;

/*! ---------------------------------------------------------------------------
 */
Lm32Swi::Lm32Swi( daq::EbRamAccess* pEbAccess )
   :m_pEbAccess( pEbAccess )
{
   uint32_t tmpLm32MailboxSlot;
  m_pEbAccess->readLM32( &tmpLm32MailboxSlot, sizeof( tmpLm32MailboxSlot ),
                         offsetof( FG::SCU_SHARED_DATA_T, oSaftLib.oFg.mailBoxSlot ) );

   /*!
    * @todo Using of mailbox-slots respectively software interrupts
    *       for the communication between host and LM32 are to complex
    *       not synchronous and perhaps not necessary.\n
    *       A better solution in the future will be the server-client
    *       command algorithm already implemented for ADAC-DAQs.
    */
   m_lm32MailboxSlot = convertByteEndian( tmpLm32MailboxSlot );

   if( m_lm32MailboxSlot >= ARRAY_SIZE(MSI_BOX_T::slots) )
   {
      std::string errorMessage =
         "Mailbox slot of LM32 is out of range: ";
      errorMessage += std::to_string( m_lm32MailboxSlot );
      errorMessage += "!";
      throw Exception( errorMessage );
   }

   m__pMailBox = reinterpret_cast<MSI_BOX_T*>
                 (
                    m_pEbAccess->getEbPtr()->findDeviceBaseAddress( DaqEb::gsiId,
                    static_cast<FeSupport::Scu::Etherbone::DeviceId>(MSI_MSG_BOX) )
                 );
}

/*! ---------------------------------------------------------------------------
 */
Lm32Swi::~Lm32Swi( void )
{
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Swi::send( FG::FG_OP_CODE_T opCode, uint param )
{
   if( param > 0xFFFF )
   {
      std::string errorMessage = "Parameter of signal: ";
      errorMessage += FG::fgCommand2String( opCode );
      errorMessage += " value: ";
      errorMessage += std::to_string( param );
      errorMessage += " is out of range!";
      throw Exception( errorMessage );
   }

   using SIGNAL_T = TYPEOF(MSI_SLOT_T::signal);
   SIGNAL_T signal = (opCode << (BIT_SIZEOF( SIGNAL_T ) / 2)) | param;

   m_pEbAccess->getEbPtr()->write( reinterpret_cast<etherbone::address_t>
                                   (&m__pMailBox->slots[m_lm32MailboxSlot].signal),
                                   reinterpret_cast<eb_user_data_t>(&signal),
                                   EB_BIG_ENDIAN | EB_DATA32 );

}

//================================== EOF ======================================
