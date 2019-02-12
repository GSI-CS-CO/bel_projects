
MIAN_MODULE := ring_index_test.c

SOURCE += $(SCU_LIB_SRC_DIR)/scu_ddr3.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_ramBuffer.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

DEFINES += CONFIG_DDR_PEDANTIC_CHECK
DEFINES += CONFIG_RAM_PEDANTIC_CHECK
#DEFINES += DEBUGLEVEL=2
# DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS
DEFINES += CONFIG_SCU_USE_DDR3

CODE_OPTIMIZATION = 2

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
