REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

MIAN_MODULE := ecaMsiTest.c

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

CODE_OPTIMIZATION = s

# NEW := 1

ifdef NEW
 SOURCE          += $(SCU_LIB_SRC_DIR)/lm32Interrupts.c
 DEFINES += _CONFIG_NEW_MSI
else
 SOURCE          += $(SCU_LIB_SRC_DIR)/irq.c
endif

DEFINES += CONFIG_INTERRUPT_PEDANTIC_CHECK
#DEFINES += MAX_LM32_INTERRUPTS=4


include $(REPOSITORY_DIR)/makefiles/makefile.scu
