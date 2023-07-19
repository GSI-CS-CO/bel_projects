#!/bin/bash

# Deploy the FBAS artifacts for the Yocto based timing RTE

# Usage: ./deploy-yocto-rte.sh

# sources
ABS_PATH=$(readlink -f $0)
FBAS_PATH=${ABS_PATH%/rte*}
FW_PATH="$FBAS_PATH/fw"
TEST_PATH="$FBAS_PATH/test"
ASL_PATH="$FBAS_PATH/rte/asl"

# target RTE (bash construct for overriding variable)
: ${TARGET_RTE:="fbas-yocto"} # override value from Makefile or CLI

# RTE location
: ${NFSBASE_PATH:="/common/export"}
NFSINIT_PATH="$NFSBASE_PATH/nfsinit"
TIMING_RTE_PATH="$NFSBASE_PATH/timing-rte"
TARGET_RTE_PATH="$TIMING_RTE_PATH/$TARGET_RTE"
: ${ARCH:="x86_64"}

# RTE loader script
YOCTO_RTE_LOADER="timing-rte-${TARGET_RTE}-loader"

deploy_fbas_artifacts() {
	# TR LM32 firmware
	echo "deploy $FW_PATH/*.bin to $TARGET_RTE_PATH/firmware"
	mkdir -p $TARGET_RTE_PATH/firmware
	cp -u -p $FW_PATH/*.bin $TARGET_RTE_PATH/firmware

	# test scripts
	echo "deploy $TEST_PATH/scu/*.sh to $TARGET_RTE_PATH/$ARCH/bin"
	mkdir -p $TARGET_RTE_PATH/$ARCH/bin
	cp -u -p $TEST_PATH/scu/*.sh $TARGET_RTE_PATH/$ARCH/bin

	# test artifacts
	echo "deploy $TEST_PATH/scu/*.sched to $TARGET_RTE_PATH/test"
	mkdir -p $TARGET_RTE_PATH/test
	cp -u -p $TEST_PATH/scu/*.sched $TARGET_RTE_PATH/test

}

deploy_rte_scripts() {
	echo "deploy $ASL_PATH/loader.sh to $TARGET_RTE_PATH/"
	cp -u -p $ASL_PATH/loader.sh $TARGET_RTE_PATH/

	echo "deploy $ASL_PATH/$YOCTO_RTE_LOADER to $NFSINIT_PATH/global/"
	cp -u -p $ASL_PATH/$YOCTO_RTE_LOADER $NFSINIT_PATH/global/
}

check_locations() {
	if [ -d $TARGET_RTE_PATH ]; then
		echo "TARGET_RTE_PATH: $TARGET_RTE_PATH"
	else
		echo "- TARGET_RTE_PATH: $TARGET_RTE_PATH is not found"
	fi

	if [ -d $FW_PATH ]; then
		echo "FW_PATH: $FW_PATH"
	else
		echo "- FW_PATH: $FW_PATH is not found"
	fi

	if [ -d $TEST_PATH ]; then
		echo "TEST_PATH: $TEST_PATH"
	else
		echo "- TEST_PATH: $TEST_PATH is not found"
	fi

	if [ -d $ASL_PATH ]; then
		echo "ASL_PATH: $ASL_PATH"
	else
		echo "- ASL_PATH: $ASL_PATH is not found"
	fi
}

deploy_fbas_artifacts
deploy_rte_scripts
