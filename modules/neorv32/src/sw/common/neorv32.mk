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

# Default target name (can be overridden in project Makefile)
TARGET ?= program

# Default linker script (can be overridden)
LINKER_SCRIPT ?= ../common/linker_wb_ram.ld
DMEM_SIZE ?= 16384

# Directories and includes (can be overridden)
NEORV32_INC_FILES_DIR ?= ../../../../../ip_cores/neorv32/sw/lib/include/
NEORV32_SRC_FILES_DIR ?= ../../../../../ip_cores/neorv32/sw/lib/source/
NEORV32_COMMON_DIR := ../common/
SDB_INCLUDE_DIR ?= ../../../../../ip_cores/fpga-config-space/sdbfs/include/linux
PICOLIBC_INC ?= /usr/lib/picolibc/$(RISC_V_COMPILER)/include
PICOLIBC_LIB ?= /usr/lib/picolibc/$(RISC_V_COMPILER)/lib/rv32i/ilp32
GCC_LIB_DIR := $(dir $(shell $(RISC_V_COMPILER)-gcc -print-libgcc-file-name))

# Architecture flags (common to C and ASM)
ARCH_ABI_FLAGS := -march=rv32i_zicsr_zifencei -mabi=ilp32

# Flags
CFLAGS ?= -Os -g -Wall -Wextra -fno-builtin
COMMON_CFLAGS := $(ARCH_ABI_FLAGS) $(CFLAGS) -ffunction-sections -fdata-sections \
                 -I$(NEORV32_INC_FILES_DIR) -I$(NEORV32_COMMON_DIR) -I$(PICOLIBC_INC) -I$(SDB_INCLUDE_DIR) \
                 -D NEORV32
COMMON_ASFLAGS := $(ARCH_ABI_FLAGS)
LINKER_FLAGS := -nostartfiles -nostdlib $(ARCH_ABI_FLAGS) \
                -Wl,--defsym=__dmem_size=$(DMEM_SIZE) -T $(LINKER_SCRIPT) \
                -Wl,--build-id=none -L$(GCC_LIB_DIR)rv32i/ilp32 -L$(PICOLIBC_LIB) \
                -lc -lgcc -Wl,--gc-sections

# Source files (project can override or extend)
ASM_SOURCES ?= $(NEORV32_COMMON_DIR)start.s

# Object files
OBJS := $(ASM_SOURCES:.s=.o) $(C_SOURCES:.c=.o)

# Each application gets its own object files for the shared sources.
LOCAL_OBJECTS ?= 1

# Applications may keep shared-source objects in their own build directory.
ifeq ($(LOCAL_OBJECTS),1)
  OBJS := $(notdir $(OBJS))
  vpath %.c $(sort $(dir $(C_SOURCES)))
  vpath %.s $(sort $(dir $(ASM_SOURCES)))
endif

# Default targets
all: compile transform

compile: $(OBJS) $(LINKER_SCRIPT)
	$(RISC_V_COMPILER)-gcc $(OBJS) -o $(TARGET).elf $(LINKER_FLAGS)

%.o: %.c
	$(RISC_V_COMPILER)-gcc $(COMMON_CFLAGS) -MMD -MP -c $< -o $@

%.o: %.s
	$(RISC_V_COMPILER)-gcc $(COMMON_ASFLAGS) -c $< -o $@

transform: compile
	$(RISC_V_COMPILER)-objcopy -O binary $(TARGET).elf $(TARGET).bin
	$(RISC_V_COMPILER)-objdump -d $(TARGET).elf > $(TARGET).dis
	python3 ../../scripts/bin2mif.py

clean:
	rm -f *.o *.d *.dis *.bin *.elf *.mif

# Recompile when build settings change; the compiler tracks included headers.
$(OBJS): Makefile $(NEORV32_COMMON_DIR)neorv32.mk
-include $(OBJS:.o=.d)
