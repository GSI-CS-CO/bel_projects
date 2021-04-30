/*!
 * @file crt0ScuLm32.S
 * @brief Startup code for Lattice Micro 32 LM32 applications in the SCU 
 *        with and without FreeRTOS
 *
 * @copyright 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @date 20.05.2020
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @note When the compiler switch CONFIG_RTOS is defined then this module
 *       is specialized for FreeRTOS.
 */
#include <lm32signal.h>

#ifndef MICO32_FULL_CONTEXT_SAVE_RESTORE
 #define MICO32_FULL_CONTEXT_SAVE_RESTORE
#endif

#ifdef CONFIG_RTOS
   .extern vPortInterruptHandler
#else
   /*
    * This function is implemented in module lm32Interrupts.c 
    */
   .extern _irq_entry
#endif
.extern _onException
.extern _onSysCall
.extern __init
.extern main
.extern __exit
#ifndef CONFIG_NO_INTERRUPTS
   /*
    * Nesting counter for atomic sections.
    * This global variable will used in module lm32Interrupts.c and
    * in the case of using FreeRTOS in module portasm.S.
    */
   .global   __atomic_section_nesting_count
   .section  .data
   .align    4
   .type     __atomic_section_nesting_count, @object
   .size     __atomic_section_nesting_count, 4
__atomic_section_nesting_count:
   .long     1
#endif

#ifndef CONFIG_NO_RESET_COUNTER
   .global  __reset_count
   .section .data
   .align    4
   .type     __reset_count, @object
   .size     __reset_count, 4
__reset_count:
   .long -1    /* Pre-initializing __reset_count with -1 by assembler */
#endif

/* ----------------------------------------------------------------------------
 * Exception handlers - Must be 32 bytes long respectively 8 instructions.
 * @see LatticeMico32ProcessorReferenceManual39.pdf page 22
 */
.section    .boot, "ax", @progbits

.global _reset_handler
.type   _reset_handler, @function
_reset_handler:
/*
 * init r0 from CSR to make sure it's got a valid state before XOR (simulation)
 */
    rcsr    r0, CFG 
    xor     r0, r0, r0
    wcsr    IE, r0             /* Disable the global interrupts */
    wcsr    IM, r0             /* Crear interrupt mask register */ 
    mvhi    r1,     hi(_reset_handler)
    ori     r1, r1, lo(_reset_handler)
    wcsr    EBA, r1
    calli   _crt0
.size	_reset_handler, .-_reset_handler

.global _breakpoint_handler
.type   _breakpoint_handler, @function
_breakpoint_handler:
    sw      (sp+0), ra
    calli   _save_all
    mvi     r1,  SIGTRAP
    calli   _onException
    bi      _restore_all_and_return
    nop
    nop
    nop
.size _breakpoint_handler, .-_breakpoint_handler

.global _instruction_bus_error_handler
.type   _instruction_bus_error_handler, @function
_instruction_bus_error_handler:
    sw      (sp+0), ra
    calli   _save_all
    mvi     r1,  SIGSEGV
    calli   _onException
    bi      _restore_all_and_return
    nop
    nop
    nop
.size _instruction_bus_error_handler, .-_instruction_bus_error_handler

.global _watchpoint_handler
.type   _watchpoint_handler, @function
_watchpoint_handler:
    sw      (sp+0), ra
    calli   _save_all
    mvi     r1,  SIGTRAP
    calli   _onException
    bi      _restore_all_and_return
    nop
    nop
    nop
.size _watchpoint_handler, .-_watchpoint_handler

.global _data_bus_error_handler
.type   _data_bus_error_handler, @function
_data_bus_error_handler:
    sw      (sp+0), ra
    calli   _save_all
    mvi     r1, SIGSEGV
    calli  _onException
    bi     _restore_all_and_return
    nop
    nop
    nop
.size _data_bus_error_handler, .-_data_bus_error_handler

.global _divide_by_zero_handler
.type   _divide_by_zero_handler, @function
_divide_by_zero_handler:
    sw      (sp+0), ra
    calli   _save_all
    mvi     r1,  SIGFPE
    calli   _onException
    bi      _restore_all_and_return
    nop
    nop
    nop
.size _divide_by_zero_handler, .-_divide_by_zero_handler

.global _interrupt_handler
.type   _interrupt_handler, @function
_interrupt_handler:
#ifdef CONFIG_RTOS
    /*
     * In the case of using FreeRTOS the interrupt handling will made in module portasm.S.
     * The in portasm.S implemented function vPortInterruptHandler will invoke the
     * interrupt main function _irq_entry.
     */
    bi      vPortInterruptHandler    /* 1 */
    nop                              /* 2 */
    nop                              /* 3 */
    nop                              /* 4 */
    nop                              /* 5 */
#else
    sw      (sp+0), ra               /* 1 */
    calli   _save_all                /* 2 */
    mvi     r1, SIGINT               /* 3 */
 #ifndef CONFIG_NO_INTERRUPTS
    calli   _irq_entry               /* 4 */
 #else
    wcsr    IE, r0                   /* 4 */
 #endif
    bi      _restore_all_and_return  /* 5 */
#endif
    nop                              /* 6 */
    nop                              /* 7 */
    nop                              /* 8 */
.size     _interrupt_handler, .-_interrupt_handler
.global _system_call_handler
.type _system_call_handler, @function
_system_call_handler:
    sw      (sp+0), ra
    calli   _save_all
    mv      r1, sp
    calli   _onSysCall
    bi      _restore_all_and_return
    nop
    nop
    nop
.size _system_call_handler, .-_system_call_handler

/* ----------------------------------------------------------------------------
 * Function _crt0
 */
.section    .text, "ax", @progbits
.global _crt0
.type   _crt0, @function
_crt0:
    /* Clear r0 */
    xor     r0, r0, r0
    /* Setup stack and global pointer */
    mvhi    sp,     hi(_fstack)
    ori     sp, sp, lo(_fstack)

    /* 
     * Initializing the stack overflow indicator variable
     */
    mvhi    r1,     hi(_endram)
    ori     r1, r1, lo(_endram)
    mvhi    r2,     hi(STACK_MAGIC)
    ori     r2, r2, lo(STACK_MAGIC)
    sw      (r1+0), r2 /* _endram = STACK_MAGIC; */
    
    /*
     * Initializing .bss by zero.
     */
    mvhi    r1,     hi(_fbss)
    ori     r1, r1, lo(_fbss)
    mvi     r2, 0
    mvhi    r3,     hi(_ebss)
    ori     r3, r3, lo(_ebss)
    sub     r3, r3, r1
    calli   memset      /* memset( _fbss, 0, _ebss - _fbss ); */

#ifndef CONFIG_NO_INTERRUPTS
    /*
     * In the case of a unexpected reset event within a atomic section the
     * atomic nesting counter has to be pre initialized with 1 here!
     * This makes it possible using atomic sections before the 
     * interrupts are initialized and enabled.
     */
    mvhi    r1,     hi(__atomic_section_nesting_count)
    ori     r1, r1, lo(__atomic_section_nesting_count)
    mvi     r2, 1       /* __atomic_section_nesting_count = 1; */
    sw      (r1+0), r2
#endif
#ifndef CONFIG_NO_RESET_COUNTER
    /*
     * Incrementing the reset counter: __reset_count
     */
    mvhi    r1,     hi(__reset_count)
    ori     r1, r1, lo(__reset_count)
    lw      r1, (r1+0)
    addi    r2, r1, 1
    mvhi    r1,     hi(__reset_count)
    ori     r1, r1, lo(__reset_count)
    sw      (r1+0), r2
#endif
    /*
     * The C-function __init() is optional and is implemented in module stubs.c by default. 
     * It is called BEFORE the main function. 
     */
    calli   __init

    /*
     * Calling the C-function main( 0, 0, 0 )
     */
    mvi     r1, 0
    mvi     r2, 0
    mvi     r3, 0
    calli   main

    /*
     * The C-function __exit() is optional and is implemented in module stubs.c by default.
     * It is called AFTER the main function.
     */
    calli   __exit

    /*
     * Endless loop!
     */
loopf:
    bi loopf

/* ----------------------------------------------------------------------------
 * Function _save_all
 */
.global _save_all
.type   _save_all, @function
_save_all:
#ifdef MICO32_FULL_CONTEXT_SAVE_RESTORE
    addi    sp, sp, -128
#else
    addi    sp, sp, -60
#endif
    sw      (sp+4), r1
    sw      (sp+8), r2
    sw      (sp+12), r3
    sw      (sp+16), r4
    sw      (sp+20), r5
    sw      (sp+24), r6
    sw      (sp+28), r7
    sw      (sp+32), r8
    sw      (sp+36), r9
    sw      (sp+40), r10
#ifdef MICO32_FULL_CONTEXT_SAVE_RESTORE
    sw      (sp+44), r11
    sw      (sp+48), r12
    sw      (sp+52), r13
    sw      (sp+56), r14
    sw      (sp+60), r15
    sw      (sp+64), r16
    sw      (sp+68), r17
    sw      (sp+72), r18
    sw      (sp+76), r19
    sw      (sp+80), r20
    sw      (sp+84), r21
    sw      (sp+88), r22
    sw      (sp+92), r23
    sw      (sp+96), r24
    sw      (sp+100), r25
    sw      (sp+104), r26
    sw      (sp+108), r27
    sw      (sp+120), ea
    sw      (sp+124), ba
    /* ra and sp need special handling, as they have been modified */
    lw      r1, (sp+128)
    sw      (sp+116), r1
    mv      r1, sp
    addi    r1, r1, 128
    sw      (sp+112), r1
#else
    sw      (sp+52), ea
    sw      (sp+56), ba
    /* ra and sp need special handling, as they have been modified */
    lw      r1, (sp+60)
    sw      (sp+48), r1
    mv      r1, sp
    addi    r1, r1, 60
    sw      (sp+44), r1
#endif
//    xor     r1, r1, r1
//    wcsr    ie, r1
    ret
.size   _save_all, .-_save_all

/* ----------------------------------------------------------------------------
 * Function _restore_all_and_return
 */
.global _restore_all_and_return
.type   _restore_all_and_return, @function
    /* Restore all registers and return from exception */
_restore_all_and_return:
//    addi    r1, r0, 2
//    wcsr    ie, r1
    lw      r1, (sp+4)
    lw      r2, (sp+8) 
    lw      r3, (sp+12) 
    lw      r4, (sp+16) 
    lw      r5, (sp+20) 
    lw      r6, (sp+24) 
    lw      r7, (sp+28) 
    lw      r8, (sp+32) 
    lw      r9, (sp+36) 
    lw      r10, (sp+40)
#ifdef MICO32_FULL_CONTEXT_SAVE_RESTORE
    lw      r11, (sp+44)
    lw      r12, (sp+48)
    lw      r13, (sp+52)
    lw      r14, (sp+56)
    lw      r15, (sp+60)
    lw      r16, (sp+64)
    lw      r17, (sp+68)
    lw      r18, (sp+72)
    lw      r19, (sp+76)
    lw      r20, (sp+80)
    lw      r21, (sp+84)
    lw      r22, (sp+88)
    lw      r23, (sp+92)
    lw      r24, (sp+96)
    lw      r25, (sp+100)
    lw      r26, (sp+104)
    lw      r27, (sp+108)
    lw      ra, (sp+116)
    lw      ea, (sp+120)
    lw      ba, (sp+124)
    /* Stack pointer must be restored last, in case it has been updated */
    lw      sp, (sp+112)
#else
    lw      ra, (sp+48)
    lw      ea, (sp+52)
    lw      ba, (sp+56)
    /* Stack pointer must be restored last, in case it has been updated */
    lw      sp, (sp+44)
#endif
    nop
    eret
.size   _restore_all_and_return, .-_restore_all_and_return

/*================================== EOF ====================================*/
