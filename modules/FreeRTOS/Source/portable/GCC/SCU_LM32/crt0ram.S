/*
 * LatticeMico32 C startup code.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* From include/sys/signal.h */  
#define SIGINT  2   /* interrupt */
#define SIGTRAP 5   /* trace trap */
#define SIGFPE  8   /* arithmetic exception */
#define SIGSEGV 11  /* segmentation violation */

/* Exception handlers - Must be 32 bytes long. */
        .section    .boot, "ax", @progbits        
        .global     _start  
_start:    
        .global _reset_handler
	    .type _reset_handler, @function
_reset_handler:
    /* Clear r0 */
    xor     r0, r0, r0
    wcsr    IE, r0
    wcsr    IM, r0
    mvhi    r1, hi(_reset_handler)
    ori     r1, r1, lo(_reset_handler)
    wcsr    EBA, r1
    calli   _crt0
    nop
	.size     _reset_handler, .-_reset_handler
        .global _breakpoint_handler
	    .type _breakpoint_handler, @function

_breakpoint_handler:
    rcsr    r7, DEBA
    addi    r7, r7, 32
    b       r7
    nop
    nop
    nop
    nop
    nop
	.size     _breakpoint_handler, .-_breakpoint_handler
        .global _instruction_bus_error_handler
	    .type _instruction_bus_error_handler, @function
_instruction_bus_error_handler:
    rcsr    r7, DEBA
    addi    r7, r7, 64
    b       r7
    nop
    nop
    nop
    nop
    nop
	.size     _instruction_bus_error_handler, .-_instruction_bus_error_handler
        .global _watchpoint_handler
	    .type _watchpoint_handler, @function
_watchpoint_handler:
    rcsr    r7, DEBA
    addi    r7, r7, 96
    b       r7
    nop
    nop
    nop
    nop
    nop
	.size     _watchpoint_handler, .-_watchpoint_handler
        .global _data_bus_error_handler
	    .type _data_bus_error_handler, @function
_data_bus_error_handler:
    rcsr    r7, DEBA
    addi    r7, r7, 128
    b       r7
    nop
    nop
    nop
    nop
    nop
	.size     _data_bus_error_handler, .-_data_bus_error_handler
        .global _divide_by_zero_handler
	    .type _divide_by_zero_handler, @function
_divide_by_zero_handler:
    rcsr    r7, DEBA
    addi    r7, r7, 160
    b       r7
    nop
    nop
    nop
    nop
    nop
	.size     _divide_by_zero_handler, .-_divide_by_zero_handler
        .global _interrupt_handler
	    .type _interrupt_handler, @function
_interrupt_handler:
    bi		vPortInterruptHandler
    nop
    nop
    nop
    nop
    nop
    nop
    nop
	.size     _interrupt_handler, .-_interrupt_handler
        .global _system_call_handler
	    .type _system_call_handler, @function
_system_call_handler:
    rcsr    r7, DEBA
    addi    r7, r7, 224
    b       r7
    nop
    nop
    nop
    nop
    nop
	.size     _system_call_handler, .-_system_call_handler
        .global _crt0
	    .type _crt0, @function
_crt0:
    /* Setup stack and global pointer */
    mvhi    sp, hi(_fstack)
    ori     sp, sp, lo(_fstack)
    mvhi    gp, hi(_gp)
    ori     gp, gp, lo(_gp)
    /* Clear BSS */
    mvhi    r1, hi(_fbss)
    ori     r1, r1, lo(_fbss)
    mvhi    r3, hi(_ebss)
    ori     r3, r3, lo(_ebss)
	.size     _crt0, .-_crt0
        .global .ClearBSS
	    .type .ClearBSS, @function
.ClearBSS:
    be      r1, r3, .CallConstructor
    sw      (r1+0), r0
    addi    r1, r1, 4
    bi      .ClearBSS
	.size     .ClearBSS, .-.ClearBSS
        .global .CallConstructor
	    .type .CallConstructor, @function
.CallConstructor:
    /* Call C++ constructors */
    calli   _init
    
    /* Call main program */
    mvi     r1, 0
    mvi     r2, 0
    mvi     r3, 0

    /* invoke application's main routine*/
    calli	main
 
    /* Call exit, which doesn't return, to perform any clean up */
    calli   _fini
    calli   _exit
    
