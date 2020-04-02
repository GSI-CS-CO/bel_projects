###############################################################################
##                                                                           ##
##         Makefile for test application of FreeRTOS on SCU-LM32             ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    FreeRTOS/Demo/SCU_LM32_GCC/up-down_count/Makefile                ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    24.03.2020                                                       ##
###############################################################################
REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

MIAN_MODULE := up-down_count.c
SOURCE      += $(SCU_LIB_SRC_DIR)/shared_memory_helper.c

# The following flag is essential so that the build system adds the
# FreeRTOS sources to this project.
 USE_RTOS = 1

RTOS_USING_HEAP = 1

# TOOLCHAIN_DIR = $(INCLUDED_TOOLCHAIN_DIR)

CODE_OPTIMIZATION = s


# When the following compiler switch is defined, then the assert-macros
# within the FreeRTOS kernel will activated.
# Meaningful in debug mode.
 DEFINES += CONFIG_RTOS_PEDANTIC_CHECK
 DEFINES += CONFIG_INTERRUPT_PEDANTIC_CHECK

DEFINES += MAX_LM32_INTERRUPTS=2

# FreeRTOS will handle the stack defined in FreeRTOSConfig.h.
# Therefore we can overwrite the default size of the base stack size (10k)
# into a smaller size.
STACK_SIZE := 256

SHARED_SIZE := 20

DOX_REFERENCES_RELATION = YES
include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
