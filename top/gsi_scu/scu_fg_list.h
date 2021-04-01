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

#include <scu_function_generator.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! ---------------------------------------------------------------------------
 * @brief Prints all found function generators.
 */
void printFgs( void );

/*! ---------------------------------------------------------------------------
 * @brief Print the values and states of all channel registers.
 */
void print_regs( void );

/*! ---------------------------------------------------------------------------
 */
#ifdef CONFIG_SCU_DAQ_INTEGRATION
void addAddacToFgList( const void* pScuBusBase,
                       const unsigned int slot,
                       FG_MACRO_T* pFGlist );
#endif

/*! --------------------------------------------------------------------------
 * @brief Finding of all kinds of function generators connected to
 *        this SCU.
 */
void scan_all_fgs( volatile uint16_t *base_adr,
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

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* ifndef _SCU_FG_LIST_H */

/*================================== EOF ====================================*/
