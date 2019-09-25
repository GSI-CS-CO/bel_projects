###############################################################################
##                                                                           ##
##              Makefile LM32 FG software scu_control.bin                    ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    Makefile                                                         ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    09.01.2019                                                       ##
###############################################################################

REPOSITORY_DIR := $(shell git rev-parse --show-toplevel)
TARGET_DIR     := ..
DEPLOY_DIR     := .
TARGET         := scu_control

# ORIGIN := 1

ifdef ORIGIN
  USE_HISTORY=1
  MIAN_MODULE    = $(SCU_DIR)/main.c
# MIAN_MODULE    = $(SCU_DIR)/main_origin.c
else
  MIAN_MODULE    = $(SCU_DIR)/scu_main.c
  # SCU_DAQ=1
  # MIL_DAQ_USE_DDR3=1
endif

ifdef MIL_DAQ_USE_DDR3
   DEFINES += CONFIG_MIL_DAQ_USE_RAM
   USE_SCU_EXTERN_RAM := 1
endif
ifdef USE_HISTORY
   DEFINES += HISTORY
endif

DEFINES += CONFIG_NO_LM32_ASSERT
DEFINES += CONFIG_PRINT_BUFSIZE=128
DEFINES += CONFIG_PRINTF_64BIT
DEFINES += CONFIG_ASSERT
DEFINES += SDBFS_BIG_ENDIAN
DEFINES += CONFIG_WR_NODE

ifdef SCU_DAQ
  DEFINES += CONFIG_SCU_DAQ_INTEGRATION
  DEFINES += CONFIG_DAQ_SW_SEQUENCE
  DEFINES += DAQ_MAX_CHANNELS=4
else
  # New functions will not compiled when the following switch is set.
  DEFINES += CONFIG_OLD_SCU_SW
endif


SOURCE += $(SCU_LIB_SRC_DIR)/aux.c
SOURCE += $(SCU_LIB_SRC_DIR)/irq.c
SOURCE += $(SCU_LIB_SRC_DIR)/display.c
SOURCE += $(SCU_LIB_SRC_DIR)/LM32assert.c
SOURCE += $(SCU_LIB_SRC_DIR)/stack-check.c

# SOURCE += $(SCU_LIB_SRC_DIR)/wr_time.c
SOURCE += $(WR_DIR)/pp_printf/div64.c
ifdef ORIGIN
#  SOURCE += $(SCU_LIB_SRC_DIR)/dbg.c
endif
SOURCE += $(SCU_DIR)/scu_bus.c
SOURCE += $(SCU_DIR)/fg.c
SOURCE += $(SCU_DIR)/cb.c
SOURCE += $(SCU_DIR)/dow_crc.c
ifdef USE_HISTORY
   SOURCE += $(SCU_DIR)/history.c
endif
SOURCE += $(SCU_DIR)/scu_mil.c
SOURCE += $(WR_DIR)/dev/w1-temp.c

ifdef SCU_DAQ
  SOURCE += $(SCU_LIB_SRC_DIR)/scu_ddr3.c
  SOURCE += $(DAQ_LM32_DIR)/daq.c
  SOURCE += $(DAQ_LM32_DIR)/daq_command_interface_uc.c
  SOURCE += $(DAQ_LM32_DIR)/daq_ramBuffer.c
  SOURCE += $(DAQ_LM32_DIR)/daq_main.c
  USE_SCU_EXTERN_RAM := 1
endif

ifdef USE_SCU_EXTERN_RAM
  SOURCE += $(DAQ_DIR)/daq_ring_admin.c
  DEFINES += CONFIG_SCU_USE_DDR3
endif

# To be 100% backward compatible we can use the old toolchain.
# If the variable TOOLCHAIN_DIR commented out, then the first toolchain found
# in the environment variable $PATH will used if present,
# else the included toolchain of this repository will used never the less.
ifdef ORIGIN
  TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)
  # Absolutely necessary for the correct order of the variables
  # within the shared memory!
   CFLAGS += -fno-toplevel-reorder
  CODE_OPTIMIZATION = s
else
  CODE_OPTIMIZATION = s
  # TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)
endif

RAM_SIZE    = 147456

ifdef SCU_DAQ
   SHARED_SIZE = 81968
   ifdef USE_HISTORY
      STACK_SIZE  =  4096
      # STACK_SIZE = 5104
   endif
else
   SHARED_SIZE = 81920
endif

# NO_LTO=1


 SCU_URL = scuxl0107

# Including common makefile for arbitrary LM32 software for SCU
include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
