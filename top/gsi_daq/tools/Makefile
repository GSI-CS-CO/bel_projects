
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

MIAN_MODULE := daqt.cpp

SOURCE += parse_opts.cpp
SOURCE += daqt_command_line.cpp
SOURCE += gnuplotstream.cpp
SOURCE += find_process.c

SOURCE += $(SCU_LIB_SRC_DIR)/scu_ddr3.c
SOURCE += $(DAQ_DIR)/daq_ramBuffer.c
SOURCE += $(SCU_LIB_SRC_DIR)/eb_object_transfer.c
SOURCE += $(DAQ_DIR)/daq_interface.cpp
SOURCE += $(DAQ_DIR)/daq_administration.cpp
SOURCE += $(DAQ_DIR)/scu_eb_interface.cpp


DEFINES += _BSD_SOURCE
DEFINES += _POSIX_C_SOURCE
DEFINES += CONFIG_SCU_USE_DDR3
#DEFINES += DEBUGLEVEL=1
DEFINES += CONFIG_DAQ_TEST
# DEFINES += NDEBUG
DEFINES += CONFIG_DAQ_DEBU

HOST ?= tcp/$(SCU_URL)

#CALL_ARGS         ?=  -h
#CALL_ARGS         ?= $(HOST) --continue 3 1
#CALL_ARGS         ?= $(HOST) -vs
#CALL_ARGS         ?= $(HOST) 3 -l5 1 3 2 9 3 9 4 -l7 -e
#CALL_ARGS         ?= $(HOST)  -eC 3 1 3 2 3 3 3 4 9 1 9 2 9 3 9 4
#CALL_ARGS         ?= $(HOST) -tEC  3 1
CALL_ARGS         ?= $(HOST)  -C 3 1 3 2 -z 9 2

#CALL_ARGS         ?= $(HOST) -Hrte 3 1 3 2

REMOTE_CALL_ARGS  = "dev/wbm0"

LIBS += stdc++
LIBS += etherbone

DEBUG = 1


include $(REPOSITORY_DIR)/makefiles/makefile.scun