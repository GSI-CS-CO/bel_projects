# this is basically a copy of what the gcc compiler generates under -Os flags from the C code :
#
#     void delay_96plus32n_ns(uint32_t n) {
#       while(n--) asm("nop"); // if asm("nop") is missing, compiler will optimize the loop away
#     }
#
# The following code takes exaclty 32 ns per loop iteration on an LM32 with 62.5 MHz clock
# plus 96 ns offset
	.file	"wr_mil_delay_96plus32n_ns.s"
	.section	.text
	.align 4
	.global	delay_96plus32n_ns
	.type	delay_96plus32n_ns, @function
delay_96plus32n_ns:
	bi       .L3
.L4:
	nop
	addi     r1, r1, -1
.L3:
	bne    r1,r0,.L4
	b        ra
	.size	delay_96plus32n_ns, .-delay_96plus32n_ns
	.ident	""
