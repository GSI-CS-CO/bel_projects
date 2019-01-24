MIAN_MODULE := continue_test.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

SOURCE += $(SCU_DIR)/daq.c
SOURCE += $(SCU_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/dow_crc.c

SOURCE += $(SCU_LIB_SRC_DIR)/aux.c
SOURCE += $(SCU_LIB_SRC_DIR)/irq.c
SOURCE += $(WR_DIR)/dev/w1-temp.c

DEFINES += CONFIG_DAQ_PEDANTIC_CHECK
DEFINES += CONFIG_SCU_BUS_PEDANTIC_CHECK
DEFINES += CONFIG_DAQ_DEBUG
#DEFINES += CONFIG_NO_LM32_ASSERT

CODE_OPTIMIZATION = s

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================

