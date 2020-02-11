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
/*!
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/FunctionGeneratorQuadratic#cntrl_reg
 */
typedef struct PACKED_SIZE
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__DOXYGEN__)
   /*!
    * @brief  Add frequency select: bit [15:13]
    */
   uint16_t frequency_select: 3;

   /*!
    * @brief step value M [w/r] bit [12:10]
    */
   uint16_t step:             3;

   /*!
    * @brief virtual function generator number [w/r] bit [9:4]
    */
   uint16_t number:           6;

   /*!
    * @brief stopped flag [r] bit [3]
    */
   const uint16_t stopped:    1;

   /*!
    * @brief running flag [r] bit [2]
    */
   const uint16_t running:    1;

   uint16_t __not_used__:     1;

   /*!
    * @brief Reset, 1 -> active bit [0]
    */
   uint16_t reset:            1;
#else
   #error Big endian is requested for this bit- field structure!
#endif
} FG_CTRL_RG_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof(FG_CTRL_RG_T) == sizeof(uint16_t));
#endif

/*!
 * @brief Image of the function generators hardware registers.
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/FunctionGeneratorQuadratic#cntrl_reg
 */
typedef struct PACKED_SIZE
{
   /*!
    * @brief control register [r/w]
    */
   volatile FG_CTRL_RG_T cntrl_reg;

   /*!
    * @brief quadratic value 'a' [r/w]
    *
    * A 16 bit value that needs to be written once after each data
    * request interrupt.
    */
   volatile uint16_t coeff_a_reg;

   /*!
    * @brief linear value 'b' [r/w]
    *
    * A 16 bit value that needs to be written once after each data
    * request interrupt.
    */
   volatile uint16_t coeff_b_reg;

   /*!
    * @brief writing to this register starts the FG [w]
    *
    * At the same time the signal brdcst_o of the FG macro is raised.
    * This can be used to start a second FG.
    */
   volatile uint16_t broadcast_start;

   /*!
    * @brief scale factor for value 'a' [r/w]
    *
    * Value must be between 0 and 64.
    */
   volatile uint16_t shift_reg;

   /*!
    * @brief start value (high) [r/w]
    *
    * Bit [31:16] of the start value
    */
   volatile uint16_t start_h_reg;

   /*!
    * @brief start value (low)
    *
    * Bit [15:0] of the start value
    */
   volatile uint16_t start_l_reg;

   /*!
    * @brief ramp count register [r]
    *
    * Shows the count of interpolated ramp segments
    */
   volatile const uint16_t ramp_cnt_l_reg;

   /*!
    * @brief ramp count register [r]
    *
    * Shows the count of interpolated ramp segments
    */
   volatile const uint16_t ramp_cnt_h_reg;

   /*!
    * @brief tag low word [r/w]
    */
   volatile uint16_t tag_low_reg;

   /*!
    * @brief tag high word [r/w]
    */
   volatile uint16_t tag_high_reg;

   /*!
    * @brief firmware version of the fg macro [r]
    */
   volatile const uint16_t fw_version;

} FG_REGISTER_T;

#ifndef __DOXYGEN__
/*
 * Okay - this is especially for the doubters... ;-)
 */
STATIC_ASSERT( offsetof( FG_REGISTER_T, cntrl_reg ) == FG_CNTRL * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, coeff_a_reg ) == FG_A * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, coeff_b_reg ) == FG_B * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, broadcast_start ) == FG_BROAD * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, shift_reg ) == FG_SHIFT * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, start_h_reg ) == FG_STARTH * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, start_l_reg ) == FG_STARTL * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, ramp_cnt_l_reg ) == FG_RAMP_CNT_LO * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, ramp_cnt_h_reg ) == FG_RAMP_CNT_HI * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, tag_low_reg ) == FG_TAG_LOW * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, tag_high_reg ) == FG_TAG_HIGH * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, fw_version ) == FG_VER * sizeof( uint16_t ) );
STATIC_ASSERT( sizeof( FG_REGISTER_T ) == 12 * sizeof( uint16_t ));
/*
 * Satisfied?
 */
#endif

/*! ---------------------------------------------------------------------------
 * @brief Returns the pointer of the register structure of a
 *        SCU-bus function generator.
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @param slot Slot number, valid range 1 .. MAX_SCU_SLAVES (12)
 * @param number Number of functions generator macro 0 or 1.
 * @return Pointer of register object.
 * @see FG_REGISTER_T
 */
static inline
FG_REGISTER_T* getFgRegister( const void* pScuBusBase,
                              const unsigned int slot,
                              const unsigned int number )
{
   return (FG_REGISTER_T*)
           &(((uint16_t*)scuBusGetAbsSlaveAddr( pScuBusBase, slot ))
              [(number == 0)? FG1_BASE : FG2_BASE]);
}


#ifdef CONFIG_MIL_FG
/*!
 * @brief Image of the MIL- function generators hardware registers.
 */
typedef struct PACKED_SIZE
{
   /*!
    * @brief control register [r/w]
    */
   volatile FG_CTRL_RG_T cntrl_reg;

   /*!
    * @brief quadratic value 'a' [r/w]
    *
    * A 16 bit value that needs to be written once after each data
    * request interrupt.
    */
   volatile uint16_t coeff_a_reg;

   /*!
    * @brief linear value 'b' [r/w]
    *
    * A 16 bit value that needs to be written once after each data
    * request interrupt.
    */
   volatile uint16_t coeff_b_reg;

   /*!
    * @brief scale factor for value 'a' [r/w]
    *
    * Value must be between 0 and 64.
    */
   volatile uint16_t shift_reg;

   /*!
    * @brief C coefficient low value
    */
   volatile uint16_t coeff_c_low_reg;

   /*!
    * @brief C coefficient high value
    */
   volatile uint16_t coeff_c_high_reg;

} FG_MIL_REGISTER_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( FG_MIL_REGISTER_T ) == 6 * sizeof(uint16_t) );
#endif
#endif /* CONFIG_MIL_FG */

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
