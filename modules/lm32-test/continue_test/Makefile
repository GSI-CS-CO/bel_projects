MIAN_MODULE := continue_test.c

SYN_DIR = ../../../

SCU_DIR = $(SYN_DIR)top/gsi_scu

INCLUDE_DIRS = ./
INCLUDE_DIRS += $(SCU_DIR)

SOURCE += $(SCU_DIR)/daq.c
SOURCE += $(SCU_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/dow_crc.c

SOURCE += $(SYN_DIR)/modules/lm32-include/aux.c
SOURCE += $(SYN_DIR)/modules/lm32-include/irq.c
SOURCE += $(SYN_DIR)/ip_cores/wrpc-sw/dev/w1-temp.c

DEFINES += CONFIG_DAQ_PEDANTIC_CHECK
DEFINES += CONFIG_SCU_BUS_PEDANTIC_CHECK
DEFINES += CONFIG_DAQ_DEBUG

URL = tcp/scuxl0035

include $(SYN_DIR)/modules/lm32_makefiles/makefile.scu
#=================================== EOF ======================================

