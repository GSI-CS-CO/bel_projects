.section .init
.globl _start
_start:
    la x31, _stack_top
    la sp, _stack_top # Set stack pointer
    jal ra, main
    mv x31, a0        # Get return value from main

exit_label:
    j exit_label
