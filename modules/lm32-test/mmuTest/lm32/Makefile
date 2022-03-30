

REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

MIAN_MODULE := mmutest.c

CODE_OPTIMIZATION = s

SOURCE += $(SCU_LIB_SRC_DIR)/scu_std_init.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_ddr3.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_mmu.c

DEFINES += CONFIG_SCU_USE_DDR3

#INCLUDE_DIRS   += $(RTOS_PORT_DIR)

# NO_LTO := 1

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
