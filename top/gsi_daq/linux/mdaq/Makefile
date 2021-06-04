###############################################################################
##                                                                           ##
##  Makefile to generate the static library mdaq.a for Mil-DAQ applications  ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    gsi_daq/linux/mdaq/Makefile                                      ##
## Author:  Ulrich Becker <u.becker@gsi.de>                                  ##
## Company: GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Date:    09.09.2019                                                       ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
STATIC_LIBRARY := 1

TARGET         := mdaq

EB_SRC= $(DAQ_DIR)/feSupport/scu/etherbone
INCLUDE_DIRS += $(EB_SRC)
INCLUDE_DIRS += $(MDAQ_LINUX_DIR)
INCLUDE_DIRS += $(DAQ_DIR)/tools

SOURCE =  $(MDAQ_LINUX_DIR)/mdaq_administration.cpp
SOURCE += $(MDAQ_LINUX_DIR)/mdaq_interface.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_access.cpp
SOURCE += $(DAQ_LINUX_DIR)/scu_fg_list.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_base_interface.cpp

# NO_LTO = 1

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
