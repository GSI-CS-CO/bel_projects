###############################################################################
##                                                                           ##
##  Makefile to generate the static library libfg_feedback.a for function-   ##
##  Generator feedback applications.
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    gsi_daq/linux/Makefile                                           ##
## Author:  Ulrich Becker <u.becker@gsi.de>                                  ##
## Company: GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Date:    10.09.2019                                                       ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
STATIC_LIBRARY := 1

TARGET         := fg_feedback

EB_SRC= $(DAQ_DIR)/feSupport/scu/etherbone
INCLUDE_DIRS += $(EB_SRC)
INCLUDE_DIRS += $(SDAQ_LINUX_DIR)
INCLUDE_DIRS += $(MDAQ_LINUX_DIR)

SOURCE =  $(SDAQ_LINUX_DIR)/daq_administration.cpp
SOURCE += $(SDAQ_LINUX_DIR)/daq_interface.cpp
SOURCE += $(MDAQ_LINUX_DIR)/mdaq_administration.cpp
SOURCE += $(MDAQ_LINUX_DIR)/mdaq_interface.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
SOURCE += $(DAQ_LINUX_DIR)/scu_fg_feedback.cpp
SOURCE += $(DAQ_LINUX_DIR)/scu_fg_list.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_base_interface.cpp
SOURCE += $(SCU_LIB_SRC_DIR)/circular_index.c
#SOURCE += $(DAQ_DIR)/daq_ring_admin.c

DEFINES += CONFIG_MIL_FG

DEFINES += _BSD_SOURCE
DEFINES += _POSIX_C_SOURCE
DEFINES += _DEFAULT_SOURCE

#---- Necessary for backward compatibility if the compiler version greater or equal 5 ----
DEFINES += _GLIBCXX_USE_CXX11_ABI=0
DEFINES += BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#-----------------------------------------------------------------------------------------

DEFINES += DAQ_MAX_CHANNELS=4

DEFINES += CONFIG_DAQ_TIME_MEASUREMENT
DEFINES += CONFIG_SCU_USE_DDR3
DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS


DOX_INPUT += $(DAQ_DIR)/daq_descriptor.h
DOX_INPUT += $(DAQ_DIR)/daq_command_interface.h
DOX_INPUT += $(DAQ_DIR)/daq_ring_admin.h


include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================

