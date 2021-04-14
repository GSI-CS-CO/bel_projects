/*!
 * @file lm32AssemblerMacros.h
 * @brief     Some macros for LM32 GNU-Assembler for handling LM32 exceptions.
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      27.05.2020
 */
#ifndef _LM32ASSEMBLERMACROS_H
#define _LM32ASSEMBLERMACROS_H

#ifndef __lm32__
  #error This headder file is for the target Latice Micro32 (LM32) only!
#endif
#ifndef __ASSEMBLER__
  #error This headder file is for GNU-Assembler only!
#endif

/*!
 * @brief Interrupt enable storage flag of LM32 interrupt enable register.
 */
#define EIE (1 << 1)

/*!
 * @brief The byte alignment of LM32 is 4 bytes.
 */
#define ALIGN 4


/*!
 * @brief Stack pointer correction value
 * @note ST_OFS will defined in the concerning assembler module.
 */
#define SP_CORR (ALIGN * (ST_OFS + 29))

/*! ---------------------------------------------------------------------------
 * @brief Macro calculates the absolute stack-pointer offset in bytes.
 * @param i Relative offset in 32-bit values.
 */
#define spOfs( i ) (sp + (ALIGN * (ST_OFS + i)))

/*! ---------------------------------------------------------------------------
 * Macros for storing all registers of LM32
 */
#define __R1  spOfs(29) /*!<@brief Storage position of r1  */
#define __R2  spOfs(28) /*!<@brief Storage position of r2  */
#define __R3  spOfs(27) /*!<@brief Storage position of r3  */
#define __R4  spOfs(26) /*!<@brief Storage position of r4  */
#define __R5  spOfs(25) /*!<@brief Storage position of r5  */
#define __R6  spOfs(24) /*!<@brief Storage position of r6  */
#define __R7  spOfs(23) /*!<@brief Storage position of r7  */
#define __R8  spOfs(22) /*!<@brief Storage position of r8  */
#define __R9  spOfs(21) /*!<@brief Storage position of r9  */
#define __R10 spOfs(20) /*!<@brief Storage position of r10 */
#define __R11 spOfs(19) /*!<@brief Storage position of r11 */
#define __R12 spOfs(18) /*!<@brief Storage position of r12 */
#define __R13 spOfs(17) /*!<@brief Storage position of r13 */
#define __R14 spOfs(16) /*!<@brief Storage position of r14 */
#define __R15 spOfs(15) /*!<@brief Storage position of r15 */
#define __R16 spOfs(14) /*!<@brief Storage position of r16 */
#define __R17 spOfs(13) /*!<@brief Storage position of r17 */
#define __R18 spOfs(12) /*!<@brief Storage position of r18 */
#define __R19 spOfs(11) /*!<@brief Storage position of r19 */
#define __R20 spOfs(10) /*!<@brief Storage position of r20 */
#define __R21 spOfs(9)  /*!<@brief Storage position of r21 */
#define __R22 spOfs(8)  /*!<@brief Storage position of r22 */
#define __R23 spOfs(7)  /*!<@brief Storage position of r23 */
#define __R24 spOfs(6)  /*!<@brief Storage position of r24 */
#define __R25 spOfs(5)  /*!<@brief Storage position of r25 */
#define __GP  spOfs(4)  /*!<@brief Storage position of r26, alias gp */
#define __FP  spOfs(3)  /*!<@brief Storage position of r27, alias fp */
#define __RA  spOfs(2)  /*!<@brief Storage position of r29, alias ra */
#define __EA  spOfs(1)  /*!<@brief Storage position of r30, alias ea */

/*! --------------------------------------------------------------------------
 * @brief Loads the pointer of a global 32-bit C/C++ variable in a register.
 * @param reg Register name.
 * @param var Name of the global variable.
 */
#ifdef __DOXYGEN__
#define LOAD_ADDR( reg, var )
#else
.macro LOAD_ADDR reg, var
   mvhi  \reg,  hi(\var)
   ori   \reg,  \reg, lo(\var)
.endm
#endif

/*! ---------------------------------------------------------------------------
 * @brief Loads the value of a global 32-bit C/C++ variable in a register.
 * @param reg Register name.
 * @param var Name of the global variable.
 */
#ifdef __DOXYGEN__
#define LOAD_VAR( reg, var )
#else
.macro LOAD_VAR reg, var
   LOAD_ADDR \reg, \var
   lw        \reg, (\reg+0)
.endm
#endif

#endif /* ifndef _LM32ASSEMBLERMACROS_H */
/*================================== EOF ====================================*/
