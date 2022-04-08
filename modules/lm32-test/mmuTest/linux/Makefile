REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

DEBUG = 1
#DEFINES += CONFIG_DEBUG_MMU

MIAN_MODULE := mmutest.cpp

EB_SRC= $(DAQ_DIR)/feSupport/scu/etherbone


SOURCE += $(SCU_LIB_SRC_DIR)/scu_mmu_fe.cpp
SOURCE += $(SCU_LIB_SRC_DIR)/scu_mmu.c
SOURCE += $(SCU_LIB_SRC_DIR)/lm32_hexdump.c

SOURCE += $(EB_SRC)/EtherboneConnection.cpp
SOURCE += $(EB_SRC)/BusException.cpp

INCLUDE_DIRS += $(EB_SRC)

DEFINES += CONFIG_SCU_USE_DDR3
 USE_STATIC_LIBS := 1

ifdef USE_STATIC_LIBS
 ADDITIONAL_OBJECTS += $(IP_CORE_DIR)/etherbone-core/api/.libs/libetherbone.a
else
 LIBS += etherbone
endif

LIBS += stdc++

CALL_ARGS = scuxl0035

include $(REPOSITORY_DIR)/makefiles/makefile.scun
#=================================== EOF ======================================