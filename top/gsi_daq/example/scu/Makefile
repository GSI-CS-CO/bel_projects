###############################################################################
##                                                                           ##
##       Makefile to create a minimal example of using a DAQ channel.        ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/example/scu/Makefile                                    ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     31.05.2019                                                      ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

# USE_SHARED_LIB := 1

EB_SRC= $(DAQ_DIR)/feSupport/scu/etherbone

MIAN_MODULE := daqexample.cpp

ifndef USE_SHARED_LIB
  SOURCE += $(SCU_LIB_SRC_DIR)/circular_index.c
  SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
  SOURCE += $(SDAQ_LINUX_DIR)/daq_interface.cpp
  SOURCE += $(SDAQ_LINUX_DIR)/daq_administration.cpp
  SOURCE += $(DAQ_LINUX_DIR)/daq_base_interface.cpp
endif

SOURCE += $(EB_SRC)/EtherboneConnection.cpp
SOURCE += $(EB_SRC)/BusException.cpp

DEFINES += DEBUGLEVEL=1
DEFINES += CONFIG_DAQ_TEST
# DEFINES += NDEBUG
DEFINES += VERSION=0.1
DEFINES += CONFIG_DAQ_DEBUG
DEFINES += CONFIG_DAQ_TIME_MEASUREMENT


INCLUDE_DIRS += $(EB_SRC)
INCLUDE_DIRS += $(SDAQ_LINUX_DIR)
INCLUDE_DIRS += $(DAQ_DIR)/tools
# LIBS += boost_system

# ADDITIONAL_OBJECTS += /opt/gsi/3rdparty/boost/1.54.0/lib/x86_64/libboost_system.a
ifdef USE_SHARED_LIB
  SDAQ_LIB = $(SDAQ_LINUX_DIR)/deploy_x86_64/result/libsdaq.a
  ADDITIONAL_OBJECTS += $(SDAQ_LIB)
endif

LIBS += pthread
LIBS += stdc++
LIBS += etherbone


CALL_ARGS         = "tcp/$(SCU_URL)"
REMOTE_CALL_ARGS  ?= "dev/wbm0"

DEBUG = 1

#------------------------------------------------------------------------------
# Additional source code for Doxygen which will not integrated automatically:
DOX_INPUT += $(DAQ_DIR)/daq_descriptor.h
DOX_INPUT += $(DAQ_DIR)/daq_command_interface.h
DOX_INPUT += $(DAQ_DIR)/generated/shared_mmap.h
DOX_INPUT += $(DAQ_LINUX_DIR)/daq_calculations.hpp
DOX_OPTIMIZE_OUTPUT_FOR_C = "NO"
DOX_RECURSIVE = "YES"
#------------------------------------------------------------------------------

ALL = $(SDAQ_LIB)

include $(REPOSITORY_DIR)/makefiles/makefile.scun

ifdef USE_SHARED_LIB
$(SDAQ_LIB):
	make -C $(SDAQ_LINUX_DIR)
endif
#=================================== EOF ======================================
