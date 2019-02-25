
MIAN_MODULE := ../ddr3test.c

SOURCE += $(SCU_LIB_SRC_DIR)/scu_ddr3.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_ramBuffer.c
SOURCE += $(SCU_LIB_SRC_DIR)/eb_object_transfer.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

DEFINES += CONFIG_DDR_PEDANTIC_CHECK
DEFINES += CONFIG_RAM_PEDANTIC_CHECK
DEFINES += DEBUGLEVEL=2
# DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS
DEFINES += CONFIG_SCU_USE_DDR3

CODE_OPTIMIZATION = 2

CALL_ARGS         = "tcp/$(SCU_URL)"
REMOTE_CALL_ARGS  = "dev/wbm0"

LIBS += etherbone
DEBUG = 1

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
