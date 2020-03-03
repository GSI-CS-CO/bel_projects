

REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

MIAN_MODULE := timerTest.c

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

CODE_OPTIMIZATION = s


RTOS_BASE_DIR   ?= $(REPOSITORY_DIR)/modules/FreeRTOS
RTOS_SRC_DIR    ?= $(RTOS_BASE_DIR)/Source
RTOS_HEADER_DIR ?= $(RTOS_SRC_DIR)/include
RTOS_PORT_DIR   ?= $(RTOS_SRC_DIR)/portable/GCC/SCU_LM32
SOURCE          += $(SCU_LIB_SRC_DIR)/lm32Interrupts.c

DEFINES += CONFIG_INTERRUPT_PEDANTIC_CHECK
DEFINES += MAX_LM32_INTERRUPTS=2

INCLUDE_DIRS   += $(RTOS_PORT_DIR)

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
