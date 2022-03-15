/*!
 * @file scu_eca_handler.h
 * @brief Handler of Event Conditioned Action for SCU function-generators
 * @date 31.01.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * Outsourced from scu_main.c
 */
#ifndef _SCU_ECA_HANDLER_H
#define _SCU_ECA_HANDLER_H

#include "eca_queue_type.h"
#include "scu_shared_mem.h"
#include "scu_main.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Type of Event Condition Action object (ECA)
 */
typedef struct
{
   const uint32_t    tag;    //!<@brief ECA-tag
   ECA_QUEUE_ITEM_T* pQueue;
#ifdef _CONFIG_ECA_BY_MSI
   ECA_CONTROL_T*    pControl;
#endif
} ECA_OBJ_T;

extern ECA_OBJ_T g_eca;

/*! ---------------------------------------------------------------------------
 * @brief Find the ECA queue of LM32
 */
void initEcaQueue( void );

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Event Condition Action (ECA) handler
 * @see schedule
 */
void ecaHandler( void );

#ifdef __cplusplus
}
#endif
#endif /* _SCU_ECA_HANDLER_H */
/*================================== EOF ====================================*/
