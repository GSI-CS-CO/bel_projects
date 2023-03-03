#!/bin/bash

# Check my timing RTE

# Usage: ./check-rte.sh CI_CD_REPO="path/to/ci_cd"

# target RTE
TARGET_RTE="fbas"

# RTE location
NFSBASE_PATH="/common/export"
NFSINIT_PATH="$NFSBASE_PATH/nfsinit"
TIMING_RTE_PATH="$NFSBASE_PATH/timing-rte"
TARGET_RTE_PATH="$TIMING_RTE_PATH/$TARGET_RTE"

# RTE builder (in CI/CD)
CI_CD_REPO="$HOME/ci_cd"
CI_CD_TIMING_RTE="$CI_CD_REPO/scripts/deployment/RTE_Timing"
TIMING_RTE_BUILDER="$CI_CD_TIMING_RTE/build-rte.sh"

# settings for the RTE builder
MY_RTE_KERNEL4="no"          # use kernel 3.x

# check the RTE directory
check_rte_location() {
	if [ -d $TARGET_RTE_PATH ]; then
		echo "PASS: target RTE directory is available: $TARGET_RTE_PATH"
	else
		echo "FAIL: target RTE directory is not found: $TARGET_RTE_PATH"
	fi
}

# check the RTE builder
check_rte_builder() {
	if [ -f $TIMING_RTE_BUILDER ]; then
		echo "PASS: RTE builder is available: $TIMING_RTE_BUILDER"
	else
		echo "FAIL: RTE builder is not found: $TIMING_RTE_BUILDER"
	fi
}

# check the RTE builder settings
check_rte_builder_settings() {
	passed=0
	expected=2
	echo "checking the RTE builder settings"
	act_kernel4=$(grep -Eo "^BEL_BUILD_KERNEL4=\".*\"" $TIMING_RTE_BUILDER)
	act_target=$(grep -Eo "^DEPLOY_TARGET=\".*\"" $TIMING_RTE_BUILDER)
	if [[ "${act_kernel4##*=}" == *"$MY_RTE_KERNEL4"* ]]; then
		echo " + match kernel 4+ build: $act_kernel4 (their) vs $MY_RTE_KERNEL4 (our)"
		passed=$(( passed + 1 ))
	else
		echo " - mismatch kernel 4+ build: $act_kernel4 (their) vs $MY_RTE_KERNEL4 (our)"
	fi
	if [[ "${act_target##*=}" == *"$TARGET_RTE_PATH"* ]]; then
		echo " + match target location: $act_target (their) vs $TARGET_RTE_PATH (our)"
		passed=$(( passed + 1 ))
	else
		echo " - mismatch target location: $act_target (their) vs $TARGET_RTE_PATH (our)"
	fi

	if [ $passed -eq $expected ]; then
		echo "PASS: valid settings for RTE builder: $TIMING_RTE_BUILDER"
		echo "      to re-build the target RTE, invoke: $TIMING_RTE_BUILDER"
	else
		echo "FAIL: invalid settings for RTE builder: $TIMING_RTE_BUILDER"
	fi
}

check_nfsinit_scripts() {
	if [ -f $NFSBASE_PATH/nfsinit/global/timing-rte-$TARGET_RTE ]; then
		echo "PASS: RTE script for NFSinit symlink is available: $NFSINIT_PATH/global/timing-rte-$TARGET_RTE"
	else
		echo "FAIL: RTE script for NFSinit symlink is not found: $NFSINIT_PATH/global/timing-rte-$TARGET_RTE"
	fi

	if [ -f $TARGET_RTE_PATH/timing-rte.sh ]; then
		echo "PASS: RTE script for NFSinit is available: $TARGET_RTE_PATH/timing-rte.sh"
	else
		echo "FAIL: RTE script for NFSinit is not found: $TARGET_RTE_PATH/timing-rte.sh"
	fi
}
setup_rte_build() {
	echo "Create the target RTE directory: '$TARGET_RTE_PATH'"
	mkdir -p $TARGET_RTE_PATH

	# instruct to build the RTE => cannot override local variables of the script from CLI
	#echo "Invoke a command below to build the target RTE: "
	#echo "cd $CI_CD_TIMING_RTE; BEL_BUILD_KERNEL4="$MY_RTE_KERNEL4"; DEPLOY_TARGET="$TARGET_RTE_PATH"; ./${TIMING_RTE_BUILDER##*/}"

	# or apply relevant settings to the RTE builder
	sed -i "s|^BEL_BUILD_KERNEL4=\".*\"|BEL_BUILD_KERNEL4=\"$MY_RTE_KERNEL4\"|" $TIMING_RTE_BUILDER
	sed -i "s|^DEPLOY_TARGET=.*|DEPLOY_TARGET=\"$TARGET_RTE_PATH\"|" $TIMING_RTE_BUILDER
}

check_rte_location
check_rte_builder
check_rte_builder_settings
check_nfsinit_scripts
