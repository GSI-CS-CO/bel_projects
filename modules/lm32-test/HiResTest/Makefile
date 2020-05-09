MIAN_MODULE := hirestest.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

SOURCE += $(DAQ_LM32_DIR)/daq.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/dow_crc.c

SOURCE += $(SCU_LIB_SRC_DIR)/aux.c
SOURCE += $(SCU_LIB_SRC_DIR)/irq.c
SOURCE += $(WR_DIR)/dev/w1-temp.c

DEFINES += CONFIG_DAQ_SINGLE_APP
DEFINES += CONFIG_DAQ_PEDANTIC_CHECK
DEFINES += CONFIG_SCU_BUS_PEDANTIC_CHECK
DEFINES += CONFIG_DAQ_DEBUG
CODE_OPTIMIZATION=0

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================

