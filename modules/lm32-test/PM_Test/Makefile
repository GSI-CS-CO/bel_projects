MIAN_MODULE := pmtest.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

SOURCE += $(DAQ_LM32_DIR)/daq.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/dow_crc.c
SOURCE += $(WR_DIR)/dev/w1-temp.c

DEFINES += CONFIG_DAQ_SINGLE_APP
DEFINES += CONFIG_DAQ_PEDANTIC_CHECK
DEFINES += CONFIG_SCU_BUS_PEDANTIC_CHECK
DEFINES += CONFIG_DAQ_DEBUG

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================


