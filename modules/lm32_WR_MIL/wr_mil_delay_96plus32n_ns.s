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
	.ident	"hand written"
