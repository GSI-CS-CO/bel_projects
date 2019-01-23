MIAN_MODULE := scubussniff.c

SYN_DIR = ../../../

SOURCE += $(SCU_LIB_SRC_DIR)/lm32_hexdump.c

SOURCE += $(SCU_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/dow_crc.c
SOURCE += $(SYN_DIR)/ip_cores/wrpc-sw/dev/w1-temp.c

DEFINES += CONFIG_SCU_BUS_PEDANTIC_CHECK

include $(SYN_DIR)/modules/lm32_makefiles/makefile.scu
#=================================== EOF ======================================

