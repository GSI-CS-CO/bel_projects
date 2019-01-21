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

SYN_DIR    = ../../../..
TARGET_DIR = ..
DEPLOY_DIR = .

TARGET = scu_control
MIAN_MODULE = $(SCU_DIR)/main.c

DEFINES += HISTORY
DEFINES += CONFIG_NO_LM32_ASSERT

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
CFLAGS += -fno-toplevel-reorder

# Including common makefile for arbitrary LM32 software for SCU
include $(SYN_DIR)/modules/lm32_makefiles/makefile.scu
#=================================== EOF ======================================
