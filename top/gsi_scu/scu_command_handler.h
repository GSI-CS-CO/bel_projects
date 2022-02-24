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

#include <sw_queue.h>
#include "scu_main.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Data type for the waiting queue which holds the commands form
 *        SAFT-LIB.
 * @see g_queueSaftCmd
 */
typedef uint32_t SAFT_CMD_T;
   
/*!
 * @brief Waiting queue containing commands sent by SAFT-LIB.
 */
extern SW_QUEUE_T g_queueSaftCmd;

/*! ---------------------------------------------------------------------------
 * @brief Initializing of the SAFT-LIB command handler.
 */
STATIC inline ALWAYS_INLINE
void initCommandHandler( void )
{
   queueReset( &g_queueSaftCmd );
}

/*! ---------------------------------------------------------------------------
 * @brief Evaluates and executes the commands received by SAFT-LIB.
 */
void commandHandler( void );

#ifdef __cplusplus
}
#endif

#endif /* _SCU_COMMAND_HANDLER_H */
/*================================== EOF ====================================*/
