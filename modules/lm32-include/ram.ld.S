OUTPUT_FORMAT("elf32-lm32")
ENTRY(_reset_handler)

MEMORY
{
	ram (rwx) :
	ORIGIN = CONFIG_RAMADDR,
	LENGTH = CONFIG_RAMSIZE - CONFIG_STACKSIZE
	stack (rw) :
	ORIGIN = CONFIG_RAMADDR + CONFIG_RAMSIZE - CONFIG_STACKSIZE,
	LENGTH = CONFIG_STACKSIZE
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

	.buildid ADDR(.boot) + CONFIG_BOOTSIZE :
		{ KEEP(*(.buildid .buildid.*)) } > ram

	_fshared = .;
	.shared ADDR(.buildid) + CONFIG_BUILDIDSIZE :
		{ PROVIDE(_startshared = .); *(.shared .shared.*) } > ram
	_eshared = .;

	_ftext = .;
	.text ADDR(.shared) + CONFIG_SHAREDSIZE :
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

	PROVIDE(mprintf = pp_printf);
	PROVIDE(printf = pp_printf);
	PROVIDE(sprintf = pp_sprintf);
	PROVIDE(vprintf = pp_vprintf);
	PROVIDE(vsprintf = pp_vsprintf);
}
