# PREFIX  controls where programs and libraries get installed
# STAGING can be used to store 'install' output into a staging folder
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   make PREFIX=/usr all
#   make STAGING=/tmp/package PREFIX=/usr install
#   ... will compile the programs to expect installation into /usr, but
#       will actually install them into /tmp/package/usr for zipping.
STAGING     ?=
PREFIX      ?= /usr/local
EXTRA_FLAGS ?=
PWD         := $(shell pwd)

all::	etherbone tools eca sdbfs toolchain firmware driver

install::	etherbone-install tools-install eca-install driver-install

clean::	etherbone-clean tools-clean eca-clean sdbfs-clean driver-clean toolchain-clean firmware-clean scu-clean exploder-clean pexarria5-clean

distclean::	clean
	git clean -xfd .
	for i in etherbone-core fpga-config-space general-cores wr-cores wrpc-sw; do cd ip_cores/$$i; git clean -xfd .; cd ../..; done

etherbone::
	$(MAKE) -C ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" all

etherbone-clean::
	$(MAKE) -C ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" clean

etherbone-install::
	$(MAKE) -C ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" install

tools::		etherbone
	$(MAKE) -C tools EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" all

tools-clean::
	$(MAKE) -C tools EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" clean

tools-install::
	$(MAKE) -C tools EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" install

eca::		etherbone
	$(MAKE) -C ip_cores/wr-cores/modules/wr_eca EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" all

eca-clean::
	$(MAKE) -C ip_cores/wr-cores/modules/wr_eca EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" clean

eca-install::
	$(MAKE) -C ip_cores/wr-cores/modules/wr_eca EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" install

driver::
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb all

driver-clean::
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb clean

driver-install::
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb install

sdbfs::
	$(MAKE) -C ip_cores/fpga-config-space/sdbfs DIRS="lib userspace doc" all

sdbfs-clean::
	$(MAKE) -C ip_cores/fpga-config-space/sdbfs DIRS="lib userspace doc" clean

gcc-4.5.3-lm32.tar.xz:
	wget http://www.ohwr.org/attachments/1301/gcc-4.5.3-lm32.tar.xz

toolchain:	gcc-4.5.3-lm32.tar.xz
	tar xvJf gcc-4.5.3-lm32.tar.xz
	mv lm32 toolchain
	touch toolchain

toolchain-clean::
	rm -rf toolchain

ip_cores/wrpc-sw/.config:
	cp ip_cores/wrpc-sw/configs/gsi_defconfig $@

firmware::	sdbfs etherbone toolchain ip_cores/wrpc-sw/.config
	$(MAKE) -C ip_cores/wrpc-sw EB=$(PWD)/ip_cores/etherbone-core/api SDBFS=$(PWD)/ip_cores/fpga-config-space/sdbfs/userspace PATH=$(PWD)/toolchain/bin:$(PATH) all

firmware-clean::
	$(MAKE) -C ip_cores/wrpc-sw EB=$(PWD)/ip_cores/etherbone-core/api SDBFS=$(PWD)/ip_cores/fpga-config-space/sdbfs/userspace PATH=$(PWD)/toolchain/bin:$(PATH) clean

scu::		firmware
	$(MAKE) -C syn/gsi_scu/control

scu-clean::
	$(MAKE) -C syn/gsi_scu/control clean

vetar::		firmware
	$(MAKE) -C syn/gsi_vetar/wr_core_demo

vetar-clean::
	$(MAKE) -C syn/gsi_vetar/wr_core_demo clean

exploder::	firmware
	$(MAKE) -C syn/gsi_exploder/wr_core_demo

exploder-clean::
	$(MAKE) -C syn/gsi_exploder/wr_core_demo clean

pexarria5::	firmware
	$(MAKE) -C syn/gsi_pexarria5/control

pexarria5-clean::
	$(MAKE) -C syn/gsi_pexarria5/control clean
