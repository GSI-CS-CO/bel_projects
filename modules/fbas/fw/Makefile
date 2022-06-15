# build FW binaries for multiple platforms: SCU3, Pexaria5

export TARGET        ?= fbas

export .DEFAULT_GOAL := fwbin

# common settings
export SHARED_SIZE   ?= 8K
export USRCPUCLK     ?= 125000
export VERSION       = 00.01.00
export PATHFW        = .
export DEBUGLVL      = 2
export EXTRA_FLAGS   ?=

# platform-specific settings
PLATFPATHSCU3       := ../../../syn/gsi_scu/control3
PLATFPATHPEX5       := ../../../syn/gsi_pexarria5/control

.PHONY: fwbin
fwbin:
	@echo --- build $(TARGET) for platform $(PLATFPATHSCU3)
	$(MAKE) -f platform.mk PLATFPATH=$(PLATFPATHSCU3) $@
ifeq ($(TARGET),fbas)
	@echo --- build $(TARGET) for platform $(PLATFPATHPEX5)
	$(MAKE) -f platform.mk PLATFPATH=$(PLATFPATHPEX5) $@
endif

# ram.ld is built recursively from platform.mk
.PHONY: ram.ld
ram.ld:
	$(MAKE) -f ../../../syn/build.mk $@

clean::
	rm -f $(PATHFW)/*.o $(PATHFW)/*.a $(PATHFW)/*.elf $(PATHFW)/*.bin
