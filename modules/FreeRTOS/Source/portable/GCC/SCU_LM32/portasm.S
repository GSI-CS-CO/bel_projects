/*!
 *  @file portasm.S
 *  @brief Assembler part of LM32 port for FreeRtos.
 *
 *  @date 14.01.2020
 *  @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *  Origin:  Richard Barry.(V4.7.0)
 */

/*!
 * @brief Pointer to current Task Control Block.
 * @see task.c
 */
.extern pxCurrentTCB

.global vStartFirstTask
.global vPortYield
.global vPortInterruptHandler

#ifndef __DOXYGEN__

.section .text
.align 4

/*! ---------------------------------------------------------------------------
 * @brief Macro to save registers on the stack.
 */
.macro portSAVE_CONTEXT
   /* Make room for the context on the stack. */
   addi      sp, sp, -120

   /* Register R0 is always 0, so start with R1 */
   sw        (sp+120), r1
   sw        (sp+116), r2
   sw        (sp+112), r3
   sw        (sp+108), r4
   sw        (sp+104), r5
   sw        (sp+100), r6
   sw        (sp+ 96), r7
   sw        (sp+ 92), r8
   sw        (sp+ 88), r9
   sw        (sp+ 84), r10
   sw        (sp+ 80), r11
   sw        (sp+ 76), r12
   sw        (sp+ 72), r13
   sw        (sp+ 68), r14
   sw        (sp+ 64), r15
   sw        (sp+ 60), r16
   sw        (sp+ 56), r17
   sw        (sp+ 52), r18
   sw        (sp+ 48), r19
   sw        (sp+ 44), r20
   sw        (sp+ 40), r21
   sw        (sp+ 36), r22
   sw        (sp+ 32), r23
   sw        (sp+ 28), r24
   sw        (sp+ 24), r25
   sw        (sp+ 20), gp
   sw        (sp+ 16), fp
   sw        (sp+ 12), ra
   sw        (sp+  8), ea

   /* Save the top of stack value to the TCB. */
   and       r1, r0, r0
   mvhi      r1, hi(pxCurrentTCB)
   ori       r1, r1, lo(pxCurrentTCB)
   lw        r1, (r1+0)
   sw        (r1+0), sp
.endm


/*! ---------------------------------------------------------------------------
 * @brief Macro to restore registers from the stack.
 */
.macro portRESTORE_CONTEXT
   /* Load last Stack value from TCB. */
   and       sp, r0, r0
   mvhi      sp, hi(pxCurrentTCB)
   ori       sp, sp, lo(pxCurrentTCB) /* After access, SP holds address of pointer to actual TCB */
   lw        sp,(sp+0)                /* After access, SP holds pointer to actual TCB */
   mv        r2, sp                   /* Copy SP to R2 */
   lw        sp, (sp+0)               /* After access, SP hold stack pointer from actual TCB */

   addi      r1, sp, +120             /* Calculate new stack pointer */
   sw        (r2+0), r1               /* Store new Stack pointer in actual TCB */

   /* Load all registers from Stack */
   lw        ea,  (sp+  8)
   lw        ra,  (sp+ 12)
   lw        fp,  (sp+ 16)
   lw        gp,  (sp+ 20)
   lw        r25, (sp+ 24)
   lw        r24, (sp+ 28)
   lw        r23, (sp+ 32)
   lw        r22, (sp+ 36)
   lw        r21, (sp+ 40)
   lw        r20, (sp+ 44)
   lw        r19, (sp+ 48)
   lw        r18, (sp+ 52)
   lw        r17, (sp+ 56)
   lw        r16, (sp+ 60)
   lw        r15, (sp+ 64)
   lw        r14, (sp+ 68)
   lw        r13, (sp+ 72)
   lw        r12, (sp+ 76)
   lw        r11, (sp+ 80)
   lw        r10, (sp+ 84)
   lw        r9,  (sp+ 88)
   lw        r8,  (sp+ 92)
   lw        r7,  (sp+ 96)
   lw        r6,  (sp+100)
   lw        r5,  (sp+104)
   lw        r4,  (sp+108)
   lw        r3,  (sp+112)
   lw        r2,  (sp+116)
   lw        r1,  (sp+4)   /* Check Status */
.endm


/*! ---------------------------------------------------------------------------
 * @brief Start first task and enable global interrupt
 */
vStartFirstTask:
   mvi       r1, 2
   wcsr      ie, r1      /* Set EIE Bit */

   and       r1, r0, r0
   mvhi      r1, hi(pxCurrentTCB)
   ori       r1, r1, lo(pxCurrentTCB)
   lw        r1, (r1+0)
   portRESTORE_CONTEXT
   lw        r1, (sp+120) /* Load Register R1 back */
   addi      sp, sp, 120  /* Correct Stack pointer */
   eret                   /* Return-Address is stored in EA */


/*! ---------------------------------------------------------------------------
 * @brief PortYield function for context switching
 */
vPortYield:
   wcsr      ie, r0             /* Disable interrupts */

   portSAVE_CONTEXT
   sw        (sp+  4), r0       /* Store flag "No Interrupt Save" */

   calli     vTaskSwitchContext /* Call this C-function implemented in task.c */

   and       r1, r0, r0
   mvhi      r1, hi(pxCurrentTCB)
   ori       r1, r1, lo(pxCurrentTCB)
   lw        r1, (r1+0)
   portRESTORE_CONTEXT

   bne       r0, r1, int_save   /* Jump if flag "Interrupt Save"  */
   mv        ea, ra             /* Return-Address is stored in RA */

int_save:
   mvi       r1, 2
   wcsr      ie, r1              /* Set EIE Bit */

   lw        r1, (sp+120)       /* Load register R1 back */
   addi      sp, sp, 120        /* Correct stack pointer */

   eret                         /* Return-address is stored in EA */


/*! ---------------------------------------------------------------------------
 * @brief Interrupt Handler. Jump target from _interrupt_handler
 *        implemented in crt0FreeRTOS.S
 */
vPortInterruptHandler:
   portSAVE_CONTEXT
   ori       r1, r0, 1
   sw        (sp+  4), r1       /* Set flag "Interrupt Save" */

   calli     ISRHandler         /* Call this C-function implemented in Interrupts.c */

   and       r1, r0, r0
   mvhi      r1, hi(pxCurrentTCB)
   ori       r1, r1,lo(pxCurrentTCB)
   lw        r1, (r1+0)
   portRESTORE_CONTEXT

   bne       r0, r1, exept_return /* Jump if flag "Interrupt Save" */
   mv        ea, ra               /* Return-Address is stored in RA */
exept_return:
   lw        r1, (sp+120)         /* Load register R1 back */
   addi      sp, sp, 120          /* Correct stack pointer */

   eret                           /* Return-address is stored in EA */

#endif /* ifndef __DOXYGEN__ */
/*================================== EOF ====================================*/

