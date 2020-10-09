###############################################################################
##                                                                           ##
##     Makefile to create a minimal example of using a feedback channel.     ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/example/feedback/Makefile                               ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     26.05.2020                                                      ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

# USE_SHARED_LIB := 1

DEFINES += CONFIG_FW_VERSION_3
DEFINES += DEBUGLEVEL

EB_SRC= $(DAQ_DIR)/feSupport/scu/etherbone

MIAN_MODULE := feedback-example.cpp

ifndef USE_SHARED_LIB
 SOURCE += $(SCU_LIB_SRC_DIR)/circular_index.c
 SOURCE += $(DAQ_LINUX_DIR)/scu_lm32_mailbox.cpp
 SOURCE += $(DAQ_LINUX_DIR)/scu_fg_list.cpp
 SOURCE += $(DAQ_LINUX_DIR)/scu_fg_feedback.cpp
 SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
 SOURCE += $(DAQ_LINUX_DIR)/daq_base_interface.cpp
 SOURCE += $(MDAQ_LINUX_DIR)/mdaq_interface.cpp
 SOURCE += $(MDAQ_LINUX_DIR)/mdaq_administration.cpp
 SOURCE += $(SDAQ_LINUX_DIR)/daq_administration.cpp
 SOURCE += $(SDAQ_LINUX_DIR)/daq_interface.cpp
else
 # When a library will used so the following header files
 # has to be told Doxygen explicitly.
 DOX_INPUT += $(DAQ_LINUX_DIR)/scu_fg_list.hpp
 DOX_INPUT += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.hpp
 DOX_INPUT += $(MDAQ_LINUX_DIR)/mdaq_interface.hpp
 DOX_INPUT += $(MDAQ_LINUX_DIR)/mdaq_administration.hpp
endif

DOX_INPUT += $(DAQ_DIR)/daq_ring_admin.h

SOURCE += $(EB_SRC)/EtherboneConnection.cpp
SOURCE += $(EB_SRC)/BusException.cpp

DEFINES += CONFIG_MIL_FG
DEFINES += CONFIG_SCU_DAQ_INTEGRATION
DEFINES += _BSD_SOURCE
DEFINES += _POSIX_C_SOURCE
DEFINES += _DEFAULT_SOURCE
#---- Necessary for backward compatibility if the compiler version greater or equal 5 ----
DEFINES += _GLIBCXX_USE_CXX11_ABI=0
DEFINES += BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#-----------------------------------------------------------------------------------------

DEFINES += DAQ_MAX_CHANNELS=4

#DEFINES += CONFIG_NO_SCU_RAM
DEFINES += CONFIG_SCU_USE_DDR3
DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS

INCLUDE_DIRS += $(EB_SRC)
INCLUDE_DIRS += $(MDAQ_LINUX_DIR)
INCLUDE_DIRS += $(SDAQ_LINUX_DIR)
INCLUDE_DIRS += $(DAQ_DIR)/tools
# LIBS += boost_system

# ADDITIONAL_OBJECTS += /opt/gsi/3rdparty/boost/1.54.0/lib/x86_64/libboost_system.a
ifdef USE_SHARED_LIB
 MDAQ_LIB = $(MDAQ_LINUX_DIR)/deploy_x86_64/result/libmdaq.a
 ADDITIONAL_OBJECTS += $(MDAQ_LIB)
endif

LIBS += pthread
LIBS += stdc++
LIBS += etherbone


# CALL_ARGS         = "tcp/scuxl0107"

# CALL_ARGS         = "tcp/scuxl0212"
 CALL_ARGS         = "tcp/scuxl0035"
REMOTE_CALL_ARGS  ?= "dev/wbm0"

DEBUG = 1

#------------------------------------------------------------------------------
# Additional source code for Doxygen which will not integrated automatically:
DOX_INPUT += $(DAQ_DIR)/generated/shared_mmap.h
DOX_INPUT += $(DAQ_LINUX_DIR)/daq_calculations.hpp
DOX_OPTIMIZE_OUTPUT_FOR_C = "NO"
DOX_RECURSIVE = "YES"
#------------------------------------------------------------------------------

 # NO_LTO = 1

ALL = $(MDAQ_LIB)

include $(REPOSITORY_DIR)/makefiles/makefile.scun

ifdef USE_SHARED_LIB

$(MDAQ_LIB):
	make -C $(MDAQ_LINUX_DIR)
endif
#=================================== EOF ======================================
