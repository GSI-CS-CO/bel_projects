
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

MIAN_MODULE := daq_app_test.cpp

SOURCE += $(SCU_LIB_SRC_DIR)/scu_ddr3.c
SOURCE += $(DAQ_DIR)/daq_ramBuffer.c
SOURCE += $(SCU_LIB_SRC_DIR)/eb_object_transfer.c
SOURCE += $(DAQ_DIR)/daq_interface.cpp
SOURCE += $(DAQ_DIR)/daq_administration.cpp
# SOURCE += $(DAQ_DIR)/scu_eb_interface.cpp

DEFINES += CONFIG_SCU_USE_DDR3
DEFINES += DEBUGLEVEL=1
DEFINES += CONFIG_DAQ_TEST
# DEFINES += NDEBUG
DEFINES += CONFIG_DAQ_DEBUG

DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS

CALL_ARGS         = "tcp/$(SCU_URL)"
REMOTE_CALL_ARGS  = "dev/wbm0"

LIBS += stdc++
LIBS += etherbone
# LIBS += boost_thread
DEBUG = 1


include $(REPOSITORY_DIR)/makefiles/makefile.scun