MIAN_MODULE := scubussniff.c

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

SOURCE += $(SCU_LIB_SRC_DIR)/lm32_hexdump.c

SOURCE += $(SCU_LIB_SRC_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/dow_crc.c
SOURCE += $(WR_DIR)/dev/w1-temp.c

DEFINES += CONFIG_SCU_BUS_PEDANTIC_CHECK

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================

