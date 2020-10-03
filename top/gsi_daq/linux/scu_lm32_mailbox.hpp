/*!
 *  @file scu_lm32_mailbox.hpp
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
#ifndef _SCU_LM32_MAILBOX_HPP
#define _SCU_LM32_MAILBOX_HPP
#ifdef __lm32__
   #error This module is not for the LM32 target!
#endif
#include <scu_shared_mem.h>
#include <daq_eb_ram_buffer.hpp>

namespace Scu
{

/*!
 * @brief Class sends a software interrupt to LM32
 */
class Lm32Swi
{
   daq::EbRamAccess* m_pEbAccess;
   uint m_lm32MailboxSlot;

   /*!
    * CAUTION: m__pMailBox is a foreign pointer and only valid within LM32 scope!
    *          It will use for address offset calculation only.
    * @see scu_mailbox.h
    */
   gsi::MSI_BOX_T* m__pMailBox;

public:
   Lm32Swi( daq::EbRamAccess* pEbAccess );
   ~Lm32Swi( void );

   daq::EbRamAccess* getEbRamAcess( void ) const
   {
      return m_pEbAccess;
   }

   uint getMailboxSlot( void ) const
   {
      return m_lm32MailboxSlot;
   }

   void send( FG::FG_OP_CODE_T opCode, uint param = 0 );
};

} // namespace Scu

#endif // ifndef _SCU_LM32_MAILBOX_HPP
//---------------------------------- EOF --------------------------------------
