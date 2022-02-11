#!/bin/bash
###############################################################################
##                                                                           ##
##       Published the header files and libraries of SCU DAQ and FG          ##
##                                                                           ##
##---------------------------------------------------------------------------##
## File:   public.sh                                                         ##
## Author: Ulrich Becker                                                     ##
## (c)     GSI Helmholtz Centre for Heavy Ion Research GmbH                  ##
## Date:   14.10.2020                                                        ##
###############################################################################
#VERSION_DIR="4.3"
#VERSION_DIR="4.3_develop"
VERSION_DIR="4.3_non_DDR34MIL"

SOURCE_BASE_DIR="/common/home/bel/ubecker/lnx/src/gsi/readable/bel_projects/"
DESTINATION_BASE_DIR="/common/usr/cscofe/opt/daq-fg/${VERSION_DIR}/"
HEADER_DIR="${DESTINATION_BASE_DIR}include/"
LIB_DIR="${DESTINATION_BASE_DIR}/lib/"
LM32_BIN_DIR="${DESTINATION_BASE_DIR}lm32_firmware"
EXAMPLE_DIR="${DESTINATION_BASE_DIR}example"

mkdir -p $HEADER_DIR
mkdir -p $LIB_DIR
mkdir -p $LM32_BIN_DIR
mkdir -p $EXAMPLE_DIR

LIB_FILE=${SOURCE_BASE_DIR}top/gsi_daq/linux/feedback/deploy_x86_64/result/libscu_fg_feedback.a
LM32_FW=${SOURCE_BASE_DIR}syn/gsi_scu/control2/scu_control.bin
EXAMPLE_FILE=${SOURCE_BASE_DIR}/top/gsi_daq/example/feedback/feedback-example.cpp

COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/linux/scu_fg_feedback.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/scu_control_config.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/daq_fg_allocator.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/linux/daq_calculations.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/linux/sdaq/daq_administration.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/linux/sdaq/daq_interface.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/linux/watchdog_poll.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/linux/daq_base_interface.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/linux/daq_exception.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/linux/scu_env.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/linux/daq_eb_ram_buffer.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/linux/daq_access.hpp"
#COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/feSupport/scu/etherbone/EtherboneConnection.hpp"
#COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}ip_cores/etherbone-core/api/etherbone.h"
#COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/feSupport/scu/etherbone/Constants.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}modules/lm32-include/scu_assert.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/lm32/daq_ramBuffer.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/daq_ring_admin.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}modules/lm32-include/circular_index.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}modules/lm32-include/helper_macros.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}modules/lm32-include/scu_ddr3.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/daq_descriptor.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}modules/lm32-include/scu_bus_defines.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_scu/scu_function_generator.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/tools/daqt_messages.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}modules/lm32-include/eb_console_helper.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/daq_command_interface.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_scu/generated/shared_mmap.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/linux/mdaq/mdaq_administration.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/linux/mdaq/mdaq_interface.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/linux/scu_fg_list.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_daq/linux/scu_lm32_mailbox.hpp"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_scu/scu_shared_mem.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}modules/lm32-include/scu_mailbox.h"
COPY_LIST="${COPY_LIST} ${SOURCE_BASE_DIR}top/gsi_scu/scu_circular_buffer.h"

[ -f "$LIB_FILE" ] || echo "Library file: \"$LIB_FILE\" not found!" 1>&2
cp -u $LIB_FILE $LIB_DIR

[ -f "$LM32_FW" ] || echo "LM32 firmware file: \"$LM32_FW\" not found!" 1>&2
cp -u $LM32_FW $LM32_BIN_DIR

[ -f "$EXAMPLE_FILE" ] || echo "Example file \"$EXAMPLE_FILE\" not found" 1>&2
cp -u $EXAMPLE_FILE $EXAMPLE_DIR

n=0
for i in $COPY_LIST
do
   [ -f "$i" ] || echo "Header file: \"$i\" not found!" 1>&2
   cp -u $i $HEADER_DIR 2>/dev/null
   [ "$?" == "0" ] && ((n++))
done
echo "$n header files copied."

#=================================== EOF ======================================
