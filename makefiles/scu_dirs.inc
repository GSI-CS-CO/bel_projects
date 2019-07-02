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

SCU_DIR = $(TOP_DIR)/gsi_scu
DAQ_DIR = $(TOP_DIR)/gsi_daq

SCU_LIB_SRC_DIR = $(REPOSITORY_DIR)/modules/lm32-include
IP_CORE_DIR     = $(REPOSITORY_DIR)/ip_cores
WR_DIR          = $(IP_CORE_DIR)/wrpc-sw

INCLUDE_DIRS += $(DAQ_DIR)
INCLUDE_DIRS += $(DAQ_DIR)/generated
INCLUDE_DIRS += $(SCU_LIB_SRC_DIR)
INCLUDE_DIRS += $(SCU_DIR)
INCLUDE_DIRS += $(REPOSITORY_DIR)/tools
INCLUDE_DIRS += $(WR_DIR)/sdb-lib
INCLUDE_DIRS += $(WR_DIR)/pp_printf
INCLUDE_DIRS += $(IP_CORE_DIR)/etherbone-core/hdl/eb_master_core

DEFINES += CONFIG_SCU

# ------------ Additional defines for Doxygen ---------------------------------
DOX_OUTPUT_DIRECTORY ?= $(HOME)/Documents/scr_doc/$(TARGET)
DOX_PROJECT_LOGO     ?= $(MAKEFILE_DIR)/GSI_Logo_rgb_72dpi.png
DOX_INPUT            += $(SCU_LIB_SRC_DIR)/helper_macros.h
DOX_INPUT            += $(SCU_LIB_SRC_DIR)/scu_assert.
DOX_INPUT            += $(SCU_LIB_SRC_DIR)/scu_bus_defines.h
DOX_INPUT            += $(SCU_LIB_SRC_DIR)/eb_console_helper.h
DOX_INPUT            += $(MAKEFILE_DIR)/scu_dirs.inc
# -------------- End of defines for Doxygen -----------------------------------
#=================================== EOF ======================================
