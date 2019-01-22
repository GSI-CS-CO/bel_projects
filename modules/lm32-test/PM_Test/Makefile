MIAN_MODULE := pmtest.c

SYN_DIR = ../../../

SOURCE += $(SCU_DIR)/daq.c
SOURCE += $(SCU_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/dow_crc.c
SOURCE += $(WR_DIR)/dev/w1-temp.c

DEFINES += CONFIG_DAQ_PEDANTIC_CHECK
DEFINES += CONFIG_SCU_BUS_PEDANTIC_CHECK
DEFINES += CONFIG_DAQ_DEBUG

TARGET_URL_FILE = $(HOME)/scu.url

include $(SYN_DIR)/modules/lm32_makefiles/makefile.scu
#=================================== EOF ======================================


