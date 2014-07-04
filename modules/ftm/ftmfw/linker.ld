OUTPUT_FORMAT("elf32-lm32")
ENTRY(_start)
GROUP(-lgcc -lc)
MEMORY
{
    ram :
 ORIGIN = 0x00000000,
 LENGTH = 65536 - 8192 - 8192
    stack :
 ORIGIN = 65536 - 8192,
 LENGTH = 8192
    shared :
 ORIGIN = 65536 - 8192 - 8192,
 LENGTH = 8192
}

SECTIONS
{
 .boot : { *(.boot) } > ram
 .text : { *(.text .text.*) } > ram =0
 .rodata : { *(.rodata .rodata.*) } > ram
 .data : {
  *(.data .data.*)
  _gp = ALIGN(16) + 0x7ff0;
 } > ram
 .bss : {
  _fbss = .;
  *(.bss .bss.*)
  *(COMMON)
  _ebss = .;
 } > ram
 .shared_mem : { *(.shared_mem) } > shared

 PROVIDE(_startshared = ORIGIN(shared));
 PROVIDE(_endshared = ORIGIN(shared) + LENGTH(shared) - 4);

 PROVIDE(_endram = ORIGIN(stack));
 PROVIDE(_fstack = ORIGIN(stack) + LENGTH(stack) - 4);
}
PROVIDE(mprintf = pp_printf);
