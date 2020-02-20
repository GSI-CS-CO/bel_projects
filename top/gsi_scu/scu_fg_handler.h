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
 * @brief task definition of scu_bus_handler
 * called by the scheduler in the main loop
 * decides which action for a scu bus interrupt is suitable
 * @param pThis pointer to the current task object (not used)
 * @see schedule
 */
void scu_bus_handler( register TASK_T* pThis FG_UNUSED );

#ifdef __cplusplus
}
#endif
#endif /* _SCU_FG_MAIN_H */
/*================================== EOF ====================================*/
