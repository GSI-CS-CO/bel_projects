OUTPUT_FORMAT("elf32-lm32")

SECTIONS
{
	.boot   : { _fboot   = .; *(.boot);            _eboot   = .; }
	.shared	: { _fshared = .; *(.shared .shared.*) _eshared = .; }
	.text   : { _ftext   = .; *(.text .text.*)     _etext   = .; }
	.rodata : { _frodata = .; *(.rodata .rodata.*) _erodata = .; }
	.data   : { _fdata   = .; *(.data .data.*)     _edata   = .; }
	.bss    : { _fbss    = .; *(.bss .bss.*)       _ebss    = .; }
}
