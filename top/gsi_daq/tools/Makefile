###############################################################################
##                                                                           ##
##           Makefile for building the commandline tool "daqt"               ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/tools/Makefile                                          ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     11.04.2019                                                      ##
###############################################################################
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

MIAN_MODULE := daqt.cpp

SOURCE += parse_opts.cpp
SOURCE += daqt_command_line.cpp
SOURCE += daqt_attributes.cpp
SOURCE += daqt_read_stdin.cpp
SOURCE += gnuplotstream.cpp
SOURCE += find_process.c

SOURCE += $(SCU_LIB_SRC_DIR)/scu_ddr3.c
SOURCE += $(DAQ_DIR)/daq_ramBuffer.c
SOURCE += $(SCU_LIB_SRC_DIR)/eb_object_transfer.c
SOURCE += $(DAQ_DIR)/daq_interface.cpp
SOURCE += $(DAQ_DIR)/daq_administration.cpp
# SOURCE += $(DAQ_DIR)/scu_eb_interface.cpp

GNUPLOT_BIN ?= $(shell which gnuplot)

DEFINES += _BSD_SOURCE
DEFINES += _POSIX_C_SOURCE
DEFINES += CONFIG_SCU_USE_DDR3
#DEFINES += DEBUGLEVEL=1
DEFINES += CONFIG_DAQ_TEST
# DEFINES += NDEBUG
DEFINES += CONFIG_DAQ_DEBUG
DEFINES += VERSION=0.1
DEFINES += GPSTR_DEFAULT_GNUPLOT_EXE=\"$(GNUPLOT_BIN)\"

 DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS

HOST ?= tcp/$(SCU_URL)

#CALL_ARGS         ?=  -h
#CALL_ARGS         ?= $(HOST) --continue 3 1
#CALL_ARGS         ?= $(HOST) -vs
#CALL_ARGS         ?= $(HOST) 3 -l5 1 3 2 9 3 9 4 -l7 -e
#CALL_ARGS         ?= $(HOST)  -eC 3 1 3 2 3 3 3 4 9 1 9 2 9 3 9 4
#CALL_ARGS         ?= $(HOST) -tEC  3 1
#CALL_ARGS         ?= $(HOST)  -d10 -vtEC 3 1 3 2 -z 9 2
#CALL_ARGS         ?= $(HOST) -Hte 3 1
#CALL_ARGS         ?= $(HOST) -Hrtev 9 1
#CALL_ARGS         ?=  $(HOST) -G /usr/bin/gnuplot -PrzvC 3 1

#CALL_ARGS         ?=  $(HOST) -C 3 1 3 2 9 1 9 2 9 3 9 4
CALL_ARGS         ?=  $(HOST) -C 3 1 3 2
#CALL_ARGS         ?= $(HOST) -n
#CALL_ARGS         ?= -n

REMOTE_CALL_ARGS  ?= "dev/wbm0"

LIBS += stdc++
LIBS += etherbone

DEBUG = 1

DOX_INPUT += $(DAQ_DIR)/daq_descriptor.h
DOX_INPUT += $(DAQ_DIR)/daq_command_interface.h
DOX_OPTIMIZE_OUTPUT_FOR_C = "NO"

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
