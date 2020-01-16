

MIAN_MODULE := lm32RtosTest1.c
USE_RTOS = 1
# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

CODE_OPTIMIZATION = s
# CFLAGS += -fstack-protector

DEFINES += CONFIG_RTOS_PEDANTIC_CHECK

REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
