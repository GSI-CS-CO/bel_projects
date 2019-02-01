
MIAN_MODULE := ddr3test.c

SOURCE += $(SCU_LIB_SRC_DIR)/scu_ddr3.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

DEFINES += CONFIG_DDR_PEDANTIC_CHECK
DEFINES += DEBUGLEVEL=2

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
