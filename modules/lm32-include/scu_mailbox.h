/*!
 * @file scu_mailbox.h
 * @brief SCU mailbox-system for sending signals to SAFTLIB
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      12.05.2020
 */
#ifndef _SCU_MAILBOX_H
#define _SCU_MAILBOX_H

#ifdef __lm32__
 #include <scu_lm32_macros.h>
 #include <mini_sdb.h>
#else
 #include <helper_macros.h>
#endif
#include <stdint.h>


/*!
 * @defgroup MAILBOX SCU Mailbox system
 */

/*!
 * @ingroup MAILBOX
 * @brief Maximum number of mailbox slots
 */
#define MSI_MAX_SLOTS 128

#ifdef __cplusplus
extern "C" {
namespace gsi
{
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup MAILBOX
 * @brief Data type of a single mailbox slot.
 */
typedef struct HW_IMAGE
{
   uint32_t signal;
   uint32_t address;
} MSI_SLOT_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof( MSI_SLOT_T, signal ) == 0 );
STATIC_ASSERT( sizeof( MSI_SLOT_T ) == 2 * sizeof( uint32_t ) );
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup MAILBOX
 * @brief Container of mailbox slots.
 */
typedef struct HW_IMAGE
{
   volatile MSI_SLOT_T slots[MSI_MAX_SLOTS];
} MSI_BOX_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( MSI_BOX_T ) == MSI_MAX_SLOTS * sizeof( MSI_SLOT_T ) );
#endif

#if defined(__lm32__) || defined(__DOXYGEN__)

/*! ----------------------------------------------------------------------------
 * @ingroup MAILBOX PATCH
 * @brief Helper-macro for mailbox access via wishbone bus.
 *
 * This macro:
 * @code
 * MSI_BOX_SLOT_ACCESS( 42, signal ) = 4711;
 * @endcode
 * is logical equivalent to:
 * @code
 * ((MSI_BOX_T*)pCpuMsiBox)->slots[42].signal = 4711;
 * @endcode
 *
 * @see __WB_ACCESS
 *
 * @param s Slot number 0 <= s < MSI_MAX_SLOTS
 * @param M Name of slot attribute (signal or address).
 */
#define MSI_BOX_SLOT_ACCESS( s, M ) \
   __WB_ACCESS( MSI_BOX_T, uint32_t, pCpuMsiBox, slots[s].M )

/*! ----------------------------------------------------------------------------
 * @ingroup MAILBOX
 * @brief Returns and configures the next free mailbox slot.
 * @param offset Offset
 * @retval >=0 Found mailbox slot.
 * @retval -1 No free mailbox slot found.
 */
int getMsiBoxSlot( const unsigned int offset );

#endif /*if defined(__lm32__) || defined(__DOXYGEN__)*/

#ifdef __cplusplus
} /* namespace gsi */
} /* extern "C" */
#endif /* ifdef __cplusplus */

#endif /* ifndef _SCU_MAILBOX_H */
/*================================== EOF ====================================*/
