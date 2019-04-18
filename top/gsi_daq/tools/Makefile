
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

MIAN_MODULE := daqt.cpp

SOURCE += parse_opts.cpp
SOURCE += daqt_command_line.cpp
SOURCE += daqt_scan.cpp

SOURCE += $(SCU_LIB_SRC_DIR)/scu_ddr3.c
SOURCE += $(DAQ_DIR)/daq_ramBuffer.c
SOURCE += $(SCU_LIB_SRC_DIR)/eb_object_transfer.c
SOURCE += $(DAQ_DIR)/daq_interface.cpp
SOURCE += $(DAQ_DIR)/daq_administration.cpp
SOURCE += $(DAQ_DIR)/scu_eb_interface.cpp

DEFINES += CONFIG_SCU_USE_DDR3
#DEFINES += DEBUGLEVEL=1
DEFINES += CONFIG_DAQ_TEST
# DEFINES += NDEBUG
DEFINES += CONFIG_DAQ_DEBU
#CALL_ARGS         ?= tcp/$(SCU_URL) -h
#CALL_ARGS         ?= tcp/$(SCU_URL) -s

CALL_ARGS         ?= tcp/$(SCU_URL) --limit 10 3 1 9 4

REMOTE_CALL_ARGS  = "dev/wbm0"

LIBS += stdc++
LIBS += etherbone

DEBUG = 1


include $(REPOSITORY_DIR)/makefiles/makefile.scun