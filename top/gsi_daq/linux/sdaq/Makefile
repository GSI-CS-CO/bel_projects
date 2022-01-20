###############################################################################
##                                                                           ##
##  Makefile to generate the static library sdaq.a for SCU-DAQ applications  ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    gsi_daq/linux/sdaq/Makefile                                      ##
## Author:  Ulrich Becker <u.becker@gsi.de>                                  ##
## Company: GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Date:    10.09.2019                                                       ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
STATIC_LIBRARY := 1

#DEFINES += CONFIG_FW_VERSION_3

TARGET         := sdaq

EB_SRC= $(DAQ_DIR)/feSupport/scu/etherbone
INCLUDE_DIRS += $(EB_SRC)
INCLUDE_DIRS += $(SDAQ_LINUX_DIR)
INCLUDE_DIRS += $(DAQ_DIR)/tools
INCLUDE_DIRS += $(SCU_DIR)/generated

SOURCE =  $(SDAQ_LINUX_DIR)/daq_administration.cpp
SOURCE += $(SDAQ_LINUX_DIR)/daq_interface.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_access.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_base_interface.cpp
SOURCE += $(SCU_LIB_SRC_DIR)/circular_index.c

#DEFINES += _BSD_SOURCE
#DEFINES += _POSIX_C_SOURCE
#DEFINES += _DEFAULT_SOURCE
#---- Necessary for backward compatibility if the compiler version greater or equal 5 ----
#DEFINES += _GLIBCXX_USE_CXX11_ABI=0
#DEFINES += BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#-----------------------------------------------------------------------------------------

#DEFINES += DAQ_MAX_CHANNELS=4
#DEFINES += CONFIG_DAQ_TIME_MEASUREMENT
#DEFINES += CONFIG_SCU_USE_DDR3
#DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS

#DEFINES += CONFIG_SCU_DAQ_INTEGRATION

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
