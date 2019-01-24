MIAN_MODULE := pmtest.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

SOURCE += $(SCU_DIR)/daq.c
SOURCE += $(SCU_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/dow_crc.c
SOURCE += $(WR_DIR)/dev/w1-temp.c

DEFINES += CONFIG_DAQ_PEDANTIC_CHECK
DEFINES += CONFIG_SCU_BUS_PEDANTIC_CHECK
DEFINES += CONFIG_DAQ_DEBUG

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================


