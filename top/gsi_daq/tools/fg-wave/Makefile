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
DEFINES += VERSION=1.4

MIAN_MODULE := fg-wave.cpp

BASE_DIR := ..

SOURCE += fgw_commandline.cpp
SOURCE += fgw_polynom.cpp
SOURCE += $(BASE_DIR)/parse_opts.cpp
SOURCE += $(BASE_DIR)/gnuplotstream.cpp
SOURCE += $(BASE_DIR)/fgw_parser.cpp
SOURCE += $(BASE_DIR)/daqt_read_stdin.cpp

GNUPLOT_BIN ?= $(shell which gnuplot)

DEFINES += CONFIG_AUTODOC_OPTION
DEFINES += _SCU_CONTROL_CONFIG_H
DEFINES += _DEFAULT_SOURCE
#DEFINES += _BSD_SOURCE
#DEFINES += _POSIX_C_SOURCE

# DEFINES += NDEBUG

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
# LIBS += m

# CALL_ARGS = ../../../gsi_scu/sinus-test.txt
#CALL_ARGS = sinus-test500.fgw -c
CALL_ARGS = ramp_ifa_scu.fgw -c
#CALL_ARGS = gs11mu2-volt-raw-all.fgw -c
# CALL_ARGS = -r3 sinus.fgw
# CALL_ARGS = -z gs11mu2-curr-raw-p1.fgw 
#CALL_ARGS = -zd100 q.fgw
#CALL_ARGS = -zd100 q.fgw
#  DEBUG = 1


DOX_OPTIMIZE_OUTPUT_FOR_C = "NO"

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
