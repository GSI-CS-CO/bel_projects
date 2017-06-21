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
SYSCONFDIR  ?= /etc
EXTRA_FLAGS ?=
PWD         := $(shell pwd)

all::		etherbone tools sdbfs toolchain firmware driver

gateware:	all pexarria5 exploder5 vetar2a scu2 scu3

install::	etherbone-install tools-install driver-install

clean::		etherbone-clean tools-clean tlu-clean sdbfs-clean driver-clean toolchain-clean firmware-clean scu2-clean scu3-clean exploder-clean exploder5-clean pexarria5-clean sio3-clean ecatools-clean pmc-clean

distclean::	clean
	git clean -xfd .
	for i in etherbone-core fpga-config-space general-cores wr-cores wrpc-sw; do cd ip_cores/$$i; git clean -xfd .; cd ../..; done

etherbone::
	test -f ip_cores/etherbone-core/api/Makefile.in || ./ip_cores/etherbone-core/api/autogen.sh
	cd ip_cores/etherbone-core/api; test -f Makefile || ./configure --enable-maintainer-mode --prefix=$(PREFIX)
	$(MAKE) -C ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" all

etherbone-clean::
	! test -f ip_cores/etherbone-core/api/Makefile || $(MAKE) -C ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" distclean

etherbone-install::
	$(MAKE) -C ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" DESTDIR=$(STAGING) install

saftlib::
	test -f ip_cores/saftlib/Makefile.in || ./ip_cores/saftlib/autogen.sh
	cd ip_cores/saftlib; test -f Makefile || ./configure --enable-maintainer-mode --prefix=$(PREFIX) --sysconfdir=$(SYSCONFDIR)
	$(MAKE) -C ip_cores/saftlib EXTRA_FLAGS="$(EXTRA_FLAGS)" all

saftlib-clean::
	! test -f ip_cores/saftlib/Makefile || $(MAKE) -C ip_cores/saftlib EXTRA_FLAGS="$(EXTRA_FLAGS)" distclean

saftlib-install::
	$(MAKE) -C ip_cores/saftlib EXTRA_FLAGS="$(EXTRA_FLAGS)" DESTDIR=$(STAGING) install

tools::		etherbone
	$(MAKE) -C tools EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" all

tools-clean::
	$(MAKE) -C tools EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" clean

tools-install::
	$(MAKE) -C tools EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" install

ecatools:: 	etherbone eca tlu
	$(MAKE) -C tools ECA=$(PWD)/ip_cores/wr-cores/modules/wr_eca TLU=$(PWD)/ip_cores/wr-cores/modules/wr_tlu EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" ecatools

ecatools-clean::
	$(MAKE) -C tools ECA=$(PWD)/ip_cores/wr-cores/modules/wr_eca TLU=$(PWD)/ip_cores/wr-cores/modules/wr_tlu EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" ecatools-clean

ecatools-install::
	$(MAKE) -C tools ECA=$(PWD)/ip_cores/wr-cores/modules/wr_eca TLU=$(PWD)/ip_cores/wr-cores/modules/wr_tlu EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" ecatools-install

eca::		etherbone
	$(MAKE) -C ip_cores/wr-cores/modules/wr_eca EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" all

eca-clean::
	$(MAKE) -C ip_cores/wr-cores/modules/wr_eca EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" clean

eca-install::
	$(MAKE) -C ip_cores/wr-cores/modules/wr_eca EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" install

tlu::		etherbone
	$(MAKE) -C ip_cores/wr-cores/modules/wr_tlu EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" all

tlu-clean::
	$(MAKE) -C ip_cores/wr-cores/modules/wr_tlu EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" clean

tlu-install::
	$(MAKE) -C ip_cores/wr-cores/modules/wr_tlu EB=$(PWD)/ip_cores/etherbone-core/api EXTRA_FLAGS="$(EXTRA_FLAGS)" install

driver::
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb all
	$(MAKE) -C ip_cores/fpga-config-space/vme-wb all

driver-clean::
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb clean
	$(MAKE) -C ip_cores/fpga-config-space/vme-wb clean

driver-install::
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb install
	$(MAKE) -C ip_cores/fpga-config-space/vme-wb install

sdbfs::
	$(MAKE) -C ip_cores/fpga-config-space/sdbfs DIRS="lib userspace" all

sdbfs-clean::
	$(MAKE) -C ip_cores/fpga-config-space/sdbfs DIRS="lib userspace" clean

gcc-4.5.3-lm32.tar.xz:
	wget http://www.ohwr.org/attachments/1301/gcc-4.5.3-lm32.tar.xz

toolchain:	gcc-4.5.3-lm32.tar.xz
	tar xvJf gcc-4.5.3-lm32.tar.xz
	mv lm32 toolchain
	touch toolchain

toolchain-clean::
	rm -rf toolchain

wrpc-sw-config:
	$(MAKE) -C ip_cores/wrpc-sw/ gsi_defconfig

firmware::	sdbfs etherbone toolchain wrpc-sw-config
	$(MAKE) -C ip_cores/wrpc-sw EB=$(PWD)/ip_cores/etherbone-core/api SDBFS=$(PWD)/ip_cores/fpga-config-space/sdbfs/userspace PATH=$(PWD)/toolchain/bin:$(PATH) all

firmware-clean::
	$(MAKE) -C ip_cores/wrpc-sw EB=$(PWD)/ip_cores/etherbone-core/api SDBFS=$(PWD)/ip_cores/fpga-config-space/sdbfs/userspace PATH=$(PWD)/toolchain/bin:$(PATH) clean

avsoc::		firmware
	$(MAKE) -C syn/gsi_avsoc/av_rocket_board PATH=$(PWD)/toolchain/bin:$(PATH) all

avsoc-clean::
	$(MAKE) -C syn/gsi_avsoc/av_rocket_board PATH=$(PWD)/toolchain/bin:$(PATH) clean

scu2::		firmware
	$(MAKE) -C syn/gsi_scu/control2 PATH=$(PWD)/toolchain/bin:$(PATH) all

scu2-clean::
	$(MAKE) -C syn/gsi_scu/control2 PATH=$(PWD)/toolchain/bin:$(PATH) clean

scu3::		firmware
	$(MAKE) -C syn/gsi_scu/control3 PATH=$(PWD)/toolchain/bin:$(PATH) all

scu3-clean::
	$(MAKE) -C syn/gsi_scu/control3 PATH=$(PWD)/toolchain/bin:$(PATH) clean

vetar::		firmware
	$(MAKE) -C syn/gsi_vetar/wr_core_demo PATH=$(PWD)/toolchain/bin:$(PATH) all

vetar-clean::
	$(MAKE) -C syn/gsi_vetar/wr_core_demo PATH=$(PWD)/toolchain/bin:$(PATH) clean

vetar2a::	firmware
	$(MAKE) -C syn/gsi_vetar2a/wr_core_demo PATH=$(PWD)/toolchain/bin:$(PATH) all

vetar2a-clean::
	$(MAKE) -C syn/gsi_vetar2a/wr_core_demo PATH=$(PWD)/toolchain/bin:$(PATH) clean

vetar2a-ee-butis::	firmware
	$(MAKE) -C syn/gsi_vetar2a/ee_butis PATH=$(PWD)/toolchain/bin:$(PATH) all

vetar2a-ee-butis-clean::
	$(MAKE) -C syn/gsi_vetar2a/ee_butis PATH=$(PWD)/toolchain/bin:$(PATH) clean

exploder::	firmware
	$(MAKE) -C syn/gsi_exploder/wr_core_demo PATH=$(PWD)/toolchain/bin:$(PATH) all

exploder-clean::
	$(MAKE) -C syn/gsi_exploder/wr_core_demo PATH=$(PWD)/toolchain/bin:$(PATH) clean

pexarria5::	firmware
	$(MAKE) -C syn/gsi_pexarria5/control PATH=$(PWD)/toolchain/bin:$(PATH) all

pexarria5-clean::
	$(MAKE) -C syn/gsi_pexarria5/control PATH=$(PWD)/toolchain/bin:$(PATH) clean

microtca::	firmware
	$(MAKE) -C syn/gsi_microtca/control PATH=$(PWD)/toolchain/bin:$(PATH) all

microtca-clean::
	$(MAKE) -C syn/gsi_microtca/control PATH=$(PWD)/toolchain/bin:$(PATH) clean

exploder5::	firmware
	$(MAKE) -C syn/gsi_exploder5/exploder5_csco_tr PATH=$(PWD)/toolchain/bin:$(PATH) all

exploder5-clean::
	$(MAKE) -C syn/gsi_exploder5/exploder5_csco_tr PATH=$(PWD)/toolchain/bin:$(PATH) clean

addac::		firmware
	$(MAKE) -C syn/gsi_addac PATH=$(PWD)/toolchain/bin:$(PATH) all

addac-clean::
	$(MAKE) -C syn/gsi_addac PATH=$(PWD)/toolchain/bin:$(PATH) clean

addac2::	firmware
	$(MAKE) -C syn/gsi_addac2 PATH=$(PWD)/toolchain/bin:$(PATH) all

addac2-clean::
	$(MAKE) -C syn/gsi_addac2 PATH=$(PWD)/toolchain/bin:$(PATH) clean

diob::		firmware		
	$(MAKE) -C syn/scu_diob PATH=$(PWD)/toolchain/bin:$(PATH) all

diob-clean::
	$(MAKE) -C syn/scu_diob PATH=$(PWD)/toolchain/bin:$(PATH) clean

sio3::		firmware		
	$(MAKE) -C syn/scu_sio3 PATH=$(PWD)/toolchain/bin:$(PATH) all

sio3-clean::
	$(MAKE) -C syn/scu_sio3 PATH=$(PWD)/toolchain/bin:$(PATH) clean

pmc::	firmware
	$(MAKE) -C syn/gsi_pmc/control PATH=$(PWD)/toolchain/bin:$(PATH) all

pmc-clean::
	$(MAKE) -C syn/gsi_pmc/control PATH=$(PWD)/toolchain/bin:$(PATH) clean

ifa8::		firmware
	$(MAKE) -C syn/gsi_ifa8 PATH=$(PWD)/toolchain/bin:$(PATH) all

ifa8-clean::
	$(MAKE) -C syn/gsi_ifa8 PATH=$(PWD)/toolchain/bin:$(PATH) clean
