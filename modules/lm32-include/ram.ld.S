OUTPUT_FORMAT("elf32-lm32")
ENTRY(_reset_handler)

MEMORY
{
	ram (rwx) :
	ORIGIN = CONFIG_RAMADDR,
	LENGTH = CONFIG_RAMSIZE
}

SECTIONS
{
	. = ORIGIN(ram);
	_fstack = . + LENGTH(ram) - 4;

	.boot :
		{ *(.boot) } > ram

	.buildid ADDR(.boot) + CONFIG_BOOTSIZE :
		{ *(.buildid .buildid.*) } > ram

	.shared ADDR(.buildid) + CONFIG_BUILDIDSIZE :
		{ PROVIDE(_startshared = .); *(.shared .shared.*) } > ram

	.text ADDR(.shared) + CONFIG_SHAREDSIZE :
		{ *(.text .text.*) } > ram

	.rodata :
		{ *(.rodata .rodata.*) } > ram


	.data :
		{ *(.data .data.*) } > ram

	_fbss = .;
	.bss : { *(.bss .bss.*) *(COMMON) } > ram
	_ebss = .;
}
