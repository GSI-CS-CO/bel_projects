###############################################################################
##                                                                           ##
##  Makefile to generate the static library libscu_fg_feedback.a for MIL-    ##
##       and ADDAC/ACU- DAQs for function generator feedback.                ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    gsi_daq/linux/feedback/Makefile                                  ##
## Author:  Ulrich Becker <u.becker@gsi.de>                                  ##
## Company: GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Date:    14.10.2020                                                       ##
###############################################################################
STATIC_LIBRARY := 1

TARGET         := scu_fg_feedback

INCLUDE_DIRS += $(DAQ_DIR)/feSupport/scu/etherbone
INCLUDE_DIRS += $(MDAQ_LINUX_DIR)
INCLUDE_DIRS += $(SDAQ_LINUX_DIR)
INCLUDE_DIRS += $(DAQ_DIR)/tools
INCLUDE_DIRS += $(SCU_DIR)/generated

SOURCE =  $(MDAQ_LINUX_DIR)/mdaq_administration.cpp
SOURCE += $(MDAQ_LINUX_DIR)/mdaq_interface.cpp
SOURCE += $(SDAQ_LINUX_DIR)/daq_interface.cpp
SOURCE += $(SDAQ_LINUX_DIR)/daq_administration.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_access.cpp
SOURCE += $(DAQ_LINUX_DIR)/scu_lm32_mailbox.cpp
SOURCE += $(DAQ_LINUX_DIR)/scu_fg_list.cpp
SOURCE += $(DAQ_LINUX_DIR)/watchdog_poll.cpp
SOURCE += $(DAQ_LINUX_DIR)/daq_base_interface.cpp
SOURCE += $(DAQ_LINUX_DIR)/scu_fg_feedback.cpp
SOURCE += $(SCU_LIB_SRC_DIR)/circular_index.c
SOURCE += $(DAQ_DIR)/daq_fg_allocator.c

# NO_LTO = 1

ifeq ($(shell echo $${HOSTNAME:0:5}), asl74)
   DOX_OUTPUT_DIRECTORY = /common/usr/cscofe/doc/scu/daq
endif

DOX_EXPAND_AS_DEFINED      = ADD_NAMESPACE

DOX_EXAMPLE_PATH = $(DAQ_LINUX_DIR)/../example/feedback/
DOX_EXTRACT_PRIVATE = "YES"

DOX_INPUT += $(DAQ_LINUX_DIR)/feedback/scu_feedback.md
DOX_FILE_PATTERNS += *.md

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
