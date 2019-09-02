###############################################################################
##                                                                           ##
##      Makefile to create a minimal example of using a MIL-DAQ channel.     ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/example/mil/Makefile                                    ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     02.09.2019                                                      ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

EB_SRC= $(DAQ_DIR)/feSupport/scu/etherbone

MIAN_MODULE := mdaqexample.cpp

SOURCE += $(DAQ_DIR)/daq_ring_admin.c
SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
SOURCE += $(MDAQ_LINUX_DIR)/mdaq_interface.cpp
SOURCE += $(MDAQ_LINUX_DIR)/mdaq_administration.cpp
SOURCE += $(EB_SRC)/EtherboneConnection.cpp
SOURCE += $(EB_SRC)/BusException.cpp

DEFINES += _BSD_SOURCE
DEFINES += _POSIX_C_SOURCE
#---- Necessary for backward compatibility if the compiler version greater or equal 5 ----
DEFINES += _GLIBCXX_USE_CXX11_ABI=0
DEFINES += BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#-----------------------------------------------------------------------------------------

DEFINES += CONFIG_SCU_USE_DDR3
DEFINES += DEBUGLEVEL=1
DEFINES += CONFIG_DAQ_TEST
# DEFINES += NDEBUG
DEFINES += CONFIG_DAQ_DEBUG
DEFINES += VERSION=0.1
DEFINES += CONFIG_DAQ_DEBUG

DEFINES += CONFIG_DAQ_TIME_MEASUREMENT

DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS
DEFINES += __ETHERBONE_CONNECTION_MUTEX="EtherBoneConnectionMutexDaq"

INCLUDE_DIRS += $(EB_SRC)
INCLUDE_DIRS += $(MDAQ_LINUX_DIR)
# LIBS += boost_system
 ADDITIONAL_OBJECTS += /opt/gsi/3rdparty/boost/1.54.0/lib/x86_64/libboost_system.a
LIBS += pthread
LIBS += stdc++
LIBS += etherbone


CALL_ARGS         = "tcp/scuxl0107"
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

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
