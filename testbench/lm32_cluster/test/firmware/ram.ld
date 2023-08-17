OUTPUT_FORMAT("elf32-lm32")
ENTRY(_reset_handler)

MEMORY
{
	ram (rwx) :
	ORIGIN = 0x10000000,
	LENGTH = 131072 - 10240
	stack (rw) :
	ORIGIN = 0x10000000 + 131072 - 10240,
	LENGTH = 10240
}

_fstack = ORIGIN(stack) + LENGTH(stack) - 4;
_endram = ORIGIN(stack);

SECTIONS
{
	. = ORIGIN(ram);

	_fboot = .;
	.boot :
		{ *(.boot) } > ram
	_eboot = .;

	.buildid ADDR(.boot) + 0x100 :
		{ KEEP(*(.buildid .buildid.*)) } > ram

	_fshared = .;
	.shared ADDR(.buildid) + 0x400 :
		{ PROVIDE(_startshared = .); *(.shared .shared.*) } > ram
	_eshared = .;

	_ftext = .;
	.text ADDR(.shared) + 80K :
		{ *(.text .text.*) } > ram
	_etext = .;

	_frodata = .;
	.rodata :
		{ *(.rodata .rodata.*) } > ram
	_erodata = .;


	_fdata = .;
	.data :
		{ *(.data .data.*) } > ram
	_edata = .;

	_fbss = .;
	.bss : { *(.bss .bss.*) *(COMMON) } > ram
	_ebss = .;

}
