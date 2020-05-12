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
#include "scu_mailbox.h"

#ifdef __cplusplus
extern "C" {
#endif
/*!
 * @brief Control register of function generator.
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/FunctionGeneratorQuadratic#cntrl_reg
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/ScuFgDoc
 */
typedef struct PACKED_SIZE
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__DOXYGEN__)
   /*!
    * @brief  Add frequency select: (wo) bit [15:13]
    */
   unsigned int frequency_select: 3;

   /*!
    * @brief step value M (wo) bit [12:10]
    */
   unsigned int step:             3;

   /*!
    * @brief virtual function generator number (wr) bit [9:4]
    */
   unsigned int number:           6;

   /*!
    * @brief Data request bit [3]
    * @note Only in
    * @see FG_DREQ
    * @see https://github.com/GSI-CS-CO/bel_projects/blob/proposed_master/modules/function_generators/fg_quad/wb_fg_quad.vhd
    */
   const bool dataRequest:    1;

   /*!
    * @brief Indicator if function generator running. (ro) bit [2]
    * @see FG_RUNNING
    */
   const bool isRunning:      1;

   /*!
    * @brief Enable function generator (rw) bit
    * @see FG_ENABLED
    */
   bool enable:               1;

   /*!
    * @brief Reset, 1 -> active (rw) bit [0]
    */
   bool reset:                1;
#else
   #error Big endian is requested for this bit- field structure!
#endif
} FG_CTRL_RG_T_BV;

#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof(FG_CTRL_RG_T_BV) == sizeof(uint16_t) );
STATIC_ASSERT( (int)true == 1 );
#endif

#ifdef CONFIG_MIL_FG
/*!
 * @ingroup ALIAS
 * @brief Alias name for member of control register in MIL-function generator
 * @see FG_CTRL_RG_T_BV::reset
 */

#define devDrq      reset

/*!
 * @ingroup ALIAS
 * @brief Alias name for member of control register in MIL-function generator
 * @see FG_CTRL_RG_T_BV::enable
 */
#define devStateIrq enable

#endif /* ifdef CONFIG_MIL_FG */

/*!
 * @brief Access wrapper avoiding suspicious cast operations.
 */
typedef union PACKED_SIZE
{
   /*!
    * @brief Segmented access by bit vector.
    */
   FG_CTRL_RG_T_BV bv;

   /*!
    * @brief Total access by integer.
    */
   uint16_t        i16;
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

#ifndef __DOXYGEN__
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
#endif

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

/*! ---------------------------------------------------------------------------
 * @brief Returns the 16 bit shift register value
 */
STATIC inline
uint16_t getFgShiftRegValue( const FG_PARAM_SET_T* pPset )
{
   return (pPset->control & 0x3FFC0) >> 6;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the lower 16 bit value of the C- coefficient.
 * @see getFgCoeffCHigh16
 */
STATIC inline
uint16_t getFgCoeffCLow16( const FG_PARAM_SET_T* pPset )
{
   STATIC_ASSERT( sizeof( pPset->coeff_c ) == sizeof( uint32_t ) );
   return GET_LOWER_HALF( pPset->coeff_c );
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the higher 16 bit value of the C- coefficient.
 * @see getFgCoeffCLow16
 */
STATIC inline
uint16_t getFgCoeffCHigh16( const FG_PARAM_SET_T* pPset )
{
   STATIC_ASSERT( sizeof( pPset->coeff_c ) == sizeof( uint32_t ) );
   return GET_UPPER_HALF( pPset->coeff_c );
}

/*! ---------------------------------------------------------------------------
 * @brief Sets the registers of a ADAC function generator.
 */
STATIC inline void setAdacFgRegs( FG_REGISTER_T* pFgRegs,
                                  const FG_PARAM_SET_T* pPset,
                                  const uint16_t controlReg )
{
   ADDAC_FG_ACCESS( pFgRegs, cntrl_reg.i16 ) = controlReg;
   ADDAC_FG_ACCESS( pFgRegs, coeff_a_reg )   = pPset->coeff_a;
   ADDAC_FG_ACCESS( pFgRegs, coeff_b_reg )   = pPset->coeff_b;
   ADDAC_FG_ACCESS( pFgRegs, shift_reg )     = getFgShiftRegValue( pPset );
   ADDAC_FG_ACCESS( pFgRegs, start_l )       = getFgCoeffCLow16( pPset );
   ADDAC_FG_ACCESS( pFgRegs, start_h )       = getFgCoeffCHigh16( pPset );
}

/*! ---------------------------------------------------------------------------
 * @todo Replace this function by access via type FG_CTRL_RG_T
 * @see FG_CTRL_RG_T
 */
STATIC inline unsigned int getFgNumberFromRegister( const uint16_t reg )
{
#if 0
   return (reg >> 4) & 0x3F; // virtual fg number Bits 9..4
#else
   const FG_CTRL_RG_T ctrlReg = { .i16 = reg };
   return ctrlReg.bv.number;
#endif
}


/*! --------------------------------------------------------------------------
 * @brief Returns the relative offset address of the register set of a
 *        function generator macro.
 * @param number Number of functions generator macro till now 0 or 1.
 * @return Relative offset address in uint16_t alignment.
 */
STATIC inline
unsigned int getFgOffsetAddress( const unsigned int number )
{
   static const unsigned int fgAddrTab[] = { FG1_BASE, FG2_BASE };
   FG_ASSERT( number < ARRAY_SIZE( fgAddrTab ) );
   return fgAddrTab[ number ];
}

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
 * @brief Returns the pointer of the SCU-function generators control register.
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @param slot Slot number, valid range 1 .. MAX_SCU_SLAVES (12)
 * @param number Number of functions generator macro 0 or 1.
 * @return Pointer to the control-register.
 */
STATIC volatile inline
FG_CTRL_RG_T* getFgCntrlRegPtr( const void* pScuBusBase,
                                const unsigned int slot,
                                const unsigned int number )
{
   return &getFgRegisterPtr( pScuBusBase, slot, number )->cntrl_reg;
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
   volatile int16_t coeff_a_reg;

   /*!
    * @brief linear value 'b' [r/w]
    *
    * A 16 bit value that needs to be written once after each data
    * request interrupt.
    */
   volatile int16_t coeff_b_reg;

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
   volatile int16_t coeff_c_high_reg;

} FG_MIL_REGISTER_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof( FG_MIL_REGISTER_T, cntrl_reg )        == 0 * sizeof(uint16_t) );
STATIC_ASSERT( offsetof( FG_MIL_REGISTER_T, coeff_a_reg )      == 1 * sizeof(uint16_t) );
STATIC_ASSERT( offsetof( FG_MIL_REGISTER_T, coeff_b_reg )      == 2 * sizeof(uint16_t) );
STATIC_ASSERT( offsetof( FG_MIL_REGISTER_T, shift_reg )        == 3 * sizeof(uint16_t) );
STATIC_ASSERT( offsetof( FG_MIL_REGISTER_T, coeff_c_low_reg )  == 4 * sizeof(uint16_t) );
STATIC_ASSERT( offsetof( FG_MIL_REGISTER_T, coeff_c_high_reg ) == 5 * sizeof(uint16_t) );
STATIC_ASSERT( sizeof( FG_MIL_REGISTER_T ) == MIL_BLOCK_SIZE * sizeof(short) );
#endif

/*!
 * @ingroup PATCH
 * @brief Patch macro which accomplishes the register access of a
 *        ADAC function generator macro.
 * @see __FG_ACCESS
 * @param p Pointer to the concerning function generator register set.
 * @param m Name of member variable.
 * @code
 * MIL_FG_ACCESS( foo, bar ) = value;
 * @endcode
 * corresponds to
 * @code
 * foo->bar = value;
 * @endcode
 */
#define MIL_FG_ACCESS( p, m ) __FG_ACCESS( FG_MIL_REGISTER_T, uint16_t, p, m )

/*! ---------------------------------------------------------------------------
 * @brief Initializes the register set for MIL function generator.
 */
STATIC inline void setMilFgRegs( FG_MIL_REGISTER_T* pFgRegs,
                                  const FG_PARAM_SET_T* pPset,
                                  const uint16_t controlReg )
{
   pFgRegs->cntrl_reg.i16     = controlReg;
   pFgRegs->coeff_a_reg       = pPset->coeff_a;
   pFgRegs->coeff_b_reg       = pPset->coeff_b;
   pFgRegs->shift_reg         = getFgShiftRegValue( pPset );
   pFgRegs->coeff_c_low_reg   = getFgCoeffCLow16( pPset );
   pFgRegs->coeff_c_high_reg  = getFgCoeffCHigh16( pPset );
}

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
 * @brief disables the generation of irqs for the specified channel
 *  SIO and MIL extension stop generating irqs
 *  @param channel number of the channel from 0 to MAX_FG_CHANNELS-1
 * @see enable_scub_msis
 */
void disable_slave_irq( const unsigned int channel );

/*! ---------------------------------------------------------------------------
 * @brief Send signal REFILL to the SAFTLIB when the fifo level has
 *        the threshold reached. Helper function of function handleMacros().
 * @see handleMacros
 * @param channel Channel of concerning function generator.
 */
void sendRefillSignalIfThreshold( const unsigned int channel );

/*! ---------------------------------------------------------------------------
 * @brief Helper function of function handleMacros().
 * @see handleMacros
 */
STATIC inline void makeStop( const unsigned int channel )
{
   sendSignal( cbisEmpty( &g_shared.fg_regs[0], channel )?
                                                      IRQ_DAT_STOP_EMPTY :
                                                      IRQ_DAT_STOP_NOT_EMPTY,
               channel );
   disable_slave_irq( channel );
   g_shared.fg_regs[channel].state = STATE_STOPPED;
}

/*! ---------------------------------------------------------------------------
 * @brief Helper function of function handleMacros().
 * @see handleMacros
 */
STATIC inline void makeStart( const unsigned int channel )
{
   g_shared.fg_regs[channel].state = STATE_ACTIVE;
   sendSignal( IRQ_DAT_START, channel ); // fg has received the tag or brc message
}

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

#ifdef __cplusplus
}
#endif
#endif /* _SCU_FG_MACROS_H */
/*================================== EOF ====================================*/
