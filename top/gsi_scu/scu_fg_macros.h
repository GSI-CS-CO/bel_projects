/*!
 * @file scu_fg_macros.h
 * @brief Module for handling MIL and non MIL
 *        function generator macros
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 04.02.2020
 * Outsourced from scu_main.c
 */
#ifndef _SCU_FG_MACROS_H
#define _SCU_FG_MACROS_H

#include "scu_main.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! ---------------------------------------------------------------------------
 * @brief Data type for remembering the last data sent to a function generator.
 */
typedef struct
{
  // uint64_t timeout;
   uint32_t param_sent;   /*!<@brief Sent counter */
   int32_t  last_c_coeff; /*!<@brief Value of last C-coefficient of polynomial */
} FG_CHANNEL_T;

/*! ---------------------------------------------------------------------------
 * @brief Prints a error message happened in the device-bus respectively
 *        MIL bus.
 * @param status return status of the MIL-driver module.
 * @param slot Slot-number in the case the mil connection is established via
 *             SCU-Bus
 * @param msg String containing additional message text.
 */
void printDeviceError( const int status, const int slot, const char* msg );

/*! ---------------------------------------------------------------------------
 * @brief configures each function generator channel.
 *
 *  checks first, if the drq line is inactive, if not the line is cleared
 *  then activate irqs and send the first tuple of data to the function generator
 *  @param channel number of the specified function generator channel from
 *         0 to MAX_FG_CHANNELS-1
 */
int configure_fg_macro( const unsigned int channel );

/*! ---------------------------------------------------------------------------
 * @brief disable function generator channel
 * @param channel number of the function generator channel from 0 to MAX_FG_CHANNELS-1
 */
void disable_channel( const unsigned int channel );

/*! ---------------------------------------------------------------------------
 *  @brief Decide how to react to the interrupt request from the function
 *         generator macro.
 *  @param socket encoded slot number with the high bits for SIO / MIL_EXT
 *                distinction
 *  @param fg_base base address of the function generator macro
 *  @param irq_act_reg state of the irq act register, saves a read access
 *  @param pSetvalue Pointer of target for set-value.
 */
void handleMacros( const unsigned int socket,
                   const unsigned int fg_base,
                   const uint16_t irq_act_reg,
                   signed int* pSetvalue );

#ifdef __cplusplus
}
#endif
#endif /* _SCU_FG_MACROS_H */
/*================================== EOF ====================================*/
