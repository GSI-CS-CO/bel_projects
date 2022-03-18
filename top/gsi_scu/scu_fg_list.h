/*!
 * @file scu_fg_list.h
 * @brief Module for scanning the SCU for function generators and initializing
 *        the function generator list in the shared memory.
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/ScuFgDoc
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/ScuFgDoc
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 31.03.2021
 * Outsourced from scu_function_generatoe.h
 */
#ifndef _SCU_FG_LIST_H
#define _SCU_FG_LIST_H
#if !defined(__lm32__) && !defined(__DOXYGEN__)
   #error This module is for the target LM32 only!
#endif

#include "scu_shared_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_FG_PEDANTIC_CHECK
   /* CAUTION:
    * Assert-macros could be expensive in memory consuming and the
    * latency time can increase as well!
    * Especially in embedded systems with small resources.
    * Therefore use them for bug-fixing or developing purposes only!
    */
   #include <scu_assert.h>
   #define FG_ASSERT SCU_ASSERT
   #define FG_UNUSED
#else
   #define FG_ASSERT(__e)
   #define FG_UNUSED UNUSED
#endif


/*!
 * @see scu_shared_mem.h
 */
extern SCU_SHARED_DATA_T g_shared;

/*! ---------------------------------------------------------------------------
 * @brief Prints all found function generators.
 */
void printFgs( void );

/*! ---------------------------------------------------------------------------
 * @brief Print the values and states of all channel registers.
 */
void print_regs( void );

/*! ---------------------------------------------------------------------------
 * @brief Resets the entire function generator list to zero.
 *
 * @note This is necessary after a CPU-reset because this data resides in the
 *       shared memory area which is not a part of the .bss section.
 * @param pFgList Start pointer of function generator list in the
 *                shared memory area
 */
void fgListReset( FG_MACRO_T* pFgList );

int fgListAdd( const uint8_t socked,
               const uint8_t dev,
               const uint16_t cid_sys,
               const uint16_t cid_group,
               const uint8_t fg_ver,
               FG_MACRO_T* pFgList );

#ifdef CONFIG_SCU_DAQ_INTEGRATION
/*! ---------------------------------------------------------------------------
 * @brief Adds a found function generator receiding in ADDAC-device
 *        to the function generator list.
 */
void addAddacToFgList( const void* pScuBusBase,
                       const unsigned int slot,
                       FG_MACRO_T* pFGlist );


ALWAYS_INLINE STATIC inline void addAcuToFgList( const void* pScuBusBase,
                                   const unsigned int slot,
                                   FG_MACRO_T* pFGlist )
{
   addAddacToFgList( pScuBusBase, slot, pFGlist );
}

#ifdef CONFIG_DIOB_WITH_DAQ
/*! ---------------------------------------------------------------------------
 * @brief Adds a found function generator receiding in DIOB-device
 *        to the function generator list.
 */
void addDiobToFgList( const void* pScuBusBase,
                      const unsigned int slot,
                      FG_MACRO_T* pFGlist );
#endif /* CONFIG_DIOB_WITH_DAQ */
#endif /* CONFIG_SCU_DAQ_INTEGRATION */

/*! -------------------------------------------------------------------------- 
 * @brief Finding of all kinds of function generators connected to
 *        this SCU.
 */
void fgListFindAll( volatile uint16_t *base_adr,
                  #ifdef CONFIG_MIL_FG
                    volatile unsigned int* mil_base,
                  #endif
                    FG_MACRO_T* fglist,
                    uint64_t *ext_id );

/*! ---------------------------------------------------------------------------
 * @brief  init the buffers for MAX_FG_CHANNELS
 */
void fgResetAndInit( FG_CHANNEL_REG_T* cr,
                     const unsigned int channel,
                     FG_MACRO_T* macro,
                     const void* scub_base
                   #ifdef CONFIG_MIL_FG
                   , const void* devb_base
                   #endif
                   );

#ifdef CONFIG_MIL_FG

#endif

/*! ---------------------------------------------------------------------------
 * @brief Returns the index number of a FG-macro in the FG-list by the
 *        channel number
 */
STATIC inline ALWAYS_INLINE
int getFgMacroIndexFromFgRegister( const unsigned int channel )
{
   FG_ASSERT( channel < ARRAY_SIZE( g_shared.oSaftLib.oFg.aRegs ) );
   return g_shared.oSaftLib.oFg.aRegs[channel].macro_number;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the Function Generator macro of the given channel.
 */
STATIC inline ALWAYS_INLINE
FG_MACRO_T getFgMacroViaFgRegister( const unsigned int channel )
{
   FG_ASSERT( getFgMacroIndexFromFgRegister( channel ) >= 0 );
   FG_ASSERT( getFgMacroIndexFromFgRegister( channel ) < ARRAY_SIZE( g_shared.oSaftLib.oFg.aMacros ));
   return g_shared.oSaftLib.oFg.aMacros[getFgMacroIndexFromFgRegister( channel )];
}

/*! ---------------------------------------------------------------------------
 * @brief Returns "true" if the function generator of the given channel
 *        present.
 * @see FOR_EACH_FG
 * @see FOR_EACH_FG_CONTINUING
 */
STATIC inline bool isFgPresent( const unsigned int channel )
{
   if( channel >= MAX_FG_CHANNELS )
      return false;
   if( getFgMacroIndexFromFgRegister( channel ) < 0 )
      return false;
   return getFgMacroViaFgRegister( channel ).outputBits != 0;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the socked number of the given channel.
 * @note The lower 4 bits of the socket number contains the slot-number
 *       of the SCU-bus which can masked out by SCU_BUS_SLOT_MASK.
 */
STATIC inline ALWAYS_INLINE
unsigned int getSocket( const unsigned int channel )
{
   FG_ASSERT( isFgPresent( channel ) );
   return getFgMacroViaFgRegister( channel ).socket;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the device number of the given channel.
 */
STATIC inline ALWAYS_INLINE
unsigned int getDevice( const unsigned int channel )
{
   FG_ASSERT( isFgPresent( channel ) );
   return getFgMacroViaFgRegister( channel ).device;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* ifndef _SCU_FG_LIST_H */

/*================================== EOF ====================================*/
