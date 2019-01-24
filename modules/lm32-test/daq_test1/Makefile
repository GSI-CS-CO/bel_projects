
MIAN_MODULE := daq_test1.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

SOURCE += $(SCU_DIR)/daq.c
SOURCE += $(SCU_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/dow_crc.c

SOURCE += $(REPOSITORY_DIR)/ip_cores/wrpc-sw/dev/w1-temp.c

DEFINES += CONFIG_DAQ_DEBUG 

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
