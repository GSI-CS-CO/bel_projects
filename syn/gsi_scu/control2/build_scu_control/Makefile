###############################################################################
##                                                                           ##
##              Makefile LM32 FG software scu_control.bin                    ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    Makefile                                                         ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    09.01.2019                                                       ##
###############################################################################

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
TARGET_DIR     := ..
DEPLOY_DIR     := .
TARGET         := scu_control
MIAN_MODULE    = $(SCU_DIR)/main.c

DEFINES += HISTORY
DEFINES += CONFIG_NO_LM32_ASSERT

# New functions will not compiled when the following switch is set.
DEFINES += CONFIG_OLD_SCU_SW

SOURCE += $(SCU_LIB_SRC_DIR)/aux.c
SOURCE += $(SCU_LIB_SRC_DIR)/irq.c
SOURCE += $(SCU_LIB_SRC_DIR)/display.c
SOURCE += $(SCU_LIB_SRC_DIR)/wr_time.c
SOURCE += $(SCU_LIB_SRC_DIR)/dbg.c
SOURCE += $(SCU_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/fg.c
SOURCE += $(SCU_DIR)/cb.c
SOURCE += $(SCU_DIR)/dow_crc.c
SOURCE += $(SCU_DIR)/history.c
SOURCE += $(SCU_DIR)/scu_mil.c
SOURCE += $(WR_DIR)/dev/w1-temp.c

CODE_OPTIMIZATION = 2
SHARED_SIZE = 34816

# Absolutely necessary for the correct order of the variables
# within the shared memory!
CFLAGS += -fno-toplevel-reorder

# To be 100% backward compatible we can use the old toolchain.
# If the variable TOOLCHAIN_DIR commented out, then the first toolchain found
# in the environment variable $PATH will used if present,
# else the included toolchain of this repository will used never the less. 
TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

# Including common makefile for arbitrary LM32 software for SCU
include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
