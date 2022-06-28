###############################################################################
##                                                                           ##
##    Defining of some directory variables and include directories of SCU    ##
##                          for SCU makefiles                                ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    scu_dirs.inc                                                     ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    22.01.2019                                                       ##
###############################################################################

TOP_DIR = $(REPOSITORY_DIR)/top

SCU_DIR         = $(TOP_DIR)/gsi_scu
DAQ_DIR         = $(TOP_DIR)/gsi_daq
DAQ_LM32_DIR    = $(DAQ_DIR)/lm32
DAQ_LINUX_DIR   = $(DAQ_DIR)/linux
SDAQ_LINUX_DIR  = $(DAQ_LINUX_DIR)/sdaq
MDAQ_LINUX_DIR  = $(DAQ_LINUX_DIR)/mdaq

SCU_LIB_SRC_DIR = $(REPOSITORY_DIR)/modules/lm32-include
IP_CORE_DIR     = $(REPOSITORY_DIR)/ip_cores
WR_DIR          = $(IP_CORE_DIR)/wrpc-sw

THIRD_PARTY_DIR = /opt/gsi/3rdparty

INCLUDE_DIRS += $(DAQ_DIR)
# INCLUDE_DIRS += $(DAQ_LM32_DIR)/generated
INCLUDE_DIRS += $(DAQ_LM32_DIR)
INCLUDE_DIRS += $(DAQ_LINUX_DIR)
INCLUDE_DIRS += $(SCU_LIB_SRC_DIR)
INCLUDE_DIRS += $(SCU_DIR)
INCLUDE_DIRS += $(REPOSITORY_DIR)/tools
INCLUDE_DIRS += $(WR_DIR)/sdb-lib
INCLUDE_DIRS += $(WR_DIR)/pp_printf
INCLUDE_DIRS += $(IP_CORE_DIR)/etherbone-core/hdl/eb_master_core
INCLUDE_DIRS += $(IP_CORE_DIR)/wr-cores/modules/wr_eca
INCLUDE_DIRS += $(IP_CORE_DIR)/saftlib/drivers

SCU_VERSION ?= 3
DEFINES += CONFIG_SCU=$(SCU_VERSION)

# ------------ Additional defines for Doxygen ---------------------------------
DOX_OUTPUT_DIRECTORY ?= $(HOME)/Documents/scr_doc/$(TARGET)
# DOX_PROJECT_LOGO     ?= $(MAKEFILE_DIR)/GSI_Logo_rgb_72dpi.png
DOX_PROJECT_LOGO     ?= $(MAKEFILE_DIR)/fair_gsi_logo63x55.png
DOX_INPUT            += $(MAKEFILE_DIR)/scu_dirs.inc
# -------------- End of defines for Doxygen -----------------------------------
#=================================== EOF ======================================
