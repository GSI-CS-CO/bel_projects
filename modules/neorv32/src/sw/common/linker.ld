ENTRY(_start)

MEMORY
{
  ROM (rx) : ORIGIN = 0x71000000, LENGTH = 32K
  RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 16K
}

SECTIONS
{
  /* Start .text exactly at 0x0 */
  .text 0x71000000 :
  {
    /* Place the entry symbol at the very beginning */
    KEEP(*(.init))
    *(.text*)
    *(.rodata*)
  } > ROM

  .data : {
    *(.data*)
  } > RAM AT > ROM

  .bss : {
    *(.bss*)
    *(COMMON)
  } > RAM

  PROVIDE(_stack_top = ORIGIN(RAM) + LENGTH(RAM));
}

