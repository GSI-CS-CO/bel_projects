#!/bin/bash

# Check my timing RTE

# Usage: ./check-yocto-rte.sh

# target RTE (bash construct for overriding variables)
TARGET_RTE=${TARGET_RTE:-"fbas-yocto"}  # override value from Makefile or CLI

# RTE location
NFSBASE_PATH=${NFSBASE_PATH:-"/common/export"}
NFSINIT_PATH="$NFSBASE_PATH/nfsinit"
TIMING_RTE_PATH="$NFSBASE_PATH/timing-rte"
TARGET_RTE_PATH="$TIMING_RTE_PATH/$TARGET_RTE"
YOCTO_RTE_LOADER="$NFSINIT_PATH/global/timing-rte-${TARGET_RTE}-loader"
ARCH=${ARCH:-"x86_64"}

# check the RTE directory
check_rte_location() {
	if [ -d $TARGET_RTE_PATH ]; then
		echo "PASS: target RTE directory is available: $TARGET_RTE_PATH"
	else
		echo "FAIL: target RTE directory is not found: $TARGET_RTE_PATH"
	fi
}

check_fbas_stuff() {
	# check the availability of the loader script
	if [ -f $YOCTO_RTE_LOADER ]; then
		echo "PASS: RTE loader script for NFSinit symlink is available: $YOCTO_RTE_LOADER"
	else
		echo "FAIL: RTE loader script for NFSinit symlink is not found: $YOCTO_RTE_LOADER"
	fi

	# check the presence of the FBAS stuff
	ls $TARGET_RTE_PATH/firmware/*.bin
	if [ $? -eq 0 ]; then
		echo "PASS: FBAS LM32 firmware is available: $TARGET_RTE_PATH/firmware/*.bin"
	else
		echo "FAIL: FBAS LM32 firmware is not found: $TARGET_RTE_PATH/firmware/*.bin"
	fi

	ls $TARGET_RTE_PATH/$ARCH/bin/*.sh
	if [ $? -eq 0 ]; then
		echo "PASS: FBAS test scripts are available: $TARGET_RTE_PATH/$ARCH/bin/*.sh"
	else
		echo "FAIL: FBAS test scripts are not found: $TARGET_RTE_PATH/$ARCH/bin/*.sh"
	fi

	ls $TARGET_RTE_PATH/test/*.sched
	if [ $? -eq 0 ]; then
		echo "PASS: FBAS test schedules are available: $TARGET_RTE_PATH/test/*.sched"
	else
		echo "FAIL: FBAS test schedules are not found: $TARGET_RTE_PATH/test/*.sched"
	fi

}

check_rte_location
check_fbas_stuff
