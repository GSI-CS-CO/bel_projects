# PREFIX  controls where programs and libraries get installed
# STAGING can be used to store 'install' output into a staging folder
# Note: during compile (all), PREFIX must be set to the final installation path
# Example usage:
#   make PREFIX=/usr all
#   make STAGING=/tmp/package PREFIX=/usr install
#   ... will compile the programs to expect installation into /usr, but
#       will actually install them into /tmp/package/usr for zipping.
STAGING         ?=
PREFIX          ?= /usr/local
SYSCONFDIR      ?= /etc
PWD             := $(shell pwd)
EXTRA_FLAGS     ?=
WISHBONE_SERIAL ?= # Build wishbone-serial? y or leave blank
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
#   $(call sort_file, $(CHECK_SCU4))
CHECK_SCU2             = ./syn/gsi_scu/control2/scu_control
CHECK_SCU3             = ./syn/gsi_scu/control3/scu_control
CHECK_VETAR2A          = ./syn/gsi_vetar2a/wr_core_demo/vetar2a
CHECK_VETAR2A_EE_BUTIS = ./syn/gsi_vetar2a/ee_butis/vetar2a
CHECK_PEXARRIA5        = ./syn/gsi_pexarria5/control/pci_control
CHECK_EXPLODER5        = ./syn/gsi_exploder5/exploder5_csco_tr/exploder5_csco_tr
CHECK_PMC              = ./syn/gsi_pmc/control/pci_pmc
CHECK_MICROTCA         = ./syn/gsi_microtca/control/microtca_control
CHECK_PEXP             = ./syn/gsi_pexp/control/pexp_control
CHECK_SCU4             = ./syn/gsi_scu/control4/scu_control
CHECK_A10GX            = ./syn/gsi_a10gx_pcie/control/pci_control
CHECK_FTM              = ./syn/gsi_pexarria5/ftm/ftm

# Project paths
PATH_SCU2              = syn/gsi_scu/control2
PATH_SCU3              = syn/gsi_scu/control3
PATH_VETAR2A           = syn/gsi_vetar2a/wr_core_demo
PATH_VETAR2A_EE_BUTIS  = syn/gsi_vetar2a/ee_butis
PATH_PEXARRIA5         = syn/gsi_pexarria5/control
PATH_EXPLODER5         = syn/gsi_exploder5/exploder5_csco_tr
PATH_PMC               = syn/gsi_pmc/control
PATH_MICROTCA          = syn/gsi_microtca/control
PATH_PEXP              = syn/gsi_pexp/control
PATH_SCU4              = syn/gsi_scu/control4
PATH_A10GX             = syn/gsi_a10gx_pcie/control
PATH_FTM               = syn/gsi_pexarria5/ftm

define sort_file
	sort $(1).qsf >> temp_sorted
	mv temp_sorted $(1).qsf
	echo "GIT_MARKER" >> $(1).qsf
	sed -i 's/GIT_MARKER//g' $(1).qsf
	sed -i '/^$$/d' $(1).qsf
endef

define check_timing
	@test -f $(1).sta.rpt || echo "Error: Report file is missing!"
	@ls -l $(1).sta.rpt
	@cat $(1).sta.rpt | grep "Timing requirements not met" && exit 1 || { exit 0; }
	@echo "Success! All Timing requirements were met!"
endef

all:		etherbone tools sdbfs toolchain firmware driver

gateware:	all pexarria5 exploder5 vetar2a vetar2a-ee-butis scu2 scu3 pmc microtca pexp

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
ifeq ($(WISHBONE_SERIAL),)
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb all
else
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb all CONFIG_USB_SERIAL_WISHBONE=yes
endif
	$(MAKE) -C ip_cores/fpga-config-space/vme-wb all

driver-clean::
ifeq ($(WISHBONE_SERIAL),)
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb clean
else
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb clean CONFIG_USB_SERIAL_WISHBONE=yes
endif
	$(MAKE) -C ip_cores/fpga-config-space/vme-wb clean

driver-install::
ifeq ($(WISHBONE_SERIAL),)
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb install
else
	$(MAKE) -C ip_cores/fpga-config-space/pcie-wb install CONFIG_USB_SERIAL_WISHBONE=yes
endif
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

# #################################################################################################
# Arria 2 devices
# #################################################################################################

scu2:		firmware
	$(MAKE) -C $(PATH_SCU2) all

scu2-clean::
	$(MAKE) -C $(PATH_SCU2) clean

scu2-sort:
	$(call sort_file, $(CHECK_SCU2))

scu2-check:
	$(call check_timing, $(CHECK_SCU2))

scu3:		firmware
	$(MAKE) -C $(PATH_SCU3) all

scu3-clean::
	$(MAKE) -C $(PATH_SCU3) clean

scu3-sort:
	$(call sort_file, $(CHECK_SCU3))

scu3-check:
	$(call check_timing, $(CHECK_SCU3))

vetar2a:	firmware
	$(MAKE) -C $(PATH_VETAR2A) all

vetar2a-clean::
	$(MAKE) -C $(PATH_VETAR2A) clean

vetar2a-sort:
	$(call sort_file, $(CHECK_VETAR2A))

vetar2a-check:
	$(call check_timing, $(CHECK_VETAR2A))

vetar2a-ee-butis:	firmware
	$(MAKE) -C $(PATH_VETAR2A_EE_BUTIS) all

vetar2a-ee-butis-clean::
	$(MAKE) -C $(PATH_VETAR2A_EE_BUTIS) clean

vetar2a-ee-butis-sort:
	$(call sort_file, $(CHECK_VETAR2A_EE_BUTIS))

vetar2a-ee-butis-check:
	$(call check_timing, $(CHECK_VETAR2A_EE_BUTIS))

# #################################################################################################
# Arria 5 devices
# #################################################################################################

pexarria5:	firmware
	$(MAKE) -C $(PATH_PEXARRIA5) all

pexarria5-clean::
	$(MAKE) -C $(PATH_PEXARRIA5) clean

pexarria5-sort:
	$(call sort_file, $(CHECK_PEXARRIA5))

pexarria5-check:
	$(call check_timing, $(CHECK_PEXARRIA5))

microtca::	firmware
	$(MAKE) -C $(PATH_MICROTCA) all

microtca-clean::
	$(MAKE) -C $(PATH_MICROTCA) clean

microtca-sort:
	$(call sort_file, $(CHECK_MICROTCA))

microtca-check:
	$(call check_timing, $(CHECK_MICROTCA))

exploder5:	firmware
	$(MAKE) -C $(PATH_EXPLODER5) all

exploder5-clean::
	$(MAKE) -C $(PATH_EXPLODER5) clean

exploder5-sort:
	$(call sort_file, $(CHECK_EXPLODER5))

exploder5-check:
	$(call check_timing, $(CHECK_EXPLODER5))

pmc:	firmware
	$(MAKE) -C $(PATH_PMC) all

pmc-clean::
	$(MAKE) -C $(PATH_PMC) clean

pmc-sort:
	$(call sort_file, $(CHECK_PMC))

pmc-check:
	$(call check_timing, $(CHECK_PMC))

pexp:	firmware
	$(MAKE) -C $(PATH_PEXP) all

pexp-clean::
	$(MAKE) -C $(PATH_PEXP) clean

pexp-sort:
	$(call sort_file, $(CHECK_PEXP))

pexp-check:
	$(call check_timing, $(CHECK_PEXP))

ftm:	firmware
	$(MAKE) -C $(PATH_FTM) all

ftm-clean::
	$(MAKE) -C $(PATH_FTM) clean

ftm-sort:
	$(call sort_file, $(CHECK_FTM))

ftm-check:
	$(call check_timing, $(CHECK_FTM))

# #################################################################################################
# Arria 10 devices
# #################################################################################################

scu4:		firmware
	$(MAKE) -C $(PATH_SCU4) all

scu4-sort:
	$(call sort_file, $(CHECK_SCU4))

scu4-check:
	$(call check_timing, $(CHECK_SCU4))

scu4-clean::
	$(MAKE) -C $(PATH_SCU4) clean

a10gx_pcie::	firmware
	$(MAKE) -C $(PATH_A10GX) all

a10gx_pcie-clean::
	$(MAKE) -C $(PATH_A10GX) clean

a10gx_pcie-sort:
	$(call sort_file, $(CHECK_A10GX))

a10gx_pcie-check:
	$(call check_timing, $(CHECK_A10GX))

# #################################################################################################
# SCU slaves
# #################################################################################################

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

ifa8:		firmware
	$(MAKE) -C syn/gsi_ifa8 all

ifa8-clean::
	$(MAKE) -C syn/gsi_ifa8 clean

# #################################################################################################
# Legacy and unmaintained devices
# #################################################################################################

avsoc:		firmware
	$(MAKE) -C syn/gsi_avsoc/av_rocket_board all

avsoc-clean::
	$(MAKE) -C syn/gsi_avsoc/av_rocket_board clean

vetar::		firmware
	$(MAKE) -C syn/gsi_vetar/wr_core_demo PATH=$(PWD)/toolchain/bin:$(PATH) all

vetar-clean::
	$(MAKE) -C syn/gsi_vetar/wr_core_demo clean

exploder:	firmware
	$(MAKE) -C syn/gsi_exploder/wr_core_demo all

exploder-clean::
	$(MAKE) -C syn/gsi_exploder/wr_core_demo clean

pexarria10::	firmware
	$(MAKE) -C syn/gsi_pexarria10/control PATH=$(PWD)/toolchain/bin:$(PATH) all

pexarria10-clean::
	$(MAKE) -C syn/gsi_pexarria10/control PATH=$(PWD)/toolchain/bin:$(PATH) clean

pexarria10_soc::	firmware
	$(MAKE) -C syn/gsi_pexarria10_soc/control PATH=$(PWD)/toolchain/bin:$(PATH) all

pexarria10_soc-clean::
	$(MAKE) -C syn/gsi_pexarria10_soc/control PATH=$(PWD)/toolchain/bin:$(PATH) clean

### We need to run ./fix-git.sh and ./install-hdlmake.sh: make them a prerequisite for Makefile
Makefile: prereq-rule

prereq-rule::
	@test -d .git/modules/ip_cores/wrpc-sw/modules/ppsi || \
		(echo "Downloading submodules"; ./fix-git.sh)
	@test -d lib/python2.7/site-packages || \
		(echo "Installing hdlmake"; ./install-hdlmake.sh)

git_submodules_update:
	@git submodule update --recursive

git_submodules_init:
	@./fix-git.sh
