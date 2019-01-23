MIAN_MODULE := continue_test.c

SYN_DIR = ../../..

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

include $(SYN_DIR)/modules/lm32_makefiles/makefile.scu
#=================================== EOF ======================================

