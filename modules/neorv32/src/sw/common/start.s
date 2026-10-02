.section .init
.globl _start

_start:
    # Initialize gp without linker relaxation (gp is not valid yet).
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop

    la sp, _stack_top
    mv x31, sp

    # Copy initialized data from its firmware load address into DMEM.
    la a0, __data_start
    la a1, __data_end
    la a2, __data_load

copy_data:
    bgeu a0, a1, clear_bss_setup
    lw t0, 0(a2)
    sw t0, 0(a0)
    addi a0, a0, 4
    addi a2, a2, 4
    j copy_data

clear_bss_setup:
    # Zero uninitialized data before entering C code.
    la a0, __bss_start
    la a1, __bss_end

clear_bss:
    bgeu a0, a1, enter_main
    sw zero, 0(a0)
    addi a0, a0, 4
    j clear_bss

enter_main:
    call main
    mv x31, a0

halt:
    j halt
