ENTRY(_start)

MEMORY
{
  ROM (rx) : ORIGIN = 0x90000000, LENGTH = 32K
  RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 16K
}

SECTIONS
{
  /* Start .text exactly at 0x0 */
  .text 0x90000000 :
  {
    /* Place the entry symbol at the very beginning */
    . = ALIGN(4);
    KEEP(*(.init))
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
  } > ROM

  .data : {
    . = ALIGN(4);
    *(.data*)
    . = ALIGN(4);
  } > RAM AT > ROM

  .bss : {
    . = ALIGN(4);
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
  } > RAM

  PROVIDE(_stack_top = ORIGIN(RAM) + LENGTH(RAM));
}

