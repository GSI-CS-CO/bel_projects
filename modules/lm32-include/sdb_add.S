	.file	"sdb_add.c"
	.section	.text
	.align 4
	.global	sdb_add
	.type	sdb_add, @function
sdb_add:
.long	0x91600800
	b        ra
	.size	sdb_add, .-sdb_add
	.ident	"GCC: (GNU) 4.5.3"
