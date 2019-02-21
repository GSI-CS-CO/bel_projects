MIAN_MODULE := ../shared.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

SHARED_SIZE     = 32



SOURCE += $(SCU_LIB_SRC_DIR)/shared_memory_helper.c
SOURCE += $(SCU_LIB_SRC_DIR)/aux.c
SOURCE += $(SCU_LIB_SRC_DIR)/irq.c

CODE_OPTIMIZATION = 2

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================


