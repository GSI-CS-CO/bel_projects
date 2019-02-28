###############################################################################
##                                                                           ##
##                Makefile for LM32 DAQ-Control Program                      ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:   gsi_daq/buildLm32/Makefile                                        ##
## Author: Ulrich Becker <u.becker@gsi.de>                                   ##
## (C:)    2019 GSI Helmholtz Centre for Heavy Ion Research GmbH             ##
## Date:   27.02.2019                                                        ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

# TARGET_DIR     := ..
# DEPLOY_DIR     := .
TARGET         := daq_control
MIAN_MODULE    = $(DAQ_DIR)/daq_main.c


SOURCE += $(DAQ_DIR)/daq.c
SOURCE += $(DAQ_DIR)/daq_command_interface_uc.c
SOURCE += $(SCU_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/dow_crc.c
SOURCE += $(WR_DIR)/dev/w1-temp.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_ddr3.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_ramBuffer.c

DEFINES += DEBUGLEVEL=1
DEFINES += CONFIG_SCU_USE_DDR3
SHARED_SIZE = 32

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
