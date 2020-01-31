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
DEFINES += CONFIG_FG_PEDANTIC_CHECK
DEFINES += MICO32_FULL_CONTEXT_SAVE_RESTORE
# ORIGIN := 1


ifdef ORIGIN
  USE_HISTORY=1
  MIAN_MODULE    = $(SCU_DIR)/main.c
else
  MIAN_MODULE    = $(SCU_DIR)/scu_main.c
  #MIAN_MODULE    = $(SCU_DIR)/scu_main_old_task.c
  SCU_DAQ=1
  # MIL_DAQ_USE_DDR3=1
endif

ifdef MIL_DAQ_USE_DDR3
   DEFINES += CONFIG_MIL_DAQ_USE_RAM
   USE_SCU_EXTERN_RAM := 1
endif
ifdef USE_HISTORY
   DEFINES += HISTORY
endif

 DEFINES += CONFIG_READ_MIL_TIME_GAP

DEFINES += CONFIG_NO_LM32_ASSERT
DEFINES += CONFIG_PRINT_BUFSIZE=128
DEFINES += CONFIG_PRINTF_64BIT
DEFINES += CONFIG_ASSERT
DEFINES += SDBFS_BIG_ENDIAN
DEFINES += CONFIG_WR_NODE

# DEFINES += DEBUG_SAFTLIB

ifdef SCU_DAQ
  DEFINES += CONFIG_SCU_DAQ_INTEGRATION
  DEFINES += CONFIG_DAQ_SW_SEQUENCE
  DEFINES += DAQ_MAX_CHANNELS=4
else
  # New functions will not compiled when the following switch is set.
 # DEFINES += CONFIG_OLD_SCU_SW
endif


SOURCE += $(SCU_LIB_SRC_DIR)/aux.c
SOURCE += $(SCU_LIB_SRC_DIR)/irq.c
SOURCE += $(SCU_LIB_SRC_DIR)/display.c
SOURCE += $(SCU_LIB_SRC_DIR)/LM32assert.c
SOURCE += $(SCU_LIB_SRC_DIR)/stack-check.c
SOURCE += $(SCU_LIB_SRC_DIR)/eca_queue_type.c
# SOURCE += $(SCU_LIB_SRC_DIR)/wr_time.c
SOURCE += $(WR_DIR)/pp_printf/div64.c
SOURCE += $(SCU_DIR)/scu_bus.c
ifdef ORIGIN
 SOURCE += $(SCU_LIB_SRC_DIR)/dbg.c
 SOURCE += $(SCU_DIR)/fg.c
 SOURCE += $(SCU_DIR)/cb.c
else
 SOURCE += $(SCU_DIR)/scu_function_generator.c
 SOURCE += $(SCU_DIR)/scu_circular_buffer.c
endif
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
  DOX_INPUT += $(DAQ_DIR)/daq_command_interface.h
  DOX_INPUT += $(DAQ_DIR)/daq_descriptor.h
  USE_SCU_EXTERN_RAM := 1
endif

DOX_INPUT += $(SCU_DIR)/scu_shared_mem.h
DOX_INPUT += $(SCU_LIB_SRC_DIR)/scu_bus_defines.h

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
# SCU_URL = scuxl0035
 DOX_EXTRACT_STATIC         = "YES"
# DOX_EXTRACT_ALL            = "YES"
DOX_MACRO_EXPANSION        = "YES"
DOX_EXPAND_ONLY_PREDEF     = "YES"
DOX_EXPAND_AS_DEFINED      = "FSM_DECLARE_STATE __DAQ_SHARED_IO_T"
DOX_DOTFILE_DIRS           += $(SCU_DIR)

# Including common makefile for arbitrary LM32 software for SCU
include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
