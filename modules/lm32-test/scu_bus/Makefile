MIAN_MODULE := scu_bus_test.c

SYN_DIR = ../../../

SCU_DIR = $(SYN_DIR)top/gsi_scu

INCLUDE_DIRS = ./
INCLUDE_DIRS += $(SCU_DIR)

SOURCE += $(SCU_DIR)/daq.c
SOURCE += $(SCU_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/dow_crc.c

DEFINES += CONFIG_DAQ_PEDANTIC_CHECK
DEFINES += CONFIG_SCU_BUS_PEDANTIC_CHECK
DEFINES += CONFIG_DAQ_DEBUG

URL = tcp/scuxl0035

include $(SYN_DIR)/modules/lm32_makefiles/makefile.scu
#=================================== EOF ======================================

