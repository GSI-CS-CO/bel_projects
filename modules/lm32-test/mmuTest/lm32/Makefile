

MIAN_MODULE := mmutest.c

CODE_OPTIMIZATION = s

SOURCE += $(SCU_LIB_SRC_DIR)/scu_std_init.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_ddr3.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_mmu.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_mmu_lm32.c
SOURCE += $(SCU_LIB_SRC_DIR)/lm32_hexdump.c

DEFINES += CONFIG_SCU_USE_DDR3
DEFINES += CONFIG_DDR_PEDANTIC_CHECK
#DEFINES += CONFIG_DEBUG_MMU

# NO_LTO := 1

REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)
include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
