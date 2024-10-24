# Timing RTE deployment
#
# Usage: make deploy

# sources to be deployed
FW     := fw
TEST   := test
RTE    := rte
ASL    := rte/asl

# timing RTE directory for NFSinit
NFSBASE    = /common/export
TIMING_RTE = timing-rte
ARCH       = x86_64
TARGET_RTE ?= fbas
YOCTO_TARGET_RTE ?=$(TARGET_RTE)-yocto

.SILENT .PHONY: check-sl7-rte deploy-sl7-rte check-yocto-rte deploy-yocto-rte firmware

# deploy FBAS artifacts to the RTE directory (for sl7 ramdisk)
deploy-sl7-rte:
	# TR LM32 firmware
	echo deploy '$(FW)/*.bin' to '$(NFSBASE)/$(TIMING_RTE)/$(TARGET_RTE)/firmware'
	mkdir -p $(NFSBASE)/$(TIMING_RTE)/$(TARGET_RTE)/firmware
	cp -u -p $(FW)/*.bin $(NFSBASE)/$(TIMING_RTE)/$(TARGET_RTE)/firmware

	# test scripts
	echo deploy '$(TEST)/scu/*.sh to $(NFSBASE)/$(TIMING_RTE)/$(TARGET_RTE)/$(ARCH)/bin'
	cp -u -p $(TEST)/scu/*.sh $(NFSBASE)/$(TIMING_RTE)/$(TARGET_RTE)/$(ARCH)/bin

	# test artifacts (ie., schedule for local injection)
	echo deploy '$(TEST)/scu/*.sched to $(NFSBASE)/$(TIMING_RTE)/$(TARGET_RTE)/test'
	mkdir -p $(NFSBASE)/$(TIMING_RTE)/$(TARGET_RTE)/test
	cp -u -p $(TEST)/scu/*.sched $(NFSBASE)/$(TIMING_RTE)/$(TARGET_RTE)/test

	# RTE script for NFSinit symlink
	echo deploy '$(ASL)/timing-rte-$(TARGET_RTE)' to '$(NFSBASE)/nfsinit/global'
	cp -u -p $(ASL)/timing-rte-$(TARGET_RTE) $(NFSBASE)/nfsinit/global

	# NFSinit script
	echo deploy '$(ASL)/timing-rte.sh' to '$(NFSBASE)/$(TIMING_RTE)/$(TARGET_RTE)'
	cp -u -p $(ASL)/timing-rte.sh $(NFSBASE)/$(TIMING_RTE)/$(TARGET_RTE)

# deploy the FBAS artifacts to the target RTE directory (for Yocto based ramdisk)
deploy-yocto-rte:
	TARGET_RTE=$(YOCTO_TARGET_RTE) NFSBASE_PATH=$(NFSBASE) ARCH=$(ARCH) ./rte/deploy-yocto-rte.sh

# check if target RTE is built
check-sl7-rte:
	TARGET_RTE=$(TARGET_RTE) NFSBASE_PATH=$(NFSBASE) ARCH=$(ARCH) ./$(RTE)/check-rte.sh

check-yocto-rte:
	TARGET_RTE=$(YOCTO_TARGET_RTE) NFSBASE_PATH=$(NFSBASE) ARCH=$(ARCH) ./$(RTE)/check-yocto-rte.sh

# build LM32 firmware
firmware:
	$(MAKE) -C $(FW)
