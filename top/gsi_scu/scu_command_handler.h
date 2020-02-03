/*!
 * @file scu_command_handler.h Module for receiving of commands from SAFT-LIB
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 03.02.2020
 * Outsourced from scu_main.c
 */
#ifndef _SCU_COMMAND_HANDLER_H
#define _SCU_COMMAND_HANDLER_H

#include "scu_main.h"

#ifdef __cplusplus
extern "C" {
#endif

void sw_irq_handler( register TASK_T* pThis FG_UNUSED );

#ifdef __cplusplus
}
#endif

#endif /* _SCU_COMMAND_HANDLER_H */
/*================================== EOF ====================================*/
