
MIAN_MODULE := wr-test.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

DEFINES += CONFIG_WR_NODE

SOURCE += $(SCU_LIB_SRC_DIR)/wr_time.c
SOURCE += $(SCU_LIB_SRC_DIR)/aux.c
SOURCE += $(SCU_LIB_SRC_DIR)/irq.c

include $(REPOSITORY_DIR)/modules/lm32_makefiles/makefile.scu
#=================================== EOF ======================================
