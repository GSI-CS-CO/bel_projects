/*!
 * @file scu_mailbox.c
 * @brief SCU mailbox-system for sending signals to SAFTLIB
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      12.05.2020
 */

#include <scu_mailbox.h>
#include <lm32Interrupts.h>

/*! ----------------------------------------------------------------------------
 * @ingroup MAILBOX
 * @brief Configures a mailbox slot.
 * @param slot Mailbox slot number.
 * @param myOffs Offset
 */
STATIC inline ALWAYS_INLINE
void cfgMsiBox( const unsigned int slot, const unsigned int offset )
{
   STATIC_ASSERT( sizeof( pMyMsi[0] ) == sizeof( uint32_t ) );
   MSI_BOX_SLOT_ACCESS( slot, address ) = (uint32_t)&pMyMsi[offset / sizeof(uint32_t)];
}

/*! ----------------------------------------------------------------------------
 * @ingroup MAILBOX
 * @brief Returns and configures the next free mailbox slot.
 * @param offset Offset
 * @retval >=0 Found mailbox slot.
 * @retval -1 No free mailbox slot found.
 */
int getMsiBoxSlot( const unsigned int offset )
{
   int slot = 0;
   ATOMIC_SECTION()
   {  /*
       * Climbing to the first free slot.
       */
      for( ; slot < MSI_MAX_SLOTS; slot++ )
      {
         if( MSI_BOX_SLOT_ACCESS( slot, signal ) == 0xFFFFFFFF )
         {
            cfgMsiBox( slot, offset );
            break;
         }
      }
      if( slot >= MSI_MAX_SLOTS )
         slot = -1;
   }
   return slot;
}

/*================================== EOF ====================================*/
