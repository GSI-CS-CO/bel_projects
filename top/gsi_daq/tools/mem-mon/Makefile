###############################################################################
##                                                                           ##
##        Makefile for building the tool "memory-monitor" ("mem-mom")        ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:     gsi_daq/tools/mem-mon/Makefile                                  ##
## Author:   Ulrich Becker                                                   ##
## Company:  GSI Helmholtz Centre for Heavy Ion Research GmbH                ##
## Date:     12.04.2022                                                      ##
###############################################################################

DEFINES += VERSION=1.1
FOR_SCU_AND_ACC := 1
MIAN_MODULE := mem-mon.cpp

INCLUDE_DIRS += $(BASE_DIR)
INCLUDE_DIRS += $(DAQ_LINUX_DIR)

BASE_DIR := ..

EB_SRC= $(DAQ_DIR)/feSupport/scu/etherbone

SOURCE += mem_browser.cpp
SOURCE += mem_cmdline.cpp
SOURCE += $(BASE_DIR)/parse_opts.cpp
SOURCE += $(SCU_LIB_SRC_DIR)/scu_mmu.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_mmu_fe.cpp
SOURCE += $(EB_SRC)/EtherboneConnection.cpp
SOURCE += $(EB_SRC)/BusException.cpp

INCLUDE_DIRS += $(EB_SRC)

DEFINES += CONFIG_AUTODOC_OPTION
DEFINES += CONFIG_SCU_USE_DDR3
 USE_STATIC_LIBS := 1

ifdef USE_STATIC_LIBS
 ADDITIONAL_OBJECTS += $(IP_CORE_DIR)/etherbone-core/api/.libs/libetherbone.a
else
 LIBS += etherbone
endif



LIBS += stdc++

#DEBUG = 1

CALL_ARGS = scuxl0035 -v


REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================
