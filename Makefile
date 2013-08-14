all::	etherbone driver toolchain firmware

install::	etherbone-install driver-install

etherbone::
	$(MAKE) -C ip_cores/etherbone-core/api all

etherbone-install::
	$(MAKE) -C ip_cores/etherbone-core/api install

driver::
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb all

driver-install::
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb install

gcc-4.5.3-lm32.tar.xz:
	wget http://www.ohwr.org/attachments/1301/gcc-4.5.3-lm32.tar.xz

toolchain:	gcc-4.5.3-lm32.tar.xz
	tar xvJf gcc-4.5.3-lm32.tar.xz
	mv lm32 toolchain

firmware::	toolchain
	$(MAKE) PATH=$(PWD)/toolchain/bin:$(PATH) -C ip_cores/wrpc-sw all

scu::		firmware
	$(MAKE) -C syn/gsi_scu/control

exploder::	firmware
	$(MAKE) -C syn/gsi_exploder/wr_core_demo

pexarria5::	firmware
	$(MAKE) -C syn/gsi_pexarria5/control
