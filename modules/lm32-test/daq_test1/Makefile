
MIAN_MODULE := daq_test1.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

SOURCE += $(DAQ_LM32_DIR)/daq.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/dow_crc.c

SOURCE += $(REPOSITORY_DIR)/ip_cores/wrpc-sw/dev/w1-temp.c

DEFINES += CONFIG_DAQ_DEBUG 
DEFINES += CONFIG_SCU_BUS_PEDANTIC_CHECK
DEFINES += CONFIG_DAQ_SINGLE_APP

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
