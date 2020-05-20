###############################################################################
##                                                                           ##
##       Makefile for oqeueDemo application of FreeRTOS on SCU-LM32          ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:    FreeRTOS/Demo/SCU_LM32_GCC/up-down_count/Makefile                ##
## (c):     GSI Helmholtz Centre for Heavy Ion Research GmbH                 ##
## Author:  Ulrich Becker                                                    ##
## Date:    03.04.2020                                                       ##
###############################################################################
REPOSITORY_DIR = $(shell git rev-parse --show-toplevel)

MIAN_MODULE := queueDemo.c
SOURCE += $(SCU_LIB_SRC_DIR)/scu_std_init.c

# The following flag is essential so that the build system adds the
# FreeRTOS sources to this project.
 USE_RTOS = 1

# Select one of the heap modules 1 - 5.
# If this value 0 so the static memory allocation will used. That means
# the user have to allocate memory explicit. In this case the macro
# configSUPPORT_DYNAMIC_ALLOCATION will defined with 0 in portmacro.h.
# The default is 1.
RTOS_USING_HEAP = 0

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

include $(REPOSITORY_DIR)/makefiles/makefile.scu
#=================================== EOF ======================================
