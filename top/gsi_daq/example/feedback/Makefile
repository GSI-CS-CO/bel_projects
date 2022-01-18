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

#  USE_SHARED_LIB := 1

EB_SRC= $(DAQ_DIR)/feSupport/scu/etherbone

MIAN_MODULE := feedback-example.cpp

ifndef USE_SHARED_LIB
 DEFINES += DEBUGLEVEL
 SOURCE += $(SCU_LIB_SRC_DIR)/circular_index.c
 SOURCE += $(DAQ_DIR)/daq_fg_allocator.c
 SOURCE += $(DAQ_LINUX_DIR)/scu_lm32_mailbox.cpp
 SOURCE += $(DAQ_LINUX_DIR)/scu_fg_list.cpp
 SOURCE += $(DAQ_LINUX_DIR)/scu_fg_feedback.cpp
 SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
 SOURCE += $(DAQ_LINUX_DIR)/daq_access.cpp
 SOURCE += $(DAQ_LINUX_DIR)/daq_base_interface.cpp
 SOURCE += $(DAQ_LINUX_DIR)/watchdog_poll.cpp
 SOURCE += $(MDAQ_LINUX_DIR)/mdaq_interface.cpp
 SOURCE += $(MDAQ_LINUX_DIR)/mdaq_administration.cpp
 SOURCE += $(SDAQ_LINUX_DIR)/daq_administration.cpp
 SOURCE += $(SDAQ_LINUX_DIR)/daq_interface.cpp
else
 # When a library will used so the following header files
 # has to be told Doxygen explicitly.
 DOX_INPUT += $(SCU_LIB_SRC_DIR)/circular_index.h
 DOX_INPUT += $(DAQ_LINUX_DIR)/scu_lm32_mailbox.cpp
 DOX_INPUT += $(DAQ_LINUX_DIR)/scu_fg_list.hpp
 DOX_INPUT += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.hpp
 DOX_INPUT += $(MDAQ_LINUX_DIR)/mdaq_interface.hpp
 DOX_INPUT += $(MDAQ_LINUX_DIR)/mdaq_administration.hpp
endif

DOX_INPUT += $(DAQ_DIR)/daq_ring_admin.h

SOURCE += $(EB_SRC)/EtherboneConnection.cpp
SOURCE += $(EB_SRC)/BusException.cpp

INCLUDE_DIRS += $(EB_SRC)
INCLUDE_DIRS += $(MDAQ_LINUX_DIR)
INCLUDE_DIRS += $(SDAQ_LINUX_DIR)
INCLUDE_DIRS += $(DAQ_DIR)/tools
INCLUDE_DIRS += $(SCU_DIR)/generated
# LIBS += boost_system

# ADDITIONAL_OBJECTS += /opt/gsi/3rdparty/boost/1.54.0/lib/x86_64/libboost_system.a
ifdef USE_SHARED_LIB
 FEEDBACK_LIB = $(DAQ_LINUX_DIR)/feedback/deploy_x86_64/result/libscu_fg_feedback.a
 ADDITIONAL_OBJECTS += $(FEEDBACK_LIB)
endif

LIBS += pthread
LIBS += stdc++
LIBS += etherbone


# CALL_ARGS         = "tcp/scuxl0107"
 CALL_ARGS         = "tcp/scuxl0192"
# CALL_ARGS         = "tcp/scuxl0212"
# CALL_ARGS         = "tcp/scuxl0035"
# CALL_ARGS = "tcp/scuxl0118"
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
