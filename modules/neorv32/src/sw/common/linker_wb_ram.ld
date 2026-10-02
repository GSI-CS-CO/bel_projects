ENTRY(_start)

MEMORY
{
  ROM (rx)  : ORIGIN = 0x71000000, LENGTH = 32K
  /* DMEM_SIZE in Makefile.common supplies __dmem_size in bytes. */
  RAM (rwx) : ORIGIN = 0x80000000, LENGTH = __dmem_size
}

SECTIONS
{
  /* The reset vector must be the first instruction in NEORV32 IMEM. */
  .text :
  {
    KEEP(*(.init))
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
  } > ROM

  /* Initialized data is stored in the firmware image. start.s copies it
   * from its ROM load address to the RAM execution address before main(). */
  .data :
  {
    . = ALIGN(4);
    __data_start = .;

    *(.data*)
    *(.sdata*)

    . = ALIGN(4);
    __data_end = .;
  } > RAM AT > ROM

  __data_load = LOADADDR(.data);
  __global_pointer$ = __data_start + 0x800;

  /* Uninitialized data consumes RAM only; start.s clears this range. */
  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    __bss_start = .;

    *(.bss*)
    *(.sbss*)
    *(COMMON)

    . = ALIGN(4);
    __bss_end = .;
  } > RAM

  /* The stack grows downward from the end of DMEM. Leave room for recursive
   * SDB traversal rather than silently allowing data to fill all RAM. */
  _stack_top = ORIGIN(RAM) + LENGTH(RAM);

  ASSERT(__bss_end < _stack_top - 4096, "Need 4K stack")
}
