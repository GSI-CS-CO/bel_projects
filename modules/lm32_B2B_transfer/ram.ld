MEMORY { RAM (rwx) : ORIGIN = 0x10000000, LENGTH = 65536 }
SECTIONS
{
. = ORIGIN(RAM);
_fstack = . + LENGTH(RAM) - 4;
.boot			: { _fboot   = .; *(.boot); 			     _eboot   = .; } > RAM
.buildid ADDR(.boot)   + 0x100 : { _fbuildid = .; *(.buildid .buildid.*) _ebuildid = .; } > RAM
.shared ADDR(.buildid) + 0x400 : { _fshared = .; PROVIDE(_startshared = .);*(.shared .shared.*)	_eshared = .; } > RAM
.text ADDR(.shared)    + 24K : { _ftext = .; *(.text .text.*)	_etext = .; } > RAM
.rodata	  : { _frodata = .; *(.rodata .rodata.*) _erodata = .; } > RAM
.data			: { _fdata   = .; *(.data .data.*)     _edata   = .; } > RAM
.bss			: { _fbss    = .; *(.bss .bss.*)       _ebss    = .; } > RAM
}
