###############################################################################
##                                                                           ##
##       Makefile to create a minimal example of using a DAQ channel.        ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/example/Makefile                                        ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     31.05.2019                                                      ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

  USE_FE_LIB_ETHERBONE=1



MIAN_MODULE := daqexample.cpp


SOURCE += $(DAQ_DIR)/daq_ring_admin.c
SOURCE += $(DAQ_DIR)/daq_interface.cpp
SOURCE += $(DAQ_DIR)/daq_administration.cpp

EB_SRC= $(DAQ_DIR)/feSupport/scu/etherbone
SOURCE += $(EB_SRC)/EtherboneConnection.cpp
SOURCE += $(EB_SRC)/BusException.cpp
SOURCE += $(DAQ_DIR)/daq_eb_ram_buffer.cpp

DEFINES += _BSD_SOURCE
DEFINES += _POSIX_C_SOURCE
DEFINES += CONFIG_SCU_USE_DDR3
DEFINES += DEBUGLEVEL=1
DEFINES += CONFIG_DAQ_TEST
# DEFINES += NDEBUG
DEFINES += CONFIG_DAQ_DEBUG
DEFINES += VERSION=0.1
DEFINES += CONFIG_DAQ_DEBUG

DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS
DEFINES += __ETHERBONE_CONNECTION_MUTEX="EtherBoneConnectionMutexDaq"

INCLUDE_DIRS += $(EB_SRC)
LIBS += boost_system
LIBS += pthread
LIBS += stdc++
LIBS += etherbone


CALL_ARGS         = "tcp/$(SCU_URL)"
REMOTE_CALL_ARGS  ?= "dev/wbm0"

LIBS += stdc++
LIBS += etherbone

DEBUG = 1

#------------------------------------------------------------------------------
# Additional source code for Doxygen which will not integrated automatically:
DOX_INPUT += $(DAQ_DIR)/daq_descriptor.h
DOX_INPUT += $(DAQ_DIR)/daq_command_interface.h
DOX_INPUT += $(DAQ_DIR)/generated/shared_mmap.h
DOX_OPTIMIZE_OUTPUT_FOR_C = "NO"
DOX_RECURSIVE = "YES"
#------------------------------------------------------------------------------

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
