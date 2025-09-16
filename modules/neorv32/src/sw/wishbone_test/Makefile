# Toolchain detection
RISC_V_ELF_GCC := $(shell command -v riscv64-unknown-elf-gcc 2>/dev/null)
RISC_V_LINUX_GCC := $(shell command -v riscv64-linux-gnu-gcc 2>/dev/null)

ifeq ($(RISC_V_ELF_GCC),)
  ifeq ($(RISC_V_LINUX_GCC),)
    $(error "No suitable RISC-V compiler found!")
  else
    RISC_V_COMPILER = riscv64-linux-gnu
   $(warning Using Linux userland RISC-V compiler!)
  endif
else
  RISC_V_COMPILER = riscv64-unknown-elf
endif

# Directories and includes
NEORV32_INC_FILES_DIR := ../../../../../ip_cores/neorv32/sw/lib/include/
NEORV32_SRC_FILES_DIR := ../../../../../ip_cores/neorv32/sw/lib/source/
PICOLIBC_INC := /usr/lib/picolibc/$(RISC_V_COMPILER)/include
PICOLIBC_LIB := /usr/lib/picolibc/$(RISC_V_COMPILER)/lib/rv32i/ilp32

# Architecture flags (common to C and ASM)
ARCH_ABI_FLAGS := -march=rv32izicsr -mabi=ilp32

# Flags
COMMON_CFLAGS := $(ARCH_ABI_FLAGS) -ffunction-sections -fdata-sections \
                 -I$(NEORV32_INC_FILES_DIR) -I$(PICOLIBC_INC)
COMMON_ASFLAGS := $(ARCH_ABI_FLAGS)
LINKER_FLAGS := -nostartfiles -nostdlib $(ARCH_ABI_FLAGS) -T linker.ld \
                -Wl,--build-id=none -lgcc -lc -L$(PICOLIBC_LIB) -Wl,--gc-sections

# Source files
C_SOURCES := \
  main.c \
  $(NEORV32_SRC_FILES_DIR)neorv32_uart.c \
  $(NEORV32_SRC_FILES_DIR)neorv32_aux.c \
  $(NEORV32_SRC_FILES_DIR)neorv32_cpu.c \
  $(NEORV32_SRC_FILES_DIR)neorv32_rte.c

ASM_SOURCES := start.s

# Object files
OBJS := $(ASM_SOURCES:.s=.o) $(C_SOURCES:.c=.o)

all: compile transform

compile: $(OBJS)
	$(RISC_V_COMPILER)-gcc $(OBJS) -o program.elf $(LINKER_FLAGS)

# Pattern rule for C → O
%.o: %.c
	$(RISC_V_COMPILER)-gcc $(COMMON_CFLAGS) -c $< -o $@

# Pattern rule for ASM → O
%.o: %.s
	$(RISC_V_COMPILER)-gcc $(COMMON_ASFLAGS) -c $< -o $@

transform:
	$(RISC_V_COMPILER)-objcopy -O binary program.elf program.bin
	$(RISC_V_COMPILER)-objdump -d program.elf > program.dis
	python3 bin2mif.py

clean:
	rm -f *.o *.dis *.bin *.elf *.mif
