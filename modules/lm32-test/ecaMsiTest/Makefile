REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

MIAN_MODULE := ecaMsiTest.c

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

CODE_OPTIMIZATION = s


# SOURCE          += $(SCU_LIB_SRC_DIR)/lm32Interrupts.c
SOURCE          += $(SCU_LIB_SRC_DIR)/irq.c

DEFINES += CONFIG_INTERRUPT_PEDANTIC_CHECK
#DEFINES += MAX_LM32_INTERRUPTS=4


include $(REPOSITORY_DIR)/makefiles/makefile.scu
