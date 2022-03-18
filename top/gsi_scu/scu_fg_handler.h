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
#include "scu_fg_macros.h"

#ifdef __cplusplus
extern "C" {
#endif
   
/*!
 * @brief Image of the function generators hardware registers.
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/FunctionGeneratorQuadratic#cntrl_reg
 */
typedef struct HW_IMAGE
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
   volatile uint16_t broad_start;

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
   volatile uint16_t start_h;

   /*!
    * @brief start value (low)
    *
    * Bit [15:0] of the start value
    */
   volatile uint16_t start_l;

   /*!
    * @brief ramp count register [r]
    *
    * Shows the count of interpolated ramp segments
    */
   volatile const uint16_t ramp_cnt_low;

   /*!
    * @brief ramp count register [r]
    *
    * Shows the count of interpolated ramp segments
    */
   volatile const uint16_t ramp_cnt_high;

   /*!
    * @brief tag low word [r/w]
    */
   volatile uint16_t tag_low;

   /*!
    * @brief tag high word [r/w]
    */
   volatile uint16_t tag_high;

   /*!
    * @brief firmware version of the fg macro [r]
    */
   volatile const uint16_t fw_version;

} FG_REGISTER_T;

/*
 * Okay - this is especially for the doubters... ;-)
 */
STATIC_ASSERT( offsetof( FG_REGISTER_T, cntrl_reg ) == FG_CNTRL * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, coeff_a_reg ) == FG_A * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, coeff_b_reg ) == FG_B * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, broad_start ) == FG_BROAD * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, shift_reg ) == FG_SHIFT * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, start_h ) == FG_STARTH * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, start_l ) == FG_STARTL * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, ramp_cnt_low ) == FG_RAMP_CNT_LO * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, ramp_cnt_high ) == FG_RAMP_CNT_HI * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, tag_low ) == FG_TAG_LOW * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, tag_high ) == FG_TAG_HIGH * sizeof( uint16_t ) );
STATIC_ASSERT( offsetof( FG_REGISTER_T, fw_version ) == FG_VER * sizeof( uint16_t ) );
STATIC_ASSERT( sizeof( FG_REGISTER_T ) == 12 * sizeof( uint16_t ));
/*
 * Satisfied?
 */

   
/*! ---------------------------------------------------------------------------
 * @ingroup PATCH
 * @brief Patch macro which accomplishes the register access of a
 *        ADAC function generator macro.
 * @see __SCU_BUS_ACCESS
 * @see __WB_ACCESS
 * @param p Pointer of type FG_REGISTER_T to the concerning
 *          function generator register set.
 * @param m Name of member variable in FG_REGISTER_T.
 * @code
 * ADDAC_FG_ACCESS( foo, bar ) = value;
 * @endcode
 * corresponds to
 * @code
 * foo->bar = value;
 * @endcode
 */
#define ADDAC_FG_ACCESS( p, m ) __SCU_BUS_ACCESS( FG_REGISTER_T, p, m )

#ifndef CONFIG_SCU_DAQ_INTEGRATION
/*! ---------------------------------------------------------------------------
 * @brief Scans the whole SCU-bus direct to the SCU-bus connected
 *        function generators
 * @param pScuBusBase Base address of SCU bus
 * @param pFGlist Start pointer of function generator list.
 */
void scanScuBusFgsDirect( const void* pScuBusBase, FG_MACRO_T* pFGlist );

#endif /* ifndef CONFIG_SCU_DAQ_INTEGRATION */

#ifdef CONFIG_NON_DAQ_FG_SUPPORT
/*! ---------------------------------------------------------------------------
 * @brief Scans the SCU- bus for function generators which doesn't have DAQs.
 * @param pScuBusBase Base address of SCU bus
 * @param pFgList Start pointer of function generator list.
 */
void scanScuBusFgsWithoutDaq( volatile uint16_t *scub_adr, FG_MACRO_T* pFgList );

#endif /* ifdef CONFIG_NON_DAQ_FG_SUPPORT */

/*! ---------------------------------------------------------------------------
 * @brief Returns the pointer of the register structure of a
 *        SCU-bus function generator by its relative offset address.
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @param slot Slot number, valid range 1 .. MAX_SCU_SLAVES (12)
 * @param fgOffset Relative offset address of function generator macro
 *        till now FG1_BASE and FG2_BASE only.
 */
STATIC inline
FG_REGISTER_T* getFgRegisterPtrByOffsetAddr( const void* pScuBusBase,
                                             const unsigned int slot,
                                             const unsigned int fgOffset )
{
   return (FG_REGISTER_T*)
          &(((uint16_t*)scuBusGetAbsSlaveAddr( pScuBusBase, slot ))[fgOffset]);

}

/*! --------------------------------------------------------------------------
 * @brief Returns the relative offset address of the register set of a
 *        function generator macro.
 * @param number Number of functions generator macro till now 0 or 1.
 * @return Relative offset address in uint16_t alignment.
 */
BUS_BASE_T getFgOffsetAddress( const unsigned int number );

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
STATIC inline
FG_REGISTER_T* getFgRegisterPtr( const void* pScuBusBase,
                                 const unsigned int slot,
                                 const unsigned int number )
{
   return getFgRegisterPtrByOffsetAddr( pScuBusBase, slot,
                                                 getFgOffsetAddress( number ));
}


/*! ---------------------------------------------------------------------------
 * @brief Returns the firmware version of the ADAC function generator macro.
 * @param pScuBusBase Base address of SCU bus
 * @param slot Slot number, valid range 1 .. MAX_SCU_SLAVES (12)
 * @return Firmware version number.
 */
STATIC inline
uint16_t getFgFirmwareVersion( const void* pScuBusBase,
                               const unsigned int slot )
{  /*
    * It will suppose that the FG macros in one SCU-bus slave are equal.
    * Therefore it doesn't matter which FG offset address will used
    * FG1_BASE or FG2_BASE.
    */
   return ADDAC_FG_ACCESS( getFgRegisterPtrByOffsetAddr( pScuBusBase,
                                                        slot,
                                                        FG1_BASE ),
                                                        fw_version
                        );
}

/*! ---------------------------------------------------------------------------
 * @brief Prepares the selected ADDAC/ACU- function generator.
 *
 * 1) Enabling the belonging interrupt. \n
 * 2) Starts both belonging DAQ channels for feedback set- and actual- values. \n
 * 3) Sets the digital to analog converter in the function generator mode. \n
 * 4) Resets the ramp-counter \n
 * 5) Sets the ECA-timing tag. \n
 *
 * @param pScuBus Pointer to the SCU-bus base address.
 * @param slot SCU-bus slot number respectively slave- number.
 * @param dev Device-number respectively one of the function generator
 *            belonging to this slave.
 * @param tag ECA- timing- tag (normally 0xDEADBEEF)
 * @return Base pointer of the registers of the selected function generator.
 */
FG_REGISTER_T* addacFgPrepare( const void* pScuBus,
                               const unsigned int slot,
                               const unsigned int dev,
                               const uint32_t tag
                             );

/*! ---------------------------------------------------------------------------
 * @brief Loads the selected ADDAC/ACU-function generator with the first
 *        polynomial data set and enable it.
 * @param pAddagFgRegs Base address of the register set of the selected
 *                     function generator.
 * @param pPset Pointer to the polynomial data set.
 * @param channel Channel number of the concerned function generator.
 */
void addacFgStart( FG_REGISTER_T* pAddagFgRegs,
                   const FG_PARAM_SET_T* pPset,
                   const unsigned int channel );

/*! --------------------------------------------------------------------------
 * @ingroup INTERRUPT
 */
void addacFgDisableIrq( const void* pScuBus,
                        const unsigned int slot,
                        const unsigned int dev );

/*! ---------------------------------------------------------------------------
 *  @brief Disables a running function generator.
 *
 * 1)
 *
 * @param pScuBus Pointer to the SCU- bus.
 * @param solt Scu-bus slot number respectively slave number.
 * @param dev Function generator number of the concerning slave.
 */
void addacFgDisable( const void* pScuBus,
                     const unsigned int slot,
                     const unsigned int dev );

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
