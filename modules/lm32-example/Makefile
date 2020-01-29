###############################################################################
##                                                                           ##
##        Simplified Makefile using LM32 build system make templates         ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     modules/lm32-example/Makefile                                   ##
## Author:   Ulrich Becker <u.becker@gsi.de>                                 ##
## (c):      GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

MIAN_MODULE := example.c
SOURCE += $(SCU_DIR)/scu_mil.c
SOURCE += $(SCU_LIB_SRC_DIR)/irq.c

SHARED_SIZE     = 8000

CODE_OPTIMIZATION = s


# TODO There is a bug in find_device_multi_in_subtree when LTO is activated
#      therefore NO_LTO = 1
NO_LTO = 1

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
