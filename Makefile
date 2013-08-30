all::	etherbone tools driver toolchain firmware

clean::	etherbone-clean tools-clean driver-clean toolchain-clean firmware-clean scu-clean exploder-clean pexarria5-clean

distclean::
	git clean -xfd .
	for i in etherbone-core fpga-config-space general-cores wr-cores wrpc-sw; do cd ip_cores/$$i; git clean -xfd .; cd ../..; done

install::	etherbone-install tools-install driver-install

etherbone::
	$(MAKE) -C ip_cores/etherbone-core/api all

etherbone-clean::
	$(MAKE) -C ip_cores/etherbone-core/api clean

etherbone-install::
	$(MAKE) -C ip_cores/etherbone-core/api install

tools::
	$(MAKE) -C tools all

tools-clean::
	$(MAKE) -C tools clean

tools-install::
	$(MAKE) -C tools install

driver::
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb all

driver-clean::
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb clean

driver-install::
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb install

gcc-4.5.3-lm32.tar.xz:
	wget http://www.ohwr.org/attachments/1301/gcc-4.5.3-lm32.tar.xz

toolchain:	gcc-4.5.3-lm32.tar.xz
	tar xvJf gcc-4.5.3-lm32.tar.xz
	mv lm32 toolchain
	touch toolchain

toolchain-clean::
	rm -rf toolchain

ip_cores/wrpc-sw/.config:
	cp ip_cores/wrpc-sw/configs/etherbone_defconfig $@

firmware::	toolchain ip_cores/wrpc-sw/.config
	$(MAKE) PATH=$(PWD)/toolchain/bin:$(PATH) -C ip_cores/wrpc-sw all

firmware-clean::
	$(MAKE) PATH=$(PWD)/toolchain/bin:$(PATH) -C ip_cores/wrpc-sw clean

scu::		firmware
	$(MAKE) -C syn/gsi_scu/control

scu-clean::
	$(MAKE) -C syn/gsi_scu/control clean

exploder::	firmware
	$(MAKE) -C syn/gsi_exploder/wr_core_demo

exploder-clean::
	$(MAKE) -C syn/gsi_exploder/wr_core_demo clean

pexarria5::	firmware
	$(MAKE) -C syn/gsi_pexarria5/control

pexarria5-clean::
	$(MAKE) -C syn/gsi_pexarria5/control clean
