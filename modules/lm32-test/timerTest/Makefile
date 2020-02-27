

REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

MIAN_MODULE := timerTest.c

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

CODE_OPTIMIZATION = s


RTOS_BASE_DIR   ?= $(REPOSITORY_DIR)/modules/FreeRTOS
RTOS_SRC_DIR    ?= $(RTOS_BASE_DIR)/Source
RTOS_HEADER_DIR ?= $(RTOS_SRC_DIR)/include
RTOS_PORT_DIR   ?= $(RTOS_SRC_DIR)/portable/GCC/SCU_LM32
# STARTUP_CODE    = $(RTOS_PORT_DIR)/crt0FreeRTOS.S
STARTUP_CODE   = crt0intTest.S
# SOURCE         += $(RTOS_PORT_DIR)/portasm.S
SOURCE         += $(RTOS_PORT_DIR)/lm32Interrupts.c

INCLUDE_DIRS   += $(RTOS_PORT_DIR)

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
