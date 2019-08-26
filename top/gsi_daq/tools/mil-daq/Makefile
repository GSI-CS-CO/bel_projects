###############################################################################
##                                                                           ##
##          Makefile for building the commandline tool "mdaqt"               ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/tools/mil_daqMakefile                                   ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     14.08.2019                                                      ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

MIAN_MODULE := mdaqt.cpp

BASE_DIR := ..
EB_SRC= $(DAQ_DIR)/feSupport/scu/etherbone

#SOURCE += daqt_command_line.cpp
#SOURCE += daqt_attributes.cpp

SOURCE += $(BASE_DIR)/daqt_read_stdin.cpp
SOURCE += $(BASE_DIR)/parse_opts.cpp
SOURCE += $(BASE_DIR)/gnuplotstream.cpp
SOURCE += $(BASE_DIR)/find_process.c

SOURCE += $(DAQ_DIR)/daq_ring_admin.c
SOURCE += $(DAQ_LINUX_DIR)/daq_eb_ram_buffer.cpp
SOURCE += $(MDAQ_LINUX_DIR)/mdaq_interface.cpp
SOURCE += $(MDAQ_LINUX_DIR)/mdaq_administration.cpp
SOURCE += mdaq_plot.cpp


SOURCE += $(EB_SRC)/EtherboneConnection.cpp
SOURCE += $(EB_SRC)/BusException.cpp


GNUPLOT_BIN ?= $(shell which gnuplot)

#---- Necessary for backward compatibility if the compiler version greater or equal 5 ----
DEFINES += _GLIBCXX_USE_CXX11_ABI=0
DEFINES += BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#-----------------------------------------------------------------------------------------

DEFINES += _BSD_SOURCE
DEFINES += _POSIX_C_SOURCE
DEFINES += CONFIG_SCU_USE_DDR3
DEFINES += DEBUGLEVEL=1
DEFINES += CONFIG_DAQ_TEST
# DEFINES += NDEBUG
DEFINES += CONFIG_DAQ_DEBUG
DEFINES += VERSION=0.1
DEFINES += GPSTR_DEFAULT_GNUPLOT_EXE=\"$(GNUPLOT_BIN)\"

 DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS

DEFINES += __ETHERBONE_CONNECTION_MUTEX="EtherBoneConnectionMutexDaq"
INCLUDE_DIRS += $(EB_SRC)
INCLUDE_DIRS += $(BASE_DIR)
INCLUDE_DIRS += $(DAQ_LINUX_DIR)/mdaq

LIBS += boost_system
LIBS += pthread
LIBS += stdc++
LIBS += etherbone

DOX_INPUT += $(DAQ_LINUX_DIR)/daq_calculations.hpp

HOST ?= tcp/$(SCU_URL)


#CALL_ARGS         ?=  $(HOST)

CALL_ARGS = tcp/scuxl0107

REMOTE_CALL_ARGS  ?= "dev/wbm0"


DEBUG = 1

DOX_OPTIMIZE_OUTPUT_FOR_C = "NO"

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
