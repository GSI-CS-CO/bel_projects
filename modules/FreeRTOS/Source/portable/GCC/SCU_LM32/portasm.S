/*!
 * @file portasm.S
 * @brief Assembler part of LM32 port for FreeRTOS.
 *
 * @date 14.01.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 */

#ifndef __lm32__
  #error This assembler module is for the target Latice Micro32 (LM32) only!
#endif
#ifndef CONFIG_RTOS
  #error Macro CONFIG_RTOS has to be defined in Makefile!
#endif

#include <lm32signal.h>
#include <lm32AssemblerMacros.h>

/*!
 * @brief Pointer to current Task Control Block.
 * @see task.c
 */
#ifdef __DOXYGEN__
extern TCB_t* volatile pxCurrentTCB;
#else
.extern pxCurrentTCB
#endif

#ifdef __DOXYGEN__
/*!
 * The atomic section nesting counter is implemented in the startup module
 * crt0ScuLm32.S
 */
extern volatile uint32_t __atomic_section_nesting_count
#else
.extern __atomic_section_nesting_count
#endif

.extern _irq_entry

#ifdef __DOXYGEN__
void vStartFirstTask( void );
#else
.global vStartFirstTask
#endif

#ifdef __DOXYGEN__
void vPortYield( void );
#else
.global vPortYield
#endif

#ifdef __DOXYGEN__
void vPortInterruptHandler( void );
#else
.global vPortInterruptHandler
#endif

/*!
 * @brief Storage offset in 32-bit values.
 * @note In this case a variable for the context switch flag will used,
 *       therefore the storage offset for register saving will be one.
 * @see __cscf
 */
#define ST_OFS 1


/*! ---------------------------------------------------------------------------
 * @brief Defining of memory place for the context switch cause flag.
 * @note At stack offset (sp+4) will stored a flack which indicates the
 *       reason of context saving.\n
 *       * If its value equal one, than a real LM32 exception
 *         - eg: a interrupt - was happened.\n
 *       * If its value equal zero, so the context saving was caused by
 *         the FreeRTOS scheduler.
 */
#define __cscf (sp + (ALIGN * (1 + 0)))

#ifndef __DOXYGEN__
.section .text
.align ALIGN
#endif

#if 1
/*! ---------------------------------------------------------------------------
 * @brief Macro saves the content op all LM32 registers
 *        (except R0, R1 and SP (R28)) on the stack.
 */
#ifdef __DOXYGEN__
#define SAVE_REGISTERS
#else
.macro SAVE_REGISTERS
   sw        __R2,  r2
   sw        __R3,  r3
   sw        __R4,  r4
   sw        __R5,  r5
   sw        __R6,  r6
   sw        __R7,  r7
   sw        __R8,  r8
   sw        __R9,  r9
   sw        __R10, r10
   sw        __R11, r11
   sw        __R12, r12
   sw        __R13, r13
   sw        __R14, r14
   sw        __R15, r15
   sw        __R16, r16
   sw        __R17, r17
   sw        __R18, r18
   sw        __R19, r19
   sw        __R20, r20
   sw        __R21, r21
   sw        __R22, r22
   sw        __R23, r23
   sw        __R24, r24
   sw        __R25, r25
   sw        __GP,  gp
   sw        __FP,  fp
   sw        __RA,  ra
   sw        __EA,  ea
.endm
#endif

/*! ---------------------------------------------------------------------------
 * @brief Macro restores all LM32 registers from stack
 *        except R0, R1 and SP (R28)
 */
#ifdef __DOXYGEN__
#define RESTORE_REGISTERS
#else
.macro RESTORE_REGISTERS
   lw        ea,  __EA
   lw        ra,  __RA
   lw        fp,  __FP
   lw        gp,  __GP
   lw        r25, __R25
   lw        r24, __R24
   lw        r23, __R23
   lw        r22, __R22
   lw        r21, __R21
   lw        r20, __R20
   lw        r19, __R19
   lw        r18, __R18
   lw        r17, __R17
   lw        r16, __R16
   lw        r15, __R15
   lw        r14, __R14
   lw        r13, __R13
   lw        r12, __R12
   lw        r11, __R11
   lw        r10, __R10
   lw        r9,  __R9
   lw        r8,  __R8
   lw        r7,  __R7
   lw        r6,  __R6
   lw        r5,  __R5
   lw        r4,  __R4
   lw        r3,  __R3
   lw        r2,  __R2
.endm
#endif

#endif
/*! ---------------------------------------------------------------------------
 * @brief Macro to save registers on the stack.
 */
#ifdef __DOXYGEN__
#define SAVE_CONTEXT
#else
.macro SAVE_CONTEXT
   /*
    * Making space for the context on the stack.
    */
   addi        sp,  sp, -SP_CORR   /* r28 */

   /*
    * Saving all register values of the current respectively old context.
    * Register R0 is always 0, therefore start with R1.
    */
   sw        __R1,  r1
   SAVE_REGISTERS      /* Save all registers except r0 and r1 */

   /*
    * Save the top of stack value to the Task Control Block (TCB).
    */
   LOAD_VAR  r1, pxCurrentTCB
   sw        (r1+0), sp
.endm
#endif

/*! ---------------------------------------------------------------------------
 * @brief Macro to restore registers from the stack.
 */
#ifdef __DOXYGEN__
#define RESTORE_CONTEXT
#else
.macro RESTORE_CONTEXT
   /*
    * Loading last stack value from Task Control Block (TCB).
    */
   LOAD_VAR  r1, pxCurrentTCB /* Load the pointer of the task control block in register R1. */
   mv        r2, r1           /* Copy R1 to R2 */
   lw        sp, (r1+0)       /* After access, SP (r28) hold stack pointer from actual TCB */
   addi      r1, sp, SP_CORR  /* Calculate new stack pointer */
   sw        (r2+0), r1       /* Store new Stack pointer in actual TCB */

   /*
    * Restoring all register values (except R0, R1 and R28 (SP))
    * for the new context.
    */
   RESTORE_REGISTERS
.endm
#endif

#ifdef CONFIG_RTOS
/*! ---------------------------------------------------------------------------
 * @brief Start first task, resets the global variable
 *        __atomic_section_nesting_count and enable global interrupt
 */
#ifdef __DOXYGEN__
void vStartFirstTask( void ) {}
#else
vStartFirstTask:
   LOAD_ADDR r1, __atomic_section_nesting_count
   sw        (r1+0), r0       /* __atomic_section_nesting_count = 0; */
   mvi       r1, EIE
   wcsr      ie, r1           /* Setting EIE bit enabling the IRQs by "eret"*/
   RESTORE_CONTEXT
   lw        r1, __R1         /* Load Register R1 back */
   addi      sp, sp, SP_CORR  /* Correct Stack pointer */
   eret                       /* Return-Address is stored in EA */
#endif

/*! ---------------------------------------------------------------------------
 * @brief Function for context switching it enables the IRQs as well.
 */
#ifdef __DOXYGEN__
void vPortYield( void ) {}
#else
vPortYield:
   wcsr      ie, r0             /* Disable interrupts */
   SAVE_CONTEXT
   sw        __cscf, r0         /* Set the context switch cause flag to zero */
   calli     vTaskSwitchContext /* Call this C-function implemented in task.c */
   RESTORE_CONTEXT
   lw        r1, __cscf         /* Load context change cause flag in r1 */
   bne       r0, r1, L_INT_SAVE /* Jump if context switch was caused by LM32 exception  */
   mv        ea, ra             /* Return-Address is stored in RA */
L_INT_SAVE:
#if 0
   LOAD_VAR  r1, __atomic_section_nesting_count
   bne       r1, r0, L_NOT_ENABLE
#endif
   mvi       r1, EIE
   wcsr      ie, r1             /* Setting EIE bit enabling the IRQs by "eret"*/
L_NOT_ENABLE:
   lw        r1, __R1           /* Load register R1 back */
   addi      sp, sp, SP_CORR    /* Correct stack pointer */
   eret                         /* Return-address is stored in EA */
#endif

#endif /* ifdef CONFIG_RTOS */

/*! ---------------------------------------------------------------------------
 * @brief Interrupt Handler. Jump target from _interrupt_handler
 *        implemented in crt0FreeRTOS.S
 * @see crt0FreeRTOS.S
 */
#ifdef __DOXYGEN__
void vPortInterruptHandler( void ) {}
#else
vPortInterruptHandler:
   SAVE_CONTEXT
   ori       r1, r0, 1          /* Set the context switch cause flag to one */
   sw        __cscf, r1         /* and store it in stack. */
   mvi       r1, SIGINT         /* Argument for C-function _irq_entry (will ignored)*/
   calli     _irq_entry         /* Call this C- callback function */
   RESTORE_CONTEXT
   lw        r1, __cscf         /* Load context switch cause flag in r1 */
   bne       r0, r1, L_RETURN   /* Jump if context change was caused from LM32 exception  */
   mv        ea, ra             /* Return-Address is stored in RA */
L_RETURN:
   lw        r1, __R1           /* Load register R1 back */
   addi      sp, sp, SP_CORR    /* Correct stack pointer */
   eret                         /* Return-address is stored in EA */
#endif

/*================================== EOF ====================================*/
