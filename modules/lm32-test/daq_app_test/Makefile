
REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)

USE_FE_LIB_ETHERBONE=1

MIAN_MODULE := daq_app_test.cpp


SOURCE += $(DAQ_DIR)/daq_ring_admin.c
SOURCE += $(DAQ_DIR)/daq_interface.cpp
SOURCE += $(DAQ_DIR)/daq_administration.cpp
# SOURCE += $(DAQ_DIR)/scu_eb_interface.cpp

ifdef USE_FE_LIB_ETHERBONE
  EB_SRC= $(DAQ_DIR)/feSupport/scu/etherbone
  SOURCE += $(EB_SRC)/EtherboneConnection.cpp
  SOURCE += $(EB_SRC)/BusException.cpp
  SOURCE += $(DAQ_DIR)/daq_eb_ram_buffer.cpp
else
  SOURCE += $(SCU_LIB_SRC_DIR)/scu_ddr3.c
  SOURCE += $(SCU_LIB_SRC_DIR)/eb_object_transfer.c
  SOURCE += $(DAQ_DIR)/daq_ramBuffer.c
endif

DEFINES += _BSD_SOURCE
DEFINES += _POSIX_C_SOURCE
DEFINES += CONFIG_SCU_USE_DDR3
DEFINES += DEBUGLEVEL=1
DEFINES += CONFIG_DAQ_TEST
# DEFINES += NDEBUG
DEFINES += CONFIG_DAQ_DEBUG

DEFINES += CONFIG_DDR3_NO_BURST_FUNCTIONS

CALL_ARGS         = "tcp/$(SCU_URL)"
REMOTE_CALL_ARGS  = "dev/wbm0"

ifdef USE_FE_LIB_ETHERBONE
  DEFINES += __ETHERBONE_CONNECTION_MUTEX="EtherBoneConnectionMutexDaq"
  INCLUDE_DIRS += $(EB_SRC)
  LIBS += boost_system
  LIBS += pthread
else
  DEFINES += CONFIG_NO_FE_ETHERBONE_CONNECTION
endif


LIBS += stdc++
LIBS += etherbone
# LIBS += boost_thread
DEBUG = 1


include $(REPOSITORY_DIR)/makefiles/makefile.scun