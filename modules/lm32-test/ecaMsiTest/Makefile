REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

MIAN_MODULE := ecaMsiTest.c

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

CODE_OPTIMIZATION = s

SOURCE += $(SCU_LIB_SRC_DIR)/scu_std_init.c
SOURCE += $(SCU_LIB_SRC_DIR)/lm32Interrupts.c
SOURCE += $(SCU_LIB_SRC_DIR)/eca_queue_type.c

DEFINES += CONFIG_INTERRUPT_PEDANTIC_CHECK
DEFINES += MAX_LM32_INTERRUPTS=1

# For DOXYGEN only, header files which doesn't have a appropriate C-file.
DOX_INPUT += $(SCU_LIB_SRC_DIR)/scu_msi.h


include $(REPOSITORY_DIR)/makefiles/makefile.scu
