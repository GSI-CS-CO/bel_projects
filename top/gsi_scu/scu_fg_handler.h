/*!
 * @file scu_fg_handler.h
 * @brief Module for handling all SCU-BUS function generators
 *        (non MIL function generators)
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 04.02.2020
 * Outsourced from scu_main.c
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/AdcDacScu
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/AdcDac2Scu
 */
#ifndef _SCU_FG_MAIN_H
#define _SCU_FG_MAIN_H

#include "scu_main.h"

#ifdef __cplusplus
extern "C" {
#endif


/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Handles a ADAC- respectively ACU- function generator.
 * @see handleMilFg
 * @param slot SCU-bus slot number respectively slave number.
 * @param fgAddrOffset Relative address offset of the concerning FG-macro
 *                     till now FG1_BASE or FG2_BASE.
 */
void handleAdacFg( const unsigned int slot,
                   const BUS_BASE_T fgAddrOffset );


#ifdef __cplusplus
}
#endif
#endif /* _SCU_FG_MAIN_H */
/*================================== EOF ====================================*/
