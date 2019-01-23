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

SCU_DIR = $(SYN_DIR)/top/gsi_scu

SCU_LIB_SRC_DIR = $(SYN_DIR)/modules/lm32-include
WR_DIR          = $(SYN_DIR)/ip_cores/wrpc-sw

INCLUDE_DIRS += $(SCU_LIB_SRC_DIR)
INCLUDE_DIRS += $(SCU_DIR)
INCLUDE_DIRS += $(SYN_DIR)/tools
INCLUDE_DIRS += $(WR_DIR)/include
INCLUDE_DIRS += $(WR_DIR)/sdb-lib
INCLUDE_DIRS += $(WR_DIR)/pp_printf
INCLUDE_DIRS += $(SYN_DIR)/ip_cores/etherbone-core/hdl/eb_master_core

DEFINES += CONFIG_SCU

# ------------ Additional defines for Doxygen ---------------------------------
DOX_OUTPUT_DIRECTORY ?= $(HOME)/Documents/scr_doc/$(TARGET)
DOX_PROJECT_LOGO     ?= $(MAKEFILE_DIR)/GSI_Logo_rgb_72dpi.png
DOX_INPUT            += $(SCU_LIB_SRC_DIR)/helper_macros.h
DOX_INPUT            += $(SCU_LIB_SRC_DIR)/eb_console_helper.h
# -------------- End of defines for Doxygen -----------------------------------
#=================================== EOF ======================================
