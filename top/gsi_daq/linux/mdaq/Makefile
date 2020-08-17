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

SOURCE =  $(MDAQ_LINUX_DIR)/mdaq_administration.cpp
SOURCE += $(MDAQ_LINUX_DIR)/mdaq_interface.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
SOURCE += $(DAQ_LINUX_DIR)/scu_fg_list.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_base_interface.cpp

DEFINES += _BSD_SOURCE
DEFINES += _POSIX_C_SOURCE
DEFINES += _DEFAULT_SOURCE
#---- Necessary for backward compatibility if the compiler version greater or equal 5 ----
DEFINES += _GLIBCXX_USE_CXX11_ABI=0
DEFINES += BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#-----------------------------------------------------------------------------------------

DEFINES += CONFIG_SCU_USE_DDR3
DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS


# DEFINES += CONFIG_NO_SCU_RAM

 # NO_LTO = 1

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
