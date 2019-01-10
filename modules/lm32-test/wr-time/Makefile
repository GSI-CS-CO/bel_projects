
MIAN_MODULE := wr-test.c

SYN_DIR = ../../../

SCU_DIR = $(SYN_DIR)top/gsi_scu

DEFINES += CONFIG_WR_NODE

INCLUDE_DIRS = ./
INCLUDE_DIRS += $(SCU_DIR)

TARGET_URL_FILE = ~/target.url

# SHARED_SIZE     = 80

SOURCE += $(SYN_DIR)/modules/lm32-include/wr_time.c

SOURCE += $(SYN_DIR)/modules/lm32-include/aux.c
SOURCE += $(SYN_DIR)/modules/lm32-include/irq.c

include $(SYN_DIR)/modules/lm32_makefiles/makefile.scu
#=================================== EOF ======================================
