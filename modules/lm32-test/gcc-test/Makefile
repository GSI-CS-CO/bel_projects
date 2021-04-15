

REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

MIAN_MODULE := gcc-test.c

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

CODE_OPTIMIZATION = 0

SOURCE += $(SCU_LIB_SRC_DIR)/lm32Interrupts.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_std_init.c


 NO_LTO := 1

include $(REPOSITORY_DIR)/makefiles/makefile.scu



.PHONY: asm
asm:
	$(CC_F) -S -O0 $(OPT_INCLUDE) $(MIAN_MODULE) -o gcc-test_O0.S
	$(CC_F) -S -O1 $(OPT_INCLUDE) $(MIAN_MODULE) -o gcc-test_O1.S
	$(CC_F) -S -O2 $(OPT_INCLUDE) $(MIAN_MODULE) -o gcc-test_O2.S
	$(CC_F) -S -O3 $(OPT_INCLUDE) $(MIAN_MODULE) -o gcc-test_O3.S
	$(CC_F) -S -Os $(OPT_INCLUDE) $(MIAN_MODULE) -o gcc-test_OS.S
	
#=================================== EOF ======================================
