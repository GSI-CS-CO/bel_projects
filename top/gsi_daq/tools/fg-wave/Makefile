###############################################################################
##                                                                           ##
##        Makefile for building the commandline tool "fg-feedback"           ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/tools/fg-wave/Makefile                                  ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     07.12.2020                                                      ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
DEFINES += VERSION=0.1

MIAN_MODULE := fg-wave.cpp

BASE_DIR := ..


SOURCE += $(BASE_DIR)/parse_opts.cpp
SOURCE += $(BASE_DIR)/gnuplotstream.cpp


GNUPLOT_BIN ?= $(shell which gnuplot)

DEFINES += CONFIG_AUTODOC_OPTION


DEFINES += _BSD_SOURCE
DEFINES += _POSIX_C_SOURCE
# DEFINES += CONFIG_DAQ_TEST
# DEFINES += NDEBUG
DEFINES += CONFIG_DAQ_DEBUG
# DEFINES += DEBUGLEVEL
DEFINES += GPSTR_DEFAULT_GNUPLOT_EXE=\"$(GNUPLOT_BIN)\"

# DEFINES += CONFIG_NO_SCU_RAM

# DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS

#INCLUDE_DIRS += $(EB_SRC)
INCLUDE_DIRS += $(BASE_DIR)
INCLUDE_DIRS += $(DAQ_LINUX_DIR)
#INCLUDE_DIRS += $(DAQ_LINUX_DIR)/sdaq

# LIBS += pthread
LIBS += stdc++
LIBS += m

CALL_ARGS = ../../../gsi_scu/sinus-test.txt

# DEBUG = 1

DOX_OPTIMIZE_OUTPUT_FOR_C = "NO"

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
