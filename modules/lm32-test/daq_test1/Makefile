
MIAN_MODULE := daq_test1.c

SYN_DIR = ../../../

SOURCE += $(SCU_DIR)/daq.c
SOURCE += $(SCU_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/dow_crc.c

SOURCE += $(SYN_DIR)/ip_cores/wrpc-sw/dev/w1-temp.c

DEFINES += CONFIG_DAQ_DEBUG 

include $(SYN_DIR)/modules/lm32_makefiles/makefile.scu
#=================================== EOF ======================================
