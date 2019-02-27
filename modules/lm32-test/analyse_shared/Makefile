MIAN_MODULE := analize_shm.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)


CODE_OPTIMIZATION = 2
# CFLAGS = -fno-toplevel-reorder

# SHARED_SIZE = 34816
SHARED_SIZE = 32952

SOURCE += $(SCU_LIB_SRC_DIR)/shared_memory_helper.c
SOURCE += $(SCU_LIB_SRC_DIR)/aux.c
SOURCE += $(SCU_LIB_SRC_DIR)/irq.c

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================


