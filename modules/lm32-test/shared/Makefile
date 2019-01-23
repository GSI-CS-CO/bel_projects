MIAN_MODULE := shared.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

SHARED_SIZE     = 16

SOURCE += $(SCU_LIB_SRC_DIR)/shared_memory_helper.c
SOURCE += $(SCU_LIB_SRC_DIR)/aux.c
SOURCE += $(SCU_LIB_SRC_DIR)/irq.c

include $(REPOSITORY_DIR)/modules/lm32_makefiles/makefile.scu
#=================================== EOF ======================================


