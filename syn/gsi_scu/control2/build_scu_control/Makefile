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
DEFINES += CONFIG_INTERRUPT_PEDANTIC_CHECK
DEFINES += MICO32_FULL_CONTEXT_SAVE_RESTORE

 DEFINES += _CONFIG_NO_DISPATCHER
 DEFINES += _CONFIG_ADDAC_FG_IN_INTERRUPT

 SCU_MIL := 1
MIL_GAP := 1
# MIL_DAQ_USE_DDR3 := 1
 SCU_DAQ := 1

BASE_VERSION = 3

 DEFINES += _CONFIG_NEW

DEFINES += FG_VERSION=$(BASE_VERSION)
VERSION_STR = $(BASE_VERSION)".1.0"

DEFINES += MAX_LM32_INTERRUPTS=1

#  USE_HISTORY=1
MIAN_MODULE    = $(SCU_DIR)/scu_main.c
#MIAN_MODULE    = $(SCU_DIR)/scu_main_old_task.c



ifdef MIL_DAQ_USE_DDR3
   DEFINES += CONFIG_MIL_DAQ_USE_RAM
   USE_SCU_EXTERN_RAM := 1
endif
ifdef USE_HISTORY
   DEFINES += HISTORY
endif

ifdef MIL_GAP
 DEFINES += CONFIG_READ_MIL_TIME_GAP
endif
# DEFINES += CONFIG_NO_LM32_ASSERT
DEFINES += CONFIG_PRINT_BUFSIZE=128
DEFINES += CONFIG_PRINTF_64BIT
DEFINES += CONFIG_ASSERT
DEFINES += SDBFS_BIG_ENDIAN
DEFINES += CONFIG_WR_NODE

# DEFINES += DEBUG_SAFTLIB
ifdef SCU_MIL
  DEFINES += CONFIG_MIL_FG
  VERSION_STR += "+MIL"
 ifdef MIL_GAP
  VERSION_STR += "+GAP"
 endif
endif
ifdef SCU_DAQ
  DEFINES += CONFIG_SCU_DAQ_INTEGRATION
  DEFINES += CONFIG_DAQ_SW_SEQUENCE
  DEFINES += DAQ_MAX_CHANNELS=4
  VERSION_STR += "+DAQ"
endif
VERSION = $(VERSION_STR)

SOURCE += $(SCU_LIB_SRC_DIR)/scu_std_init.c
SOURCE += $(SCU_LIB_SRC_DIR)/lm32Interrupts.c #_old.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_mailbox.c
SOURCE += $(SCU_LIB_SRC_DIR)/LM32assert.c
SOURCE += $(SCU_LIB_SRC_DIR)/stack-check.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_bus.c
SOURCE += $(WR_DIR)/pp_printf/div64.c
# SOURCE += $(SCU_LIB_SRC_DIR)/dbg.c
SOURCE += $(SCU_DIR)/scu_function_generator.c
SOURCE += $(SCU_DIR)/scu_circular_buffer.c
SOURCE += $(SCU_DIR)/scu_fg_macros.c
SOURCE += $(SCU_DIR)/scu_fg_handler.c
SOURCE += $(SCU_DIR)/scu_command_handler.c
SOURCE += $(SCU_DIR)/scu_temperature.c
ifdef SCU_MIL
  SOURCE += $(SCU_LIB_SRC_DIR)/eca_queue_type.c
  SOURCE += $(SCU_DIR)/scu_eca_handler.c
  SOURCE += $(SCU_DIR)/scu_mil.c
  SOURCE += $(SCU_DIR)/scu_mil_fg_handler.c
endif
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
ifdef USE_HISTORY
  SOURCE += $(SCU_DIR)/history.c
endif
ifdef USE_SCU_EXTERN_RAM
  SOURCE += $(DAQ_DIR)/daq_ring_admin.c
  DEFINES += CONFIG_SCU_USE_DDR3
endif

SOURCE += $(SCU_DIR)/dow_crc.c
SOURCE += $(WR_DIR)/dev/w1-temp.c

# For DOXYGEN only, header files which doesn't have a appropriate C-file.
DOX_INPUT += $(SCU_DIR)/scu_shared_mem.h
DOX_INPUT += $(SCU_LIB_SRC_DIR)/scu_bus_defines.h
DOX_INPUT += $(SCU_LIB_SRC_DIR)/scu_msi.h

CODE_OPTIMIZATION = s
# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

# RAM_SIZE    = 147456

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
 STACK_SIZE  =  4096


# SCU_URL = scuxl0107
# SCU_URL = scuxl0035

DOX_EXTRACT_STATIC         = "YES"
# DOX_EXTRACT_ALL            = "YES"
DOX_MACRO_EXPANSION        = "YES"
DOX_EXPAND_ONLY_PREDEF     = "YES"
DOX_EXPAND_AS_DEFINED      = "FSM_DECLARE_STATE __DAQ_SHARED_IO_T"
DOX_DOTFILE_DIRS           += $(SCU_DIR)


# Including common makefile for arbitrary LM32 software for SCU
include $(REPOSITORY_DIR)/makefiles/makefile.scu

TESTFILE       ?= sinus-test.txt
TARGETTEST_DIR ?= /
SLOT           ?= 4
FG_NUM         ?= 0

FG_UT = fg-$(SLOT)-$(FG_NUM)

.PHONY: key
key:
	ssh-copy-id -i $(HOME)/.ssh/id_rsa.pub root@$(SCU_URL)

.PHONY: cptest
cptest:
	scp $(SCU_DIR)/$(TESTFILE) root@$(SCU_URL):$(TARGETTEST_DIR)

.PHONY: runfg
runfg:
	ssh root@$(SCU_URL) "(saft-fg-ctl  -r -f $(FG_UT) -g  <$(TARGETTEST_DIR)$(TESTFILE))"

.PHONY: stop
stop:
	ssh root@$(SCU_URL) "killall saft-fg-ctl"

.PHONY: scan
scan:
	ssh root@$(SCU_URL) "saft-fg-ctl -si"

#=================================== EOF ======================================
