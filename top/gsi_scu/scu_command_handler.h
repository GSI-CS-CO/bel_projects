/*!
 * @file scu_command_handler.h
 * @brief  Module for receiving of commands from SAFT-LIB
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 03.02.2020
 * Outsourced from scu_main.c
 */
#ifndef _SCU_COMMAND_HANDLER_H
#define _SCU_COMMAND_HANDLER_H

#include "scu_main.h"

#ifndef _CONFIG_USE_OLD_CB
#include <sw_queue.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _CONFIG_USE_OLD_CB
extern SW_QUEUE_T g_queueSaftCmd;

typedef uint32_t SAFT_CMD_T;

STATIC inline ALWAYS_INLINE
void initCommandHandler( void )
{
   queueReset( &g_queueSaftCmd );
}

#endif
   
void commandHandler( register TASK_T* pThis FG_UNUSED );

#ifdef __cplusplus
}
#endif

#endif /* _SCU_COMMAND_HANDLER_H */
/*================================== EOF ====================================*/
