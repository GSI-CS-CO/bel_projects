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

# ACU := 1

# Avoiding of including of scu_control_config.h
DEFINES += _SCU_CONTROL_CONFIG_H

ifdef ACU
  TARGET := daq_acu_control
else
  TARGET := daq_addac_control
endif

MIAN_MODULE    = $(DAQ_LM32_DIR)/daq_main.c

SOURCE += $(SCU_LIB_SRC_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/dow_crc.c
SOURCE += $(WR_DIR)/dev/w1-temp.c
SOURCE += $(DAQ_LM32_DIR)/daq.c
SOURCE += $(DAQ_LM32_DIR)/daq_command_interface_uc.c
SOURCE += $(DAQ_LM32_DIR)/daq_ramBuffer.c
SOURCE += $(SCU_LIB_SRC_DIR)/lm32Interrupts.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_ddr3.c
SOURCE += $(SCU_LIB_SRC_DIR)/circular_index.c

ifdef ACU
   DEFINES += MAX_SCU_SLAVES=1
   DEFINES += DAQ_MAX_CHANNELS=4
   DEFINES += CONFIG_ACU
else
   DEFINES += MAX_SCU_SLAVES=12
   DEFINES += DAQ_MAX_CHANNELS=4
endif

DEFINES += CONFIG_DAQ_SINGLE_APP
DEFINES += CONFIG_SCU_USE_DDR3
DEFINES += CONFIG_DAQ_SW_SEQUENCE

DEFINES += CONFIG_DAQ_PEDANTIC_CHECK
DEFINES += CONFIG_RAM_PEDANTIC_CHECK
DEFINES += CONFIG_PATCH_DAQ_HW_BUG
DEFINES += CONFIG_DEBUG_RAM_WRITE_DATA
# DEFINES += DEBUGLEVEL=2
#DEFINES += CONFIG_DAQ_DEBUG

DOX_INPUT += $(DAQ_DIR)/daq_descriptor.h
DOX_INPUT += $(DAQ_DIR)/daq_command_interface.h
DOX_INPUT += $(DAQ_DIR)/daq_ring_admin.h

SHARED_SIZE = 48

RAM_SIZE    = 147456

# NO_REMOVE_UNUSED=1
# NO_PRINTF_SUBST=1
# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)
# NO_LTO = 1
include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
