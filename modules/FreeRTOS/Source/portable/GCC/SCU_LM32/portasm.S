/*!
 * @file portasm.S
 * @brief Assembler part of LM32 port for FreeRTOS.
 *
 * @date 14.01.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 * Origin:  Richard Barry.(V4.7.0)
 *
 */

/*!
 * @brief Pointer to current Task Control Block.
 * @see task.c
 */
#ifdef __DOXYGEN__
volatile TCB_t* volatile pxCurrentTCB;
#else
.extern pxCurrentTCB
#endif

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
 * Giving the opportunity to exchange the interrupt callback function by
 * pre-defining the macro __ISRHandler in the makefile.\n
 * E.g: DEFINES += __ISRHandler=myBetterISRHandler respectively
 *      -D__ISRHandler=myBetterISRHandler.
 * By default the interrupt callback function ISRHandler implemented in
 * file lm32Interrupts.c will used.
 */
#ifndef __ISRHandler
   #define __ISRHandler ISRHandler
#endif

/*! ---------------------------------------------------------------------------
 * Macros for storing all registers of LM32
 */
#define __R1  (sp+120) /*!<@brief Storage position of r1 */
#define __R2  (sp+116) /*!<@brief Storage position of r2 */
#define __R3  (sp+112) /*!<@brief Storage position of r3 */
#define __R4  (sp+108) /*!<@brief Storage position of r4 */
#define __R5  (sp+104) /*!<@brief Storage position of r5 */
#define __R6  (sp+100) /*!<@brief Storage position of r6 */
#define __R7  (sp+ 96) /*!<@brief Storage position of r7 */
#define __R8  (sp+ 92) /*!<@brief Storage position of r8 */
#define __R9  (sp+ 88) /*!<@brief Storage position of r9 */
#define __R10 (sp+ 84) /*!<@brief Storage position of r10 */
#define __R11 (sp+ 80) /*!<@brief Storage position of r11 */
#define __R12 (sp+ 76) /*!<@brief Storage position of r12 */
#define __R13 (sp+ 72) /*!<@brief Storage position of r13 */
#define __R14 (sp+ 68) /*!<@brief Storage position of r14 */
#define __R15 (sp+ 64) /*!<@brief Storage position of r15 */
#define __R16 (sp+ 60) /*!<@brief Storage position of r16 */
#define __R17 (sp+ 56) /*!<@brief Storage position of r17 */
#define __R18 (sp+ 52) /*!<@brief Storage position of r18 */
#define __R19 (sp+ 48) /*!<@brief Storage position of r19 */
#define __R20 (sp+ 44) /*!<@brief Storage position of r20 */
#define __R21 (sp+ 40) /*!<@brief Storage position of r21 */
#define __R22 (sp+ 36) /*!<@brief Storage position of r22 */
#define __R23 (sp+ 32) /*!<@brief Storage position of r23 */
#define __R24 (sp+ 28) /*!<@brief Storage position of r24 */
#define __R25 (sp+ 24) /*!<@brief Storage position of r25 */
#define __GP  (sp+ 20) /*!<@brief Storage position of r26, alias gp */
#define __FP  (sp+ 16) /*!<@brief Storage position of r27, alias fp */
#define __RA  (sp+ 12) /*!<@brief Storage position of r29, alias ra */
#define __EA  (sp+  8) /*!<@brief Storage position of r30, alias ea */

#ifndef __DOXYGEN__
.section .text
.align 4
#endif

/*! ---------------------------------------------------------------------------
 * @brief Macro to save registers on the stack.
 */
#ifdef __DOXYGEN__
#define portSAVE_CONTEXT
#else
.macro portSAVE_CONTEXT
   /* Make room for the context on the stack. */
   addi        sp,  sp, -120   /* r28 */

   /* Register R0 is always 0, so start with R1 */
   sw        __R1,  r1
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
   /* Save the top of stack value to the TCB. */
   and       r1, r0, r0
   mvhi      r1, hi(pxCurrentTCB)
   ori       r1, r1, lo(pxCurrentTCB)
   lw        r1, (r1+0)
   sw        (r1+0), sp
.endm
#endif

/*! ---------------------------------------------------------------------------
 * @brief Macro to restore registers from the stack.
 */
#ifdef __DOXYGEN__
#define portRESTORE_CONTEXT
#else
.macro portRESTORE_CONTEXT
   /* Load last Stack value from TCB. */
   and       sp, r0, r0
   mvhi      sp, hi(pxCurrentTCB)
   ori       sp, sp, lo(pxCurrentTCB) /* After access, SP (r28) holds address of pointer to actual TCB */
   lw        sp,(sp+0)                /* After access, SP (r28) holds pointer to actual TCB */
   mv        r2, sp                   /* Copy SP to R2 */
   lw        sp, (sp+0)               /* After access, SP (r28) hold stack pointer from actual TCB */
   addi      r1, sp, +120             /* Calculate new stack pointer */
   sw        (r2+0), r1               /* Store new Stack pointer in actual TCB */

   /* Load all registers from Stack */
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

#ifdef CONFIG_RTOS
/*! ---------------------------------------------------------------------------
 * @brief Start first task and enable global interrupt
 */
#ifdef __DOXYGEN__
void vStartFirstTask( void ) {}
#else
vStartFirstTask:
   mvi       r1, 2
   wcsr      ie, r1             /* Set EIE Bit */
   and       r1, r0, r0
   mvhi      r1, hi(pxCurrentTCB)
   ori       r1, r1, lo(pxCurrentTCB)
   lw        r1, (r1+0)
   portRESTORE_CONTEXT
   lw        r1, (sp+120)       /* Load Register R1 back */
   addi      sp, sp, 120        /* Correct Stack pointer */
   eret                         /* Return-Address is stored in EA */
#endif

/*! ---------------------------------------------------------------------------
 * @brief Defining of memory place for the context switch cause flag.
 * @note At stack offset (sp+4) will stored a flack which indicates the
 *       reason of context saving.\n
 *       * If its value equal one, than a real LM32 exception
 *         - eg: a interrupt - was happened.\n
 *       * If its value equal zero, so the context saving was caused by
 *         the FreeRTOS scheduler.
 */
#define __cscf (sp+4)

/*! ---------------------------------------------------------------------------
 * @brief PortYield function for context switching
 */
#ifdef __DOXYGEN__
void vPortYield( void ) {}
#else
vPortYield:
   wcsr      ie, r0             /* Disable interrupts */
   portSAVE_CONTEXT
   sw        __cscf, r0         /* Set the context switch cause flag to zero */
   calli     vTaskSwitchContext /* Call this C-function implemented in task.c */
   and       r1, r0, r0
   mvhi      r1, hi(pxCurrentTCB)
   ori       r1, r1, lo(pxCurrentTCB)
   lw        r1, (r1+0)
   portRESTORE_CONTEXT
   lw        r1, __cscf         /* Load context change cause flag in r1 */
   bne       r0, r1, L_INT_SAVE /* Jump if context switch was caused from LM32 exception  */
   mv        ea, ra             /* Return-Address is stored in RA */
L_INT_SAVE:
   mvi       r1, 2
   wcsr      ie, r1             /* Set EIE Bit */
   lw        r1, (sp+120)       /* Load register R1 back */
   addi      sp, sp, 120        /* Correct stack pointer */
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
   portSAVE_CONTEXT
   ori       r1, r0, 1          /* Set the context switch cause flag to one */
   sw        __cscf, r1         /* and store it in stack. */
   calli     __ISRHandler       /* Call this C- callback function */
   and       r1, r0, r0
   mvhi      r1, hi(pxCurrentTCB)
   ori       r1, r1,lo(pxCurrentTCB)
   lw        r1, (r1+0)
   portRESTORE_CONTEXT
   lw        r1, __cscf         /* Load context switch cause flag in r1 */
   bne       r0, r1, L_RETURN   /* Jump if context change was caused from LM32 exception  */
   mv        ea, ra             /* Return-Address is stored in RA */
L_RETURN:
   lw        r1, (sp+120)       /* Load register R1 back */
   addi      sp, sp, 120        /* Correct stack pointer */
   eret                         /* Return-address is stored in EA */
#endif

/*================================== EOF ====================================*/
