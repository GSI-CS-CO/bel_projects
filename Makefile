# PREFIX  controls where programs and libraries get installed
# STAGING can be used to store 'install' output into a staging folder
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   make PREFIX=/usr all
#   make STAGING=/tmp/package PREFIX=/usr install
#   ... will compile the programs to expect installation into /usr, but
#       will actually install them into /tmp/package/usr for zipping.
STAGING      ?=
PREFIX       ?= /usr/local
SYSCONFDIR   ?= /etc
PWD          := $(shell pwd)
EXTRA_FLAGS  ?=
export EXTRA_FLAGS

# Set variables that are passed down to sub-makes
EB=$(PWD)/ip_cores/etherbone-core/api
export EB
TLU=$(PWD)/ip_cores/wr-cores/modules/wr_tlu
export TLU
ECA=$(PWD)/ip_cores/wr-cores/modules/wr_eca
export ECA
PATH:=$(PWD)/toolchain/bin:$(PATH)

# This is mainly used to sort QSF files. After sorting it adds and deletes a "GIT marker" which will mark the file as changed.
# Additionally all empty lines will be removed.
# Example usage: 
#   $(call sort_file, "./syn/gsi_vetar2a/ee_butis/vetar2a.qsf")
define sort_file
	sort $(1) >> temp_sorted
	mv temp_sorted $(1)
	echo "GIT_MARKER" >> $(1)
	sed -i 's/GIT_MARKER//g' $(1)
	sed -i '/^$$/d' $(1)
endef

all:		etherbone tools sdbfs toolchain firmware driver

gateware:	all pexarria5 exploder5 vetar2a vetar2a-ee-butis scu2 scu3 pmc microtca

install:	etherbone-install tools-install driver-install

clean::		etherbone-clean tools-clean tlu-clean sdbfs-clean driver-clean toolchain-clean firmware-clean scu2-clean scu3-clean vetar2a-clean vetar2a-ee-butis-clean exploder5-clean pexarria5-clean sio3-clean ecatools-clean pmc-clean microtca-clean

distclean::	clean
	git clean -xfd .
	for i in etherbone-core fpga-config-space general-cores wr-cores wrpc-sw; do cd ip_cores/$$i; git clean -xfd .; cd ../..; done

etherbone::
	test -f ip_cores/etherbone-core/api/Makefile.in || ./ip_cores/etherbone-core/api/autogen.sh
	cd ip_cores/etherbone-core/api; test -f Makefile || ./configure --enable-maintainer-mode --prefix=$(PREFIX)
	$(MAKE) -C ip_cores/etherbone-core/api all

etherbone-clean::
	! test -f ip_cores/etherbone-core/api/Makefile || $(MAKE) -C ip_cores/etherbone-core/api distclean

etherbone-install::
	$(MAKE) -C ip_cores/etherbone-core/api DESTDIR=$(STAGING) install

saftlib::
	test -f ip_cores/saftlib/Makefile.in || ./ip_cores/saftlib/autogen.sh
	cd ip_cores/saftlib; test -f Makefile || ./configure --enable-maintainer-mode --prefix=$(PREFIX) --sysconfdir=$(SYSCONFDIR)
	$(MAKE) -C ip_cores/saftlib all

saftlib-clean::
	! test -f ip_cores/saftlib/Makefile || $(MAKE) -C ip_cores/saftlib distclean

saftlib-install::
	$(MAKE) -C ip_cores/saftlib DESTDIR=$(STAGING) install

tools::		etherbone
	$(MAKE) -C tools all

tools-clean::
	$(MAKE) -C tools clean

tools-install::
	$(MAKE) -C tools install

ecatools: 	etherbone eca tlu
	$(MAKE) -C tools ecatools

ecatools-clean::
	$(MAKE) -C tools ecatools-clean

ecatools-install::
	$(MAKE) -C tools ecatools-install

eca:		etherbone
	$(MAKE) -C ip_cores/wr-cores/modules/wr_eca all

eca-clean::
	$(MAKE) -C ip_cores/wr-cores/modules/wr_eca clean

eca-install::
	$(MAKE) -C ip_cores/wr-cores/modules/wr_eca install

tlu:		etherbone
	$(MAKE) -C ip_cores/wr-cores/modules/wr_tlu all

tlu-clean::
	$(MAKE) -C ip_cores/wr-cores/modules/wr_tlu clean

tlu-install::
	$(MAKE) -C ip_cores/wr-cores/modules/wr_tlu install

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

lm32-elf-gcc.tar.xz:
	wget https://github.com/GSI-CS-CO/lm32-toolchain/releases/download/v1.0-2019-05-27/lm32-elf-gcc.tar.xz

toolchain:	lm32-elf-gcc.tar.xz
	tar xvJf lm32-elf-gcc.tar.xz
	mv lm32-elf-gcc toolchain
	touch toolchain

toolchain-clean::
	rm -rf toolchain

wrpc-sw-config::
	test -s ip_cores/wrpc-sw/.config || \
		$(MAKE) -C ip_cores/wrpc-sw/ gsi_defconfig

firmware:	sdbfs etherbone toolchain wrpc-sw-config
	$(MAKE) -C ip_cores/wrpc-sw SDBFS=$(PWD)/ip_cores/fpga-config-space/sdbfs/userspace all

firmware-clean:
	$(MAKE) -C ip_cores/wrpc-sw SDBFS=$(PWD)/ip_cores/fpga-config-space/sdbfs/userspace clean

avsoc:		firmware
	$(MAKE) -C syn/gsi_avsoc/av_rocket_board all

avsoc-clean::
	$(MAKE) -C syn/gsi_avsoc/av_rocket_board clean

scu2:		firmware
	$(MAKE) -C syn/gsi_scu/control2 all

scu2-sort:
	$(call sort_file, "./syn/gsi_scu/control2/scu_control.qsf")
  
scu2-clean::
	$(MAKE) -C syn/gsi_scu/control2 clean

scu3:		firmware
	$(MAKE) -C syn/gsi_scu/control3 all

scu3-sort:
	$(call sort_file, "./syn/gsi_scu/control3/scu_control.qsf")
	
scu3-clean::
	$(MAKE) -C syn/gsi_scu/control3 clean

vetar:		firmware
	$(MAKE) -C syn/gsi_vetar/wr_core_demo all

vetar-clean::
	$(MAKE) -C syn/gsi_vetar/wr_core_demo clean

vetar2a:	firmware
	$(MAKE) -C syn/gsi_vetar2a/wr_core_demo all
	
vetar2a-sort:
	$(call sort_file, "./syn/gsi_vetar2a/wr_core_demo/vetar2a.qsf")

vetar2a-clean::
	$(MAKE) -C syn/gsi_vetar2a/wr_core_demo clean

vetar2a-ee-butis:	firmware
	$(MAKE) -C syn/gsi_vetar2a/ee_butis all
	
vetar2a-ee-butis-sort:
	$(call sort_file, "./syn/gsi_vetar2a/ee_butis/vetar2a.qsf")

vetar2a-ee-butis-clean::
	$(MAKE) -C syn/gsi_vetar2a/ee_butis clean

exploder:	firmware
	$(MAKE) -C syn/gsi_exploder/wr_core_demo all

exploder-clean::
	$(MAKE) -C syn/gsi_exploder/wr_core_demo clean

pexarria5:	firmware
	$(MAKE) -C syn/gsi_pexarria5/control all

pexarria5-sort:
	$(call sort_file, "./syn/gsi_pexarria5/control/pci_control.qsf")

pexarria5-clean::
	$(MAKE) -C syn/gsi_pexarria5/control clean

ftm:	firmware
	$(MAKE) -C syn/gsi_pexarria5/ftm all
	
ftm-sort:
	$(call sort_file, "./syn/gsi_pexarria5/ftm/ftm.qsf")

ftm-clean::
	$(MAKE) -C syn/gsi_pexarria5/ftm clean

microtca:	firmware
	$(MAKE) -C syn/gsi_microtca/control all
	
microtca-sort:
	$(call sort_file, "./syn/gsi_microtca/control/microtca_control.qsf")

microtca-clean::
	$(MAKE) -C syn/gsi_microtca/control clean

exploder5:	firmware
	$(MAKE) -C syn/gsi_exploder5/exploder5_csco_tr all

exploder5-sort:
	$(call sort_file, "./syn/gsi_exploder5/exploder5_csco_tr/exploder5_csco_tr.qsf")

exploder5-clean::
	$(MAKE) -C syn/gsi_exploder5/exploder5_csco_tr clean

addac:		firmware
	$(MAKE) -C syn/gsi_addac all

addac-clean::
	$(MAKE) -C syn/gsi_addac clean

addac2:		firmware
	$(MAKE) -C syn/gsi_addac2 all

addac2-clean::
	$(MAKE) -C syn/gsi_addac2 clean

diob:		firmware
	$(MAKE) -C syn/scu_diob all

diob-clean::
	$(MAKE) -C syn/scu_diob clean

sio3:		firmware
	$(MAKE) -C syn/scu_sio3 all

sio3-clean::
	$(MAKE) -C syn/scu_sio3 clean

pmc:	firmware
	$(MAKE) -C syn/gsi_pmc/control all
	
pmc-sort:
	$(call sort_file, "./syn/gsi_pmc/control/pci_pmc.qsf")

pmc-clean::
	$(MAKE) -C syn/gsi_pmc/control clean

ifa8:		firmware
	$(MAKE) -C syn/gsi_ifa8 all

ifa8-clean::
	$(MAKE) -C syn/gsi_ifa8 clean

### We need to run ./fix-git.sh and ./install-hdlmake.sh: make them a prerequisite for Makefile
Makefile: prereq-rule

prereq-rule::
	@test -d .git/modules/ip_cores/wrpc-sw/modules/ppsi || \
		(echo "Downloading submodules"; ./fix-git.sh)
	@test -d lib/python2.7/site-packages || \
		(echo "Installing hdlmake"; ./install-hdlmake.sh)
